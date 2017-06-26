/*--------------------------------------------------
   PMFRDLG1.C -- FRACTINT for PM

   Fractal Attributes Dialogs Module

   02/16/91      Code by Donald P. Egen (with help)

   This module contains all functions backing dialog
   boxes that handle Fractal selection, parameters,
   and so on used by the Fractal calculation engine.
 ---------------------------------------------------*/

#define INCL_WIN
#define INCL_GPI
#define INCL_DOS
#include <os2.h>
#include <process.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <smplhelp.h>

#include "pmfract.h"
#include "fractint.h"
#include "fractype.h"

static VOID LoadExtents (HWND hwnd, PNEWPARAM npTempParms);
static VOID LoadParams  (HWND hwnd, PNEWPARAM npTempParms);
static VOID LoadTrigs   (HWND hwnd, unsigned char whichtrig);
static VOID LoadImageXandY (HWND hwnd, SHORT cx, SHORT cy);
static VOID LoadOptions (HWND hwnd, PNEWPARAM npTempParms);


/*--------------------------------------------------
  This function backs the Options/Set Parameters... dialog box.

  The current parameters for the calculation engine
  are read into the local structure npTempParms
  via InitNewParms (during WM_INITDLG) and set into
  the dialog entry fields.

  Entry to any field is sensed via WM_CONTROL
  notify messages, read, interpreded as a floating
  point number, and it and other affected fields
  updated.  Currently a nonsense string is not
  explicitly detected (I assume the user is reading
  the numbers echoed to him).

  The HELP button triggers an appropriate message
  displayed via the SMPLHELP system.

  On dismiss, OK returns TRUE, Cancel returns FALSE.
  The calling code will then finish scheduling
  the parameters on an OK exit.
 --------------------------------------------------*/
MRESULT EXPENTRY SetExtentsDlgProc (HWND hwnd, USHORT msg, MPARAM mp1, MPARAM mp2)
     {
     char   achResponse[33];
     double dTemp, dDelta;
     char   tszFloatFormat[10];

     _fstrcpy(tszFloatFormat, szFloatFormat);

     switch (msg)
          {
          case WM_CONTROL:

               switch ( (USHORT) SHORT1FROMMP(mp1) )
                    {
                    case IDD_SET_XLEFT:
                         if ( ((USHORT) SHORT2FROMMP(mp1)) == EN_KILLFOCUS )
                            {
                            WinQueryDlgItemText(hwnd, IDD_SET_XLEFT,
                                 sizeof achResponse, achResponse);
                            dTemp = atof(achResponse);
                            npTempParms.XL = max(dTemp, npTempParms.mxXL);
                            sprintf(achResponse, tszFloatFormat, npTempParms.XL);
                            WinSetDlgItemText(hwnd, IDD_SET_XLEFT, achResponse);
                            npTempParms.XCenter = (npTempParms.XL + npTempParms.XR) / 2.0;
                            sprintf(achResponse, tszFloatFormat, npTempParms.XCenter);
                            WinSetDlgItemText(hwnd, IDD_SET_XCENTER, achResponse);

                            return 0;
                            }

                         break;

                    case IDD_SET_XRIGHT:
                         if ( ((USHORT) SHORT2FROMMP(mp1)) == EN_KILLFOCUS )
                            {
                            WinQueryDlgItemText(hwnd, IDD_SET_XRIGHT,
                                 sizeof achResponse, achResponse);
                            dTemp = atof(achResponse);
                            npTempParms.XR = min(dTemp,  npTempParms.mxXR);
                            sprintf(achResponse, tszFloatFormat, npTempParms.XR);
                            WinSetDlgItemText(hwnd, IDD_SET_XRIGHT, achResponse);
                            npTempParms.XCenter = (npTempParms.XL + npTempParms.XR) / 2.0;
                            sprintf(achResponse, tszFloatFormat, npTempParms.XCenter);
                            WinSetDlgItemText(hwnd, IDD_SET_XCENTER, achResponse);

                            return 0;
                            }

                         break;

                    case IDD_SET_YTOP:
                         if ( ((USHORT) SHORT2FROMMP(mp1)) == EN_KILLFOCUS )
                            {
                            WinQueryDlgItemText(hwnd, IDD_SET_YTOP,
                                 sizeof achResponse, achResponse);
                            dTemp = atof(achResponse);
                            npTempParms.YT = min(dTemp,  npTempParms.mxYT);
                            sprintf(achResponse, tszFloatFormat, npTempParms.YT);
                            WinSetDlgItemText(hwnd, IDD_SET_YTOP, achResponse);
                            npTempParms.YCenter = (npTempParms.YT + npTempParms.YB) / 2.0;
                            sprintf(achResponse, tszFloatFormat, npTempParms.YCenter);
                            WinSetDlgItemText(hwnd, IDD_SET_YCENTER, achResponse);

                            return 0;
                            }

                         break;

                    case IDD_SET_YBOTTOM:
                         if ( ((USHORT) SHORT2FROMMP(mp1)) == EN_KILLFOCUS )
                            {
                            WinQueryDlgItemText(hwnd, IDD_SET_YBOTTOM,
                                 sizeof achResponse, achResponse);
                            dTemp = atof(achResponse);
                            npTempParms.YB = max(dTemp, npTempParms.mxYB);
                            sprintf(achResponse, tszFloatFormat, npTempParms.YB);
                            WinSetDlgItemText(hwnd, IDD_SET_YBOTTOM, achResponse);
                            npTempParms.YCenter = (npTempParms.YT + npTempParms.YB) / 2.0;
                            sprintf(achResponse, tszFloatFormat, npTempParms.YCenter);
                            WinSetDlgItemText(hwnd, IDD_SET_YCENTER, achResponse);

                            return 0;
                            }

                         break;

                    case IDD_SET_XCENTER:
                         if ( ((USHORT) SHORT2FROMMP(mp1)) == EN_KILLFOCUS )
                            {
                            WinQueryDlgItemText(hwnd, IDD_SET_XCENTER,
                                 sizeof achResponse, achResponse);
                            dTemp = atof(achResponse);
                            dTemp = min(dTemp, npTempParms.mxXR);   /* upper AND lower bounds */
                            dTemp = max(dTemp, npTempParms.mxXL);
                            dDelta = dTemp - npTempParms.XCenter;
                            npTempParms.XCenter = dTemp;
                            if (dDelta < 0.0)
                               {   /* shift left - check left edge first for out of bounds */
                               dTemp = npTempParms.XL + dDelta;   /* new assumed left edge */
                               npTempParms.XL = max(dTemp, npTempParms.mxXL); /* corrected left edge */
                               dDelta = npTempParms.XL - npTempParms.XCenter;
                               npTempParms.XR = npTempParms.XCenter - dDelta;
                               }
                            else
                               {   /* shift right - check right edge first for out of bounds */
                               dTemp = npTempParms.XR + dDelta;   /* new assumed right edge */
                               npTempParms.XR = min(dTemp,  npTempParms.mxXR); /* corrected right edge */
                               dDelta = npTempParms.XR - npTempParms.XCenter;
                               npTempParms.XL = npTempParms.XCenter - dDelta;
                               }
                            sprintf(achResponse, tszFloatFormat, npTempParms.XL);
                            WinSetDlgItemText(hwnd, IDD_SET_XLEFT, achResponse);
                            sprintf(achResponse, tszFloatFormat, npTempParms.XR);
                            WinSetDlgItemText(hwnd, IDD_SET_XRIGHT, achResponse);
                            sprintf(achResponse, tszFloatFormat, npTempParms.XCenter);
                            WinSetDlgItemText(hwnd, IDD_SET_XCENTER, achResponse);

                            return 0;
                            }

                         break;

                    case IDD_SET_YCENTER:
                         if ( ((USHORT) SHORT2FROMMP(mp1)) == EN_KILLFOCUS )
                            {
                            WinQueryDlgItemText(hwnd, IDD_SET_YCENTER,
                                 sizeof achResponse, achResponse);
                            dTemp = atof(achResponse);
                            dTemp = min(dTemp, npTempParms.mxYT);   /* upper AND lower bounds */
                            dTemp = max(dTemp, npTempParms.mxYB);
                            dDelta = dTemp - npTempParms.YCenter;
                            npTempParms.YCenter = dTemp;
                            if (dDelta < 0.0)
                               {   /* shift down - check bottom edge first for out of bounds */
                               dTemp = npTempParms.YB + dDelta;   /* new assumed bottom edge */
                               npTempParms.YB = max(dTemp, npTempParms.mxYB); /* corrected bottom edge */
                               dDelta = npTempParms.YB - npTempParms.YCenter;
                               npTempParms.YT = npTempParms.YCenter - dDelta;
                               }
                            else
                               {   /* shift up - check top edge first for out of bounds */
                               dTemp = npTempParms.YT + dDelta;   /* new assumed top edge */
                               npTempParms.YT = min(dTemp, npTempParms.mxYT); /* corrected top edge */
                               dDelta = npTempParms.YT - npTempParms.YCenter;
                               npTempParms.YB = npTempParms.YCenter - dDelta;
                               }
                            sprintf(achResponse, tszFloatFormat, npTempParms.YB);
                            WinSetDlgItemText(hwnd, IDD_SET_YBOTTOM, achResponse);
                            sprintf(achResponse, tszFloatFormat, npTempParms.YT);
                            WinSetDlgItemText(hwnd, IDD_SET_YTOP, achResponse);
                            sprintf(achResponse, tszFloatFormat, npTempParms.YCenter);
                            WinSetDlgItemText(hwnd, IDD_SET_YCENTER, achResponse);

                            return 0;
                            }

                         break;

                    }
               break;

          case WM_INITDLG:
               CenterDialogBox(hwnd);

               InitNewParms(&npTempParms);

               LoadExtents (hwnd, &npTempParms);

               return 0;

          case WM_COMMAND:
               switch (COMMANDMSG(&msg)->cmd)
                    {
                    case DID_OK:
                         npNewParms = npTempParms;
                         npNewParms.fNewParms = TRUE;
                         WinDismissDlg (hwnd, TRUE) ;
                         return 0 ;

                    case DID_CANCEL:
                         npTempParms.fNewParms = FALSE;
                         WinDismissDlg (hwnd, FALSE) ;
                         return 0 ;

                    case IDD_SET_XDEF:
                         /* zoom out to maximum range */
                         npTempParms.XL = npTempParms.mxXL;
                         npTempParms.XR = npTempParms.mxXR;
                         npTempParms.YT = npTempParms.mxYT;
                         npTempParms.YB = npTempParms.mxYB;
                         npTempParms.XCenter = (npTempParms.XL + npTempParms.XR)/2.0;
                         npTempParms.YCenter = (npTempParms.YT + npTempParms.YB)/2.0;
                         /* refresh each diaglog box field */
                         LoadExtents (hwnd, &npTempParms);

                         return 0;
                    }
               break;

          case WM_HELP:
               SimpleHelp(hab, hwnd, szTitleBar,
                       (HMODULE) 0, IDT_TEXT, IDT_HELP_EXTENT);

               return 0;

               break;
          }
     return WinDefDlgProc (hwnd, msg, mp1, mp2) ;
     }

/*--------------------------------------------------
  Local subfunction to SetExtents.

  This function loads the entry fields for
  SetExtentsDlgProc.
 --------------------------------------------------*/
static VOID LoadExtents (HWND hwnd, PNEWPARAM npTempParms)
     {
     char achResponse[33];

     char   tszFloatFormat[10];

     _fstrcpy(tszFloatFormat, szFloatFormat);

     /* load each entryfield in turn */
     sprintf(achResponse, tszFloatFormat, npTempParms->XL);
     WinSetDlgItemText(hwnd, IDD_SET_XLEFT, achResponse);
     sprintf(achResponse, tszFloatFormat, npTempParms->XR);
     WinSetDlgItemText(hwnd, IDD_SET_XRIGHT, achResponse);
     sprintf(achResponse, tszFloatFormat, npTempParms->YT);
     WinSetDlgItemText(hwnd, IDD_SET_YTOP, achResponse);
     sprintf(achResponse, tszFloatFormat, npTempParms->YB);
     WinSetDlgItemText(hwnd, IDD_SET_YBOTTOM, achResponse);
     sprintf(achResponse, tszFloatFormat, npTempParms->XCenter);
     WinSetDlgItemText(hwnd, IDD_SET_XCENTER, achResponse);
     sprintf(achResponse, tszFloatFormat, npTempParms->YCenter);
     WinSetDlgItemText(hwnd, IDD_SET_YCENTER, achResponse);

     }

/*--------------------------------------------------
  This function backs the View/Zoom/Value... dialog box.

  A default of 2.00 is suggested at initialization.

  On OK exit, the entry field contents is intrepreted
  as a floating point number and returned.
 --------------------------------------------------*/
MRESULT EXPENTRY ZoomValueDlgProc (HWND hwnd, USHORT msg, MPARAM mp1, MPARAM mp2)
     {
     static USHORT usInOut = 0;

     double dTemp;
     char   achResponse[33];
     char   tszFloatFormat[10];

     _fstrcpy(tszFloatFormat, szFloatFormat);

     switch (msg)
          {
          case WM_INITDLG:
               CenterDialogBox(hwnd);

               InitNewParms(&npTempParms);
               switch (*((PUSHORT) PVOIDFROMMP(mp2) ) )
                    {
                    case IDM_ZOUT_PICK:
                         WinSetDlgItemText(hwnd, IDD_NUMBER_PROMPT,
                                                 szZoomOut) ;
                         usInOut = IDM_ZOUT_PICK;
                         break;
                    default:       /* Paranoia */
                         WinSetDlgItemText(hwnd, IDD_NUMBER_PROMPT,
                                                 szZoomIn);
                         usInOut = IDM_ZIN_PICK;
                         break;
                    }
               WinSetDlgItemText(hwnd, IDD_NUMBER_EF, "2.00");

               return 0;

          case WM_COMMAND:
               switch (COMMANDMSG(&msg)->cmd)
                    {
                    case DID_OK:
                         WinQueryDlgItemText(hwnd, IDD_NUMBER_EF,
                              sizeof achResponse, achResponse);
                         dTemp = atof(achResponse);
                         if (dTemp != 0.0)
                            {
                            switch (usInOut)
                                 {
                                 case IDM_ZIN_PICK:
                                      CalcZoomValues (&npTempParms, dTemp, TRUE);
                                      break;

                                 case IDM_ZOUT_PICK:
                                      CalcZoomValues (&npTempParms, dTemp, FALSE);
                                      break;
                                 }
                            npNewParms = npTempParms;
                            npNewParms.fNewParms = TRUE;
                            WinDismissDlg (hwnd, TRUE) ;
                            return 0 ;
                            }
                         else
                            WinAlarm(HWND_DESKTOP, WA_ERROR);
                         break;

                    case DID_CANCEL:
                         npTempParms.fNewParms = FALSE;
                         WinDismissDlg (hwnd, FALSE) ;
                         return 0 ;
                    }
               break ;
          }
     return WinDefDlgProc (hwnd, msg, mp1, mp2) ;
     }

/*--------------------------------------------------
  Subfunction for ZoomValuesDlgProc.

  Apply the new zoom value to all extent values.
 --------------------------------------------------*/
VOID    CalcZoomValues (PNEWPARAM npTempParms, double dFactor, BOOL InOrOut)
     {
     /* parm InOrOut is TRUE for IN, FALSE for OUT */

     if (InOrOut)
        {    /* IN */
        npTempParms->XL = npTempParms->XCenter -
                 (npTempParms->XCenter - npTempParms->XL) / dFactor;
        npTempParms->XR = npTempParms->XCenter +
                 (npTempParms->XR - npTempParms->XCenter) / dFactor;
        npTempParms->YT = npTempParms->YCenter +
                 (npTempParms->YT - npTempParms->YCenter) / dFactor;
        npTempParms->YB = npTempParms->YCenter -
                 (npTempParms->YCenter - npTempParms->YB) / dFactor;
        }
     else   /* OUT */
        {
        npTempParms->XL = npTempParms->XCenter -
                 (npTempParms->XCenter - npTempParms->XL) * dFactor;
        npTempParms->XR = npTempParms->XCenter +
                 (npTempParms->XR - npTempParms->XCenter) * dFactor;
        npTempParms->YT = npTempParms->YCenter +
                 (npTempParms->YT - npTempParms->YCenter) * dFactor;
        npTempParms->YB = npTempParms->YCenter -
                 (npTempParms->YCenter - npTempParms->YB) * dFactor;

        /* Out zooms are limited to [current bounds] because */
        /*  beyond those limits is outside the set anyhow */
        npTempParms->XL = max(npTempParms->XL, npTempParms->mxXL);
        npTempParms->XR = min(npTempParms->XR, npTempParms->mxXR);
        npTempParms->YT = min(npTempParms->YT, npTempParms->mxYT);
        npTempParms->YB = max(npTempParms->YB, npTempParms->mxYB);
        }

     }

/*--------------------------------------------------
  This function backs the Options/Set Parameters... dialog box.

  The current parameters for the calculation engine
  are read into the local structure npTempParms
  via InitNewParms (during WM_INITDLG) and set into
  the dialog entry fields.

  Entry to any field is sensed via WM_CONTROL
  notify messages, read, interpreded as a floating
  point number, and it and other affected fields
  updated.  Currently a nonsense string is not
  explicitly detected (I assume the user is reading
  the numbers echoed to him).

  The HELP button triggers an appropriate message
  displayed via the SMPLHELP system.

  On dismiss, OK returns TRUE, Cancel returns FALSE.
  The calling code will then finish scheduling
  the parameters on an OK exit.
 --------------------------------------------------*/
MRESULT EXPENTRY SetParametersDlgProc (HWND hwnd, USHORT msg, MPARAM mp1, MPARAM mp2)
     {
     char   achResponse[33];
     double dTemp;
     USHORT usWhichField, usNoteCode;
     char   tszFloatFormat[10];

     _fstrcpy(tszFloatFormat, szFloatFormat);

     switch (msg)
          {
          case WM_CONTROL:

               usWhichField = (USHORT) SHORT1FROMMP(mp1) ;
               usNoteCode = (USHORT) SHORT2FROMMP(mp1) ;

               if ( ( (usWhichField >= IDD_SET_P0) &&
                      (usWhichField <= IDD_SET_P3) )
                     /* if its an entryfield touched */
                     /*  losing the focus */
                     && ( usNoteCode == EN_KILLFOCUS ) )
                  {
                  WinQueryDlgItemText(hwnd, usWhichField,
                        sizeof achResponse, achResponse);
                  dTemp = atof(achResponse);
                  npTempParms.param[usWhichField - IDD_SET_P0] = dTemp;
                  sprintf(achResponse, tszFloatFormat, dTemp);
                  WinSetDlgItemText(hwnd, usWhichField, achResponse);

                  return 0;

                  }


               if ( ( (usWhichField >= IDD_SET_T1CB) &&
                      (usWhichField <= IDD_SET_T4CB) )
                    /* if it is one of the combo boxes */
                    /* and the selection has changed */
                      && ( usNoteCode == CBN_LBSELECT ) )
                  {
                      /* grab the new selection */
                  npTempParms.trigndx[usWhichField - IDD_SET_T1CB] =
                     (unsigned char) WinSendDlgItemMsg(hwnd,
                          usWhichField, LM_QUERYSELECTION,
                          MPFROMSHORT(0), MPFROMP(NULL) );

                  return 0;
                  }

               break;

          case WM_INITDLG:

               CenterDialogBox(hwnd);

               InitNewParms(&npTempParms);

               LoadParams (hwnd, &npTempParms);

                   /* if no numeric parms ... */
               if ('\0' == *fractalspecific[npTempParms.iFractType].param[0])
                  {  /* ... but there are trig parms */
                  if (0 != ( (fractalspecific[npTempParms.iFractType].flags) >> 6) & 7)
                     {  /* ... start with cursor on the trig parm */
                     WinSetFocus(HWND_DESKTOP, WinWindowFromID (hwnd, IDD_SET_T1CB) );
                     return ( (MRESULT) TRUE) ;
                     }
                  else
                     {  /* ... else start with cursor on "OK" button */
                     WinSetFocus(HWND_DESKTOP, WinWindowFromID (hwnd,DID_OK) );
                     return ( (MRESULT) TRUE) ;
                     }
                  }

               return 0;

          case WM_COMMAND:
               switch (COMMANDMSG(&msg)->cmd)
                    {
                    case DID_OK:
                         npNewParms = npTempParms;
                         npNewParms.fNewParms = TRUE;
                         WinDismissDlg (hwnd, TRUE) ;
                         return 0 ;

                    case DID_CANCEL:
                         npTempParms.fNewParms = FALSE;
                         WinDismissDlg (hwnd, FALSE) ;
                         return 0 ;

                    case IDD_SET_PDEF:
                         /* copy parms from the engine table again */
                         npTempParms.param[0] = fractalspecific[npTempParms.iFractType].paramvalue[0];
                         npTempParms.param[1] = fractalspecific[npTempParms.iFractType].paramvalue[1];
                         npTempParms.param[2] = fractalspecific[npTempParms.iFractType].paramvalue[2];
                         npTempParms.param[3] = fractalspecific[npTempParms.iFractType].paramvalue[3];
                         npTempParms.trigndx[0] = SIN;
                         npTempParms.trigndx[1] = SQR;
                         npTempParms.trigndx[2] = SINH;
                         npTempParms.trigndx[3] = COSH;
                         LoadParams (hwnd, &npTempParms);

                         return 0;

                    }
               break;

          case WM_HELP:
               SimpleHelp(hab, hwnd, szTitleBar,
                       (HMODULE) 0, IDT_TEXT, IDT_HELP_PARAMS);

               return 0;

               break;
          }
     return WinDefDlgProc (hwnd, msg, mp1, mp2) ;
     }


/*--------------------------------------------------
  Local subfunction to Options/Set Parameters.

  This function loads the dialog fields for
  SetParametersDlgProc.
 --------------------------------------------------*/
static VOID LoadParams (HWND hwnd, PNEWPARAM npTempParms)
     {
     int numTrig;
     char achResponse[33];

     char   tszFloatFormat[10];

     _fstrcpy(tszFloatFormat, szFloatFormat);

     /* load each field in turn */
     WinSetDlgItemText(hwnd, IDD_SET_PFRACTALTYPE,
                       GetFractalName(npTempParms->iFractType) );

     if ('\0' != *fractalspecific[npTempParms->iFractType].param[0])
        {
        WinSetDlgItemText(hwnd, IDD_SET_P0TEXT,
             fractalspecific[npTempParms->iFractType].param[0]);
        WinEnableWindow ( WinWindowFromID(hwnd, IDD_SET_P0), TRUE);
        sprintf(achResponse, tszFloatFormat, npTempParms->param[0]);
        WinSetDlgItemText(hwnd, IDD_SET_P0, achResponse);
        }
     else
        {
        WinEnableWindow ( WinWindowFromID(hwnd, IDD_SET_P0), FALSE);
        WinSetDlgItemText(hwnd, IDD_SET_P0TEXT, szValueNotApplicable);
        WinSetDlgItemText(hwnd, IDD_SET_P0, "");
        }

     if ('\0' != *fractalspecific[npTempParms->iFractType].param[1])
        {
        WinSetDlgItemText(hwnd, IDD_SET_P1TEXT,
             fractalspecific[npTempParms->iFractType].param[1]);
        WinEnableWindow ( WinWindowFromID(hwnd, IDD_SET_P1) , TRUE);
        sprintf(achResponse, tszFloatFormat, npTempParms->param[1]);
        WinSetDlgItemText(hwnd, IDD_SET_P1, achResponse);
        }
     else
        {
        WinEnableWindow ( WinWindowFromID(hwnd, IDD_SET_P1), FALSE);
        WinSetDlgItemText(hwnd, IDD_SET_P1TEXT, szValueNotApplicable);
        WinSetDlgItemText(hwnd, IDD_SET_P1, "");
        }

     if ('\0' != *fractalspecific[npTempParms->iFractType].param[2])
        {
        WinSetDlgItemText(hwnd, IDD_SET_P2TEXT,
             fractalspecific[npTempParms->iFractType].param[2]);
        WinEnableWindow ( WinWindowFromID(hwnd, IDD_SET_P2) , TRUE);
        sprintf(achResponse, tszFloatFormat, npTempParms->param[2]);
        WinSetDlgItemText(hwnd, IDD_SET_P2, achResponse);
        }
     else
        {
        WinEnableWindow ( WinWindowFromID(hwnd, IDD_SET_P2), FALSE);
        WinSetDlgItemText(hwnd, IDD_SET_P2TEXT, szValueNotApplicable);
        WinSetDlgItemText(hwnd, IDD_SET_P2, "");
        }

     if ('\0' != *fractalspecific[npTempParms->iFractType].param[3])
        {
        WinSetDlgItemText(hwnd, IDD_SET_P3TEXT,
             fractalspecific[npTempParms->iFractType].param[3]);
        WinEnableWindow ( WinWindowFromID(hwnd, IDD_SET_P3) , TRUE);
        sprintf(achResponse, tszFloatFormat, npTempParms->param[3]);
        WinSetDlgItemText(hwnd, IDD_SET_P3, achResponse);
        }
     else
        {
        WinEnableWindow ( WinWindowFromID(hwnd, IDD_SET_P3), FALSE);
        WinSetDlgItemText(hwnd, IDD_SET_P3TEXT, szValueNotApplicable);
        WinSetDlgItemText(hwnd, IDD_SET_P3, "");
        }

     numTrig = ( (fractalspecific[npTempParms->iFractType].flags) >> 6) & 7;

     if (numTrig >= 1)
        {
        LoadTrigs(WinWindowFromID(hwnd, IDD_SET_T1CB),
                  npTempParms->trigndx[0]);
        }
     else
        {
        WinSetDlgItemText(hwnd, IDD_SET_T1TEXT, "");
        WinEnableWindow(WinWindowFromID(hwnd, IDD_SET_T1CB), FALSE);
        }

     if (numTrig >= 2)
        {
        LoadTrigs(WinWindowFromID(hwnd, IDD_SET_T2CB),
                  npTempParms->trigndx[1]);
        }
     else
        {
        WinSetDlgItemText(hwnd, IDD_SET_T2TEXT, "");
        WinEnableWindow(WinWindowFromID(hwnd, IDD_SET_T2CB), FALSE);
        }

     if (numTrig >= 3)
        {
        LoadTrigs(WinWindowFromID(hwnd, IDD_SET_T3CB),
                  npTempParms->trigndx[2]);
        }
     else
        {
        WinSetDlgItemText(hwnd, IDD_SET_T3TEXT, "");
        WinEnableWindow(WinWindowFromID(hwnd, IDD_SET_T3CB), FALSE);
        }

     if (numTrig >= 4)
        {
        LoadTrigs(WinWindowFromID(hwnd, IDD_SET_T4CB),
                  npTempParms->trigndx[3]);
        }
     else
        {
        WinSetDlgItemText(hwnd, IDD_SET_T4TEXT, "");
        WinEnableWindow(WinWindowFromID(hwnd, IDD_SET_T4CB), FALSE);
        }

     }


/*--------------------------------------------------
  Local subfunction to Options/Set Parameters.

  This function loads the Trig function selection
  combo-boxes for SetParametersDlgProc.
 --------------------------------------------------*/
static VOID LoadTrigs(HWND hwnd, unsigned char whichtrig)
     {
     int i;

     /* zilch what might be there */
     WinSendMsg(hwnd,LM_DELETEALL, MPFROMP(NULL), MPFROMP(NULL) );

     /* load'em up */
     for (i = 0; i <= MAX_TRIG; i++)
        {
        WinSendMsg(hwnd, LM_INSERTITEM, MPFROM2SHORT(LIT_END, 0),
              MPFROMP(trigfn[i].name) );
        }

     /* and indicate the current */
     WinSendMsg(hwnd, LM_SELECTITEM, MPFROMSHORT((SHORT) whichtrig),
              MPFROMSHORT(TRUE) );

     /* and ensure it is turned on */
     WinEnableWindow(hwnd, TRUE);

     /* and set the initial entryfield value */
     WinSetWindowText(hwnd, trigfn[whichtrig].name);

     }

/*--------------------------------------------------
  This function backs the Options/Set Image... dialog box.

  Current status is set at initialization.
  Use of the Screen/Window/Printer/640x480 buttons primes the
  X and Y extent fields to suggest appropriate parameters.

  Help sends an appropriate message using the
  SMPLHELP system.
 --------------------------------------------------*/
MRESULT EXPENTRY SetImageDlgProc (HWND hwnd, USHORT msg, MPARAM mp1, MPARAM mp2)
     {

     switch (msg)
          {
          case WM_INITDLG:

               CenterDialogBox(hwnd);

               InitNewParms(&npTempParms);

               /* start with current image size */
               LoadImageXandY (hwnd, npTempParms.cx, npTempParms.cy);

               /* set the current number of colors */
               switch (npTempParms.colors)
                      {
                      case 2:
                           WinSendDlgItemMsg(hwnd, IDD_IMG_C2,
                                    BM_SETCHECK, MPFROMSHORT(1), MPFROMP(NULL) );
                           break;

                      case 16:
                           WinSendDlgItemMsg(hwnd, IDD_IMG_C16,
                                    BM_SETCHECK, MPFROMSHORT(1), MPFROMP(NULL) );
                           break;

                      case 256:
                           WinSendDlgItemMsg(hwnd, IDD_IMG_C256,
                                    BM_SETCHECK, MPFROMSHORT(1), MPFROMP(NULL) );
                           break;

                      /* I don't know how another value could have been set */

                      }

               /* and go away */
               return 0;

          case WM_CONTROL:
               {
               SHORT sWho = SHORT1FROMMP(mp1);
               SHORT sWhat = SHORT2FROMMP(mp1);

               /* keep track as buttons are pushed */
               switch (sWhat)
                    {
                    case BN_CLICKED:
                         switch (sWho)
                              {
                              /* if it is a Colors button, just record the fact */
                              case IDD_IMG_C2:
                                   npTempParms.colors = 2;
                                   return 0;

                              case IDD_IMG_C16:
                                   npTempParms.colors = 16;
                                   return 0;

                              case IDD_IMG_C256:
                                   npTempParms.colors = 256;
                                   return 0;

                              }
                    }
               break;
               }

          case WM_COMMAND:
               switch (COMMANDMSG(&msg)->cmd)
                    {
                    /* if it is a Target button, set new values */
                    /* in the entry fields */
                    case IDD_IMG_WIN:
                         LoadImageXandY (hwnd, cxClient, cyClient);

                         return 0;

                    case IDD_IMG_SCR:
                         LoadImageXandY (hwnd, (SHORT) cxScreen, (SHORT) cyScreen);

                         return 0;

                    case IDD_IMG_PRT:
                         LoadImageXandY (hwnd, (SHORT) cxPrinter, (SHORT) cyPrinter);

                         return 0;

                    case IDD_IMG_VGA:
                         LoadImageXandY (hwnd, 640, 480);

                         return 0;

                    case DID_OK:
                         /* On good return, read the values from the */
                         /* X and Y entry fields, and return them.  */

                         if ( ! (WinQueryDlgItemShort(hwnd, IDD_IMG_X,
                                   &npTempParms.cx, FALSE) &&
                                 WinQueryDlgItemShort(hwnd, IDD_IMG_Y,
                                   &npTempParms.cy, FALSE) ) )
                            {
                            /* false return is non-numeric in entry field */
                            /* (user if playing with us) */
                            /* yell and scream */
                            WinMessageBox(HWND_DESKTOP, hwnd, szBadNumeric,
                                   szTitleBar, ID_BADNUMERIC,
                                   MB_OK | MB_ERROR | MB_APPLMODAL ) ;
                            return 0;
                            }

                         /* look's good.  go away. */
                         npNewParms = npTempParms;
                         npNewParms.fNewParms = TRUE;
                         WinDismissDlg (hwnd, TRUE) ;
                         return 0 ;

                    case DID_CANCEL:
                         npTempParms.fNewParms = FALSE;
                         WinDismissDlg (hwnd, FALSE) ;
                         return 0 ;
                    }
               break ;

          case WM_HELP:
               SimpleHelp(hab, hwnd, szTitleBar,
                       (HMODULE) 0, IDT_TEXT, IDT_HELP_OPERATE);

               return 0;

               break;
          }
     return WinDefDlgProc (hwnd, msg, mp1, mp2) ;
     }

/*--------------------------------------------------
  Local subfunction to SetImageDlgProc.

  This function loads the entry fields for
  Image X and Y.
 --------------------------------------------------*/
static VOID LoadImageXandY (HWND hwnd, SHORT cx, SHORT cy)
     {

     /* Load new image size */
     WinSetDlgItemShort(hwnd, IDD_IMG_X, cx, FALSE);
     WinSetDlgItemShort(hwnd, IDD_IMG_Y, cy, FALSE);

     }

/*--------------------------------------------------
  This function backs the Options/Set Options... dialog box.

  The current options for the calculation engine
  are read into the local structure npTempParms
  via InitNewParms (during WM_INITDLG) and set into
  the dialog entry fields.

  Entry to any field is sensed via WM_CONTROL
  notify messages, read, interpreded as needed.

  The HELP button triggers an appropriate message
  displayed via the SMPLHELP system.

  On dismiss, OK returns TRUE, Cancel returns FALSE.
  The calling code will then finish scheduling
  the parameters on an OK exit.
 --------------------------------------------------*/
MRESULT EXPENTRY SetOptionsDlgProc (HWND hwnd, USHORT msg, MPARAM mp1, MPARAM mp2)
     {

     BOOL   rc;

     switch (msg)
          {
          case WM_CONTROL:
               {
               SHORT sWho = SHORT1FROMMP(mp1);
               SHORT sWhat = SHORT2FROMMP(mp1);

               /* keep track as buttons are pushed */
               switch (sWhat)
                    {
                    case BN_CLICKED:
                         switch (sWho)
                              {
                              /* if it is a Passes or Math button, just record the fact */
                              case IDD_OPT_PASSES1:
                                   npTempParms.stdCalcMode = '1';
                                   return 0;

                              case IDD_OPT_PASSES2:
                                   npTempParms.stdCalcMode = '2';
                                   return 0;

                              case IDD_OPT_PASSESG:
                                   npTempParms.stdCalcMode = 'g';
                                   return 0;

                              case IDD_OPT_PASSESB:
                                   npTempParms.stdCalcMode = 'b';
                                   return 0;

                              case IDD_OPT_MATHI:
                                   npTempParms.cFloatflag = 0;
                                   return 0;

                              case IDD_OPT_MATHF:
                                   npTempParms.cFloatflag = 1;
                                   return 0;
                              }
                    }
               break;
               }


          case WM_INITDLG:
               CenterDialogBox(hwnd);

               InitNewParms(&npTempParms);

               LoadOptions (hwnd, &npTempParms);

               return ((MRESULT) TRUE);

          case WM_COMMAND:
               switch (COMMANDMSG(&msg)->cmd)
                    {
                    case DID_OK:
                         /* On good return, read the values from the */
                         /* entry fields, and return them.  */

                         rc = WinQueryDlgItemShort(hwnd, IDD_OPT_MAXITER,
                                   &npTempParms.maxiter, FALSE);
                         rc = rc && WinQueryDlgItemShort(hwnd, IDD_OPT_BIOMORF,
                                   &npTempParms.biomorph, TRUE);
                         rc = rc && WinQueryDlgItemShort(hwnd, IDD_OPT_DECOMP,
                                   &npTempParms.decomp[0], FALSE);
                         rc = rc && WinQueryDlgItemShort(hwnd, IDD_OPT_INCOLOR,
                                   &npTempParms.inside, TRUE);
                         rc = rc && WinQueryDlgItemShort(hwnd, IDD_OPT_OUTCOLR,
                                   &npTempParms.outside, TRUE);
                         rc = rc && WinQueryDlgItemShort(hwnd, IDD_OPT_DISTEST,
                                   &npTempParms.distest, FALSE);
                         if (!rc)
                            {
                            /* false return is non-numeric in entry field */
                            /* (user is f***ing with us)                  */
                            /*   yell and scream                          */
                            WinMessageBox(HWND_DESKTOP, hwnd, szBadNumeric,
                                   szTitleBar, ID_BADNUMERIC,
                                   MB_OK | MB_ERROR | MB_APPLMODAL ) ;
                            return 0;
                            }

                         /* looks good, flag it and go away */
                         npNewParms = npTempParms;
                         npNewParms.fNewParms = TRUE;
                         WinDismissDlg (hwnd, TRUE) ;
                         return 0 ;

                    case DID_CANCEL:
                         npTempParms.fNewParms = FALSE;
                         WinDismissDlg (hwnd, FALSE) ;
                         return 0 ;

                    case IDD_SET_ODEF:
                         npTempParms.stdCalcMode = 'g';
                         npTempParms.cFloatflag = 0;
                         if (npTempParms.colors > 16)
                            npTempParms.maxiter = 250;
                         else
                            npTempParms.maxiter = 150;
                         npTempParms.biomorph = -1;
                         npTempParms.decomp[0] = 0;
                         npTempParms.inside = 1;
                         npTempParms.outside = -1;
                         npTempParms.distest = 0;
                         /* refresh each diaglog box field */
                         LoadOptions (hwnd, &npTempParms);

                         return 0;
                    }
               break;

          case WM_HELP:
               SimpleHelp(hab, hwnd, szTitleBar,
                       (HMODULE) 0, IDT_TEXT, IDT_HELP_OPERATE);

               return 0;

               break;
          }
     return WinDefDlgProc (hwnd, msg, mp1, mp2) ;
     }

/*--------------------------------------------------
  Local subfunction to SetOptionsDlgProc

  This function loads the entry fields for
  SetOptionsDlgProc.
 --------------------------------------------------*/
static VOID LoadOptions (HWND hwnd, PNEWPARAM npTempParms)
     {

     /* set the current calculation mode */
     switch (npTempParms->stdCalcMode)
            {
            case '1':
                 WinSendDlgItemMsg(hwnd, IDD_OPT_PASSES1,
                          BM_SETCHECK, MPFROMSHORT(1), MPFROMP(NULL) );
                 WinSetFocus(HWND_DESKTOP, WinWindowFromID(hwnd, IDD_OPT_PASSES1) );
                 break;

            case '2':
                 WinSendDlgItemMsg(hwnd, IDD_OPT_PASSES2,
                          BM_SETCHECK, MPFROMSHORT(1), MPFROMP(NULL) );
                 WinSetFocus(HWND_DESKTOP, WinWindowFromID(hwnd, IDD_OPT_PASSES2) );
                 break;

            case 'g':
                 WinSendDlgItemMsg(hwnd, IDD_OPT_PASSESG,
                          BM_SETCHECK, MPFROMSHORT(1), MPFROMP(NULL) );
                 WinSetFocus(HWND_DESKTOP, WinWindowFromID(hwnd, IDD_OPT_PASSESG) );
                 break;

            case 'b':
                 WinSendDlgItemMsg(hwnd, IDD_OPT_PASSESB,
                          BM_SETCHECK, MPFROMSHORT(1), MPFROMP(NULL) );
                 WinSetFocus(HWND_DESKTOP, WinWindowFromID(hwnd, IDD_OPT_PASSESB) );
                 break;

            }

     /* now the math type */
     if (npTempParms->cFloatflag)
            WinSendDlgItemMsg(hwnd, IDD_OPT_MATHF,
                          BM_SETCHECK, MPFROMSHORT(1), MPFROMP(NULL) );
     else
            WinSendDlgItemMsg(hwnd, IDD_OPT_MATHI,
                          BM_SETCHECK, MPFROMSHORT(1), MPFROMP(NULL) );

     /* now load each entry field */
     WinSetDlgItemShort(hwnd, IDD_OPT_MAXITER, npTempParms->maxiter, FALSE);
     WinSetDlgItemShort(hwnd, IDD_OPT_BIOMORF, npTempParms->biomorph, TRUE);
     WinSetDlgItemShort(hwnd, IDD_OPT_DECOMP, npTempParms->decomp[0], FALSE);
     WinSetDlgItemShort(hwnd, IDD_OPT_INCOLOR, npTempParms->inside, TRUE);
     WinSetDlgItemShort(hwnd, IDD_OPT_OUTCOLR, npTempParms->outside, TRUE);
     WinSetDlgItemShort(hwnd, IDD_OPT_DISTEST, npTempParms->distest, FALSE);

     }
