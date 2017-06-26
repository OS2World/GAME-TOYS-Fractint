/*--------------------------------------------------
   PMFRDLG2.C -- FRACTINT for PM

   External Interfaces Dialogs Module

   03/30/91      Code by Donald P. Egen (with help)

   This module contains all functions backing dialog
   boxes that handle non-Fractal Calculation Engine
   actions, such as Load, Save, Print, and About.
 ---------------------------------------------------*/

#define INCL_WIN
#define INCL_GPI
#define INCL_DOS
#include <os2.h>
#include <process.h>
#include <stdlib.h>
#include <stdio.h>
#include <smplhelp.h>
#include <string.h>

#include <opendlg.h>

#include "pmfract.h"
#include "fractint.h"
#include "fractype.h"

/* Declare local subroutines */
static int get_formula_names(PGENSEL); /* get the fractal formula names */
static int get_lsys_name(PGENSEL);     /* get the Lsystem formula name */

/*--------------------------------------------------
  This function backs the About... dialog box.

  Any dismiss is a dismiss.  No significant result
  is returned.
 --------------------------------------------------*/
MRESULT EXPENTRY AboutDlgProc (HWND hwnd, USHORT msg, MPARAM mp1, MPARAM mp2)
     {
     switch (msg)
          {
          case WM_INITDLG:
               CenterDialogBox(hwnd);

               return 0;

          case WM_COMMAND:
               switch (COMMANDMSG(&msg)->cmd)
                    {
                    case DID_OK:
                    case DID_CANCEL:
                         WinDismissDlg (hwnd, TRUE) ;
                         return 0 ;
                    }
               break ;
          }
     return WinDefDlgProc (hwnd, msg, mp1, mp2) ;
     }

/*--------------------------------------------------
  This function backs the File/New... dialog box.

  The current fractal type is set during initialization.
  The user may select from the list box.
  The result is returned on OK or double-click in the list box.

  Help sends an appropriate message using the
  SMPLHELP system.
 --------------------------------------------------*/
MRESULT EXPENTRY SelFractalDlgProc (HWND hwnd, USHORT msg, MPARAM mp1, MPARAM mp2)
     {
     SHORT i, isSelected;

     switch (msg)
          {
          case WM_INITDLG:

               CenterDialogBox(hwnd);

               InitNewParms(&npTempParms);
               /* Load the list box from the list of names */
               WinSendDlgItemMsg (hwnd, IDD_SET_FRACTTYPE_LB,
                          LM_DELETEALL, MPFROMP(NULL), MPFROMP(NULL) );

               isSelected = MANDEL;  /* default? */

               for (i = 0; i < CountFractalList; i++)
                   {
                   WinSendDlgItemMsg (hwnd, IDD_SET_FRACTTYPE_LB,
                          LM_INSERTITEM, MPFROM2SHORT(LIT_END, 0),
                          MPFROMP(fractalspecific[asFracTypeList[i]].name) );
                   if (asFracTypeList[i] == npTempParms.iFractType ||
                       fractalspecific[asFracTypeList[i]].tofloat == npTempParms.iFractType)
                        isSelected = i;
                   }

               /* Flag the current Fractal Type in the list box */
               WinSendDlgItemMsg (hwnd, IDD_SET_FRACTTYPE_LB,
                          LM_SELECTITEM, MPFROMSHORT (isSelected),
                                 MPFROMSHORT(TRUE) ) ;
               /* and put it near the middle */
               WinSendDlgItemMsg (hwnd, IDD_SET_FRACTTYPE_LB,
                          LM_SETTOPINDEX,
                          MPFROMSHORT(max(isSelected-4, 0) ),
                          MPFROMP(NULL) ) ;

               return 0;

          case WM_CONTROL:
               {
               SHORT sWho = SHORT1FROMMP(mp1);
               SHORT sWhat = SHORT2FROMMP(mp1);

               switch (sWho)
                    {
                    case IDD_SET_FRACTTYPE_LB:
                         switch (sWhat)
                              {
                              case LN_ENTER:
                                   /* fake the OK button on double click */
                                   WinSendMsg(hwnd, WM_COMMAND,
                                   MPFROMSHORT(DID_OK),
                                   MPFROM2SHORT(CMDSRC_PUSHBUTTON, FALSE) ) ;
                              return 0;

                              }
                    }
               break;
               }

          case WM_COMMAND:
               switch (COMMANDMSG(&msg)->cmd)
                    {
                    DLF dlf;
                    HFILE hfDummy;

                    case DID_OK:
                         isSelected = (SHORT) WinSendDlgItemMsg(hwnd,
                             IDD_SET_FRACTTYPE_LB, LM_QUERYSELECTION,
                             MPFROMSHORT(0), MPFROMP(NULL) );
                         npTempParms.iFractType = asFracTypeList[isSelected];
                         npTempParms.mxXL = fractalspecific[npTempParms.iFractType].xmin;
                         npTempParms.mxXR = fractalspecific[npTempParms.iFractType].xmax;
                         npTempParms.mxYT = fractalspecific[npTempParms.iFractType].ymax;
                         npTempParms.mxYB = fractalspecific[npTempParms.iFractType].ymin;
                         npTempParms.param[0] = fractalspecific[npTempParms.iFractType].paramvalue[0];
                         npTempParms.param[1] = fractalspecific[npTempParms.iFractType].paramvalue[1];
                         npTempParms.param[2] = fractalspecific[npTempParms.iFractType].paramvalue[2];
                         npTempParms.param[3] = fractalspecific[npTempParms.iFractType].paramvalue[3];
                         npTempParms.trigndx[0] = SIN;
                         npTempParms.trigndx[1] = SQR;
                         npTempParms.trigndx[2] = SINH;
                         npTempParms.trigndx[3] = COSH;

                         /* zoom out to maximum range */
                         npTempParms.XL = npTempParms.mxXL;
                         npTempParms.XR = npTempParms.mxXR;
                         npTempParms.YT = npTempParms.mxYT;
                         npTempParms.YB = npTempParms.mxYB;
                         npTempParms.XCenter = (npTempParms.XL + npTempParms.XR)/2.0;
                         npTempParms.YCenter = (npTempParms.YT + npTempParms.YB)/2.0;

                         /* handle special fractal types */
                         npTempParms.fNewParms = FALSE;       /* assume cancel below */
                         switch (npTempParms.iFractType)
                            {
                            case IFS:
                                 /* get the file name for IFS */
                                 SetupDLF (&dlf, DLG_OPENDLG, &hfDummy,
                                     "\\*.ifs", szTitleBar,
                                     szIFSTitle, szIFSHelp);
                                 _fstrcpy(dlf.szOpenFile, npTempParms.szIfsFileName);

                                 if (TDF_OLDOPEN == DlgFile(hwnd, &dlf) )
                                    {
                                    /* close the dummy file handle */
                                    DosClose(hfDummy);
                                    /* set the name */
                                    _fstrcpy(npTempParms.szIfsFileName, dlf.szOpenFile);
                                    /* flag OK */
                                    npTempParms.fNewParms = TRUE;
                                    }
                                 break;

                            case IFS3D:
                                 /* get the file name for IFS3D */
                                 SetupDLF (&dlf, DLG_OPENDLG, &hfDummy,
                                     "\\*.ifs", szTitleBar,
                                     szIFS3DTitle, szIFS3DHelp);
                                 _fstrcpy(dlf.szOpenFile, npTempParms.szIfs3dFileName);

                                 if (TDF_OLDOPEN == DlgFile(hwnd, &dlf) )
                                    {
                                    /* close the dummy file handle */
                                    DosClose(hfDummy);
                                    /* set the name */
                                    _fstrcpy(npTempParms.szIfs3dFileName, dlf.szOpenFile);
                                    /* flag OK */
                                    npTempParms.fNewParms = TRUE;
                                    }
                                 break;

                            case FORMULA:
                            case FFORMULA:
                                 /* get the name of the Formula file */
                                 SetupDLF (&dlf, DLG_OPENDLG, &hfDummy,
                                     "\\*.frm", szTitleBar,
                                     szFormTitle, szFormHelp);
                                 _fstrcpy(dlf.szOpenFile, npTempParms.szFormFileName);

                                 if (TDF_OLDOPEN == DlgFile(hwnd, &dlf) )
                                    {

                                    /* close the dummy file handle */
                                    DosClose(hfDummy);
                                    /* set the name */
                                    _fstrcpy(npTempParms.szFormFileName, dlf.szOpenFile);
                                    /* read the formula names */
                                    get_formula_names(&gs);
                                    /* now select */
                                    _fstrcpy(gs.szSelected, npTempParms.szFormName);
                                    gs.pszTitle = szSelFormula;
                                    if (WinDlgBox (HWND_DESKTOP, hwnd, SelWhatDlgProc,
                                              (HMODULE) 0, IDD_SEL_LIST, &gs) )
                                       {
                                       /* set the name selected */
                                       _fstrcpy(npTempParms.szFormName, gs.szSelected );
                                       /* flag OK */
                                       npTempParms.fNewParms = TRUE;
                                       }
                                    }
                                 break;

                            case LSYSTEM:
                                 /* get the name of the L-System Formula file */
                                 SetupDLF (&dlf, DLG_OPENDLG, &hfDummy,
                                     "\\*.l", szTitleBar,
                                     szLsysTitle, szLsysHelp);
                                 _fstrcpy(dlf.szOpenFile, npTempParms.szLSysFileName);

                                 if (TDF_OLDOPEN == DlgFile(hwnd, &dlf) )
                                    {

                                    /* close the dummy file handle */
                                    DosClose(hfDummy);
                                    /* set the name */
                                    _fstrcpy(npTempParms.szLSysFileName, dlf.szOpenFile);
                                    /* read the formula names */
                                    get_lsys_name(&gs);
                                    /* now select */
                                    _fstrcpy(gs.szSelected, npTempParms.szLSysName);
                                    if (WinDlgBox (HWND_DESKTOP, hwnd, SelWhatDlgProc,
                                              (HMODULE) 0, IDD_SEL_LIST, &gs) )
                                       {
                                       /* set the name selected */
                                       _fstrcpy(npTempParms.szLSysName, gs.szSelected );
                                       npTempParms.fNewParms = TRUE;
                                       }
                                    }
                                 break;

                            default:
                                 /* just flag OK */
                                 npTempParms.fNewParms = TRUE;
                                 break;
                            }

                         npNewParms = npTempParms;
                         WinDismissDlg (hwnd, npNewParms.fNewParms) ;
                         return 0 ;

                    case DID_CANCEL:
                         npTempParms.fNewParms = FALSE;
                         WinDismissDlg (hwnd, FALSE) ;
                         return 0 ;
                    }
               break ;

          case WM_HELP:
               SimpleHelp(hab, hwnd, szTitleBar,
                       (HMODULE) 0, IDT_TEXT, IDT_HELP_TYPES);

               return 0;

               break;
          }
     return WinDefDlgProc (hwnd, msg, mp1, mp2) ;
     }

/*--------------------------------------------------
  This function backs the Options/Set Palette... dialog box.

  This dialog effects immediate change to the display
  by allowing the user to select from the palettes that
  may be available to him.

  Note that a richer (more colors) palette will be attempted
  to be translated by the GPI bitmap handling.  The result
  will be as good as the driver/display combination is capable of.

  Help sends an appropriate message using the
  SMPLHELP system.
 --------------------------------------------------*/
MRESULT EXPENTRY SetPaletteDlgProc (HWND hwnd, USHORT msg, MPARAM mp1, MPARAM mp2)
     {

     switch (msg)
          {
          case WM_INITDLG:

               CenterDialogBox(hwnd);

               /* Flick the button showing where we are now. */
               WinSendDlgItemMsg (hwnd, cp.sCurrentPalette,
                          BM_SETCHECK, MPFROMSHORT(1), MPFROMP(NULL) );
               WinSetFocus(HWND_DESKTOP, WinWindowFromID(hwnd,
                           cp.sCurrentPalette) );

               /* If we are running 2-color, turn off the
                  16-color, 256-color, and physical options.
                  They are inappropriate for 2-color mode */
               if (cp.colors < 16)
                  {
                  WinEnableWindow ( WinWindowFromID(hwnd, IDD_PAL_VGA16), FALSE);
                  WinEnableWindow ( WinWindowFromID(hwnd, IDD_PAL_VGA256), FALSE);
                  WinEnableWindow ( WinWindowFromID(hwnd, IDD_PAL_PHYS), FALSE);
                  }

               /* If we have read a user palette, turn on the user_palette
                  button.   */

               if (cp.fHaveUserPalette)
                  {
                  WinEnableWindow ( WinWindowFromID(hwnd, IDD_PAL_USER), TRUE);
                  }

               /* and go away */
               return ((MRESULT) TRUE);

          case WM_CONTROL:
               {
               SHORT sWho = SHORT1FROMMP(mp1);
               SHORT sWhat = SHORT2FROMMP(mp1);

               /* switch the palette pointer */
               /* save new state for next time */
               switch (sWhat)
                     {
                     case BN_CLICKED:
                          switch (sWho)
                               {
                               case IDD_PAL_BW:
                                    cp.pbmiMemory = &bmiColorTableBW;
                                    cp.sCurrentPalette = IDD_PAL_BW;
                                    break;
                               case IDD_PAL_WB:
                                    cp.pbmiMemory = &bmiColorTableWB;
                                    cp.sCurrentPalette = IDD_PAL_WB;
                                    break;
                               case IDD_PAL_VGA16:
                                    cp.pbmiMemory = &bmiColorTableVGA16;
                                    cp.sCurrentPalette = IDD_PAL_VGA16;
                                    break;
                               case IDD_PAL_VGA256:
                                    cp.pbmiMemory = &bmiColorTableVGA256;
                                    cp.sCurrentPalette = IDD_PAL_VGA256;
                                    break;
                               case IDD_PAL_PHYS:
                                    cp.pbmiMemory = &bmiColorTablePhys;
                                    cp.sCurrentPalette = IDD_PAL_PHYS;
                                    break;
                               case IDD_PAL_USER:
                                    cp.pbmiMemory = &bmiColorTableUser;
                                    cp.sCurrentPalette = IDD_PAL_USER;
                                    break;
                               }

                          /* now hose the display */
                          cp.fNewBits = TRUE;
                          WinInvalidateRect(cp.hwnd, NULL, FALSE);

                          /* and go away */
                          return 0;
                     }
               }

          case WM_COMMAND:
               switch (COMMANDMSG(&msg)->cmd)
                    {
                    case DID_OK:
                    case DID_CANCEL:
                         /* any exit is an exit */
                         WinDismissDlg (hwnd, TRUE) ;
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
  This function backs the File/Print... dialog box.

  We must determine if the user wants to print to the PM Printer
  or direct to an HP PaintJet.
  If the user clicks OK, we return TRUE and the main line will
  schedule the print; else return FALSE on CANCEL.

  The Help button sends an appropriate message using the
  SMPLHELP system.
 --------------------------------------------------*/
MRESULT EXPENTRY PrintOptionsDlgProc (HWND hwnd, USHORT msg, MPARAM mp1, MPARAM mp2)
     {

     char szPortName[9];
     DLF dlf;    /* OpenDlg parm */
     HFILE hfDummy;
     int rc;

     switch (msg)
          {
          case WM_INITDLG:

               CenterDialogBox(hwnd);

               /* set up the dialog box indicating the PM Printer
                  is the suggested output.  */
               WinSendDlgItemMsg(hwnd, IDD_PRINT_PM, BM_SETCHECK,
                    MPFROMSHORT(1), NULL);
               WinSendDlgItemMsg(hwnd, IDD_PRINT_PORT, BM_SETCHECK,
                    MPFROMSHORT(1), NULL);

               /* and go back */
               return 0;

          case WM_CONTROL:
               {
               SHORT sWho = SHORT1FROMMP(mp1);
               SHORT sWhat = SHORT2FROMMP(mp1);

               switch (sWho)
                    {
                    case IDD_PRINT_PM:
                         switch (sWhat)
                              {
                              case BN_CLICKED:
                                   /* if you want PM, then disable
                                      the PJ entry fields */
                                   WinEnableWindow (
                                       WinWindowFromID(hwnd, IDD_PRINT_FILE),
                                             FALSE);
                                   WinEnableWindow (
                                       WinWindowFromID(hwnd, IDD_PRINT_PORT),
                                             FALSE);
                                   WinEnableWindow (
                                       WinWindowFromID(hwnd, IDD_PRINT_PORTNAME),
                                             FALSE);
                              return 0;

                              }
                    case IDD_PRINT_PJ:
                         switch (sWhat)
                              {
                              case BN_CLICKED:
                                   /* if you want PJ, then enable
                                      the PJ entry fields */
                                   WinEnableWindow (
                                       WinWindowFromID(hwnd, IDD_PRINT_FILE),
                                             TRUE);
                                   WinEnableWindow (
                                       WinWindowFromID(hwnd, IDD_PRINT_PORT),
                                             TRUE);
                                   WinEnableWindow (
                                       WinWindowFromID(hwnd, IDD_PRINT_PORTNAME),
                                             TRUE);
                              return 0;

                              }
                    }
               break;
               }

          case WM_COMMAND:
               switch (COMMANDMSG(&msg)->cmd)
                    {
                    case DID_OK:
                         /* on OK, we need to find out what was set
                            and save it in CALCPARM. */
                         if (1 == (USHORT) WinSendDlgItemMsg(hwnd, IDD_PRINT_PM,
                                             BM_QUERYCHECK, NULL, NULL) )
                            {
                            /* we are going to PM */
                            cp.sSubFunction = SUB_PRINT_PM;
                            }
                         else
                            {
                            /* we are going to PaintJet, but File or Port? */
                            if (1 == (USHORT) WinSendDlgItemMsg(hwnd, IDD_PRINT_FILE,
                                               BM_QUERYCHECK, NULL, NULL) )
                               {
                               /* going to File */
                               SetupDLF(&dlf, DLG_SAVEDLG, &hfDummy,
                                        "\\*.prn", szTitleBar,
                                        szPrintAction, szPrintHelp);
                               _fstrcpy(dlf.szOpenFile, cp.szPrintName);
                               rc = DlgFile(hwnd, &dlf);
                               if (rc == TDF_NOSAVE)
                                  {
                                  /* user hit cancel */
                                  WinDismissDlg (hwnd, FALSE);
                                  return 0;
                                  }
                               DosClose(hfDummy);   /* close testing handle */
                               _fstrcpy(cp.szPrintName, dlf.szOpenFile);
                               cp.sSubFunction = SUB_PRINT_PJ;
                               }
                            else
                               {
                               /* we are going to PORT LPTn */
                               cp.sSubFunction = SUB_PRINT_PJ;
                               WinQueryDlgItemText(hwnd, IDD_PRINT_PORTNAME,
                                       sizeof(szPortName), szPortName);
                               _fstrcpy(cp.szPrintName, szPortName);
                               }
                            }

                         /* Indicate good return to caller */
                         WinDismissDlg (hwnd, TRUE) ;
                         return 0 ;

                    case DID_CANCEL:
                         /* just bag it on CANCEL */
                         WinDismissDlg (hwnd, FALSE) ;
                         return 0 ;
                    }
               break ;

          case WM_HELP:
               SimpleHelp(hab, hwnd, szTitleBar,
                       (HMODULE) 0, IDT_TEXT, IDT_HELP_TYPES);

               return 0;

               break;
          }
     return WinDefDlgProc (hwnd, msg, mp1, mp2) ;
     }

/*--------------------------------------------------
  This function backs the General Selection dialog box.
  It is called during processing of File/New to
  get the L-System or Formula name after a list has
  been generated from the file selected by the user.

  The user may select from the list box.
  The result is returned on OK or double-click in the list box.

  Help sends an appropriate message using the
  SMPLHELP system.
 --------------------------------------------------*/
MRESULT EXPENTRY SelWhatDlgProc (HWND hwnd, USHORT msg, MPARAM mp1, MPARAM mp2)
     {
     SHORT i, isSelected;

     switch (msg)
          {
          case WM_INITDLG:

               CenterDialogBox(hwnd);

               /* Begin by setting the Title */
               WinSetDlgItemText(hwnd, IDD_SEL_LIST_WHAT, gs.pszTitle);

               /* Load the list box from the list of names */
               WinSendDlgItemMsg (hwnd, IDD_SEL_LIST_LB,
                          LM_DELETEALL, MPFROMP(NULL), MPFROMP(NULL) );
               isSelected = 0;   /* default if old not found in current list */
               for (i = 0; i < gs.sCountNames; i++)
                   {
                   WinSendDlgItemMsg (hwnd, IDD_SEL_LIST_LB,
                          LM_INSERTITEM, MPFROM2SHORT(LIT_END, 0),
                          MPFROMP(gs.szNames[i]) );
                   if (0 == _fstrcmp(gs.szNames[i], gs.szSelected) )
                        isSelected = i;
                   }

               /* Flag the current Selection in the list box */
               WinSendDlgItemMsg (hwnd, IDD_SEL_LIST_LB,
                          LM_SELECTITEM, MPFROMSHORT (isSelected),
                                 MPFROMSHORT(TRUE) ) ;
               /* and put it near the middle */
               WinSendDlgItemMsg (hwnd, IDD_SEL_LIST_LB,
                          LM_SETTOPINDEX,
                          MPFROMSHORT(max(isSelected-4, 0) ),
                          MPFROMP(NULL) ) ;

               return 0;

          case WM_CONTROL:
               {
               SHORT sWho = SHORT1FROMMP(mp1);
               SHORT sWhat = SHORT2FROMMP(mp1);

               switch (sWho)
                    {
                    case IDD_SEL_LIST_LB:
                         switch (sWhat)
                              {
                              case LN_ENTER:
                                   /* fake the OK button on double click */
                                   WinSendMsg(hwnd, WM_COMMAND,
                                   MPFROMSHORT(DID_OK),
                                   MPFROM2SHORT(CMDSRC_PUSHBUTTON, FALSE) ) ;
                              return 0;

                              }
                    }
               break;
               }

          case WM_COMMAND:
               switch (COMMANDMSG(&msg)->cmd)
                    {
                    case DID_OK:
                         isSelected = (SHORT) WinSendDlgItemMsg(hwnd,
                             IDD_SEL_LIST_LB, LM_QUERYSELECTION,
                             MPFROMSHORT(0), MPFROMP(NULL) );

                         _fstrcpy(gs.szSelected, gs.szNames[isSelected]);

                         npTempParms.fNewParms = TRUE;
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
                       (HMODULE) 0, IDT_TEXT, IDT_HELP_TYPES);

               return 0;

               break;
          }
     return WinDefDlgProc (hwnd, msg, mp1, mp2) ;
     }

/*--------------------------------------------------
  This function backs the Load/Save Format dialog box.
  It is called during processing of File/Open or File/Save as
  to define the format before proceeding to select a file name.
  Besides setting the format for the Load or Save operation,
  it eventually serves to set the default expected extension.

  The user may select from radio buttons presented.
  The result is returned on OK.

  Help sends an appropriate message using the
  SMPLHELP system.
 --------------------------------------------------*/
MRESULT EXPENTRY LoadSaveFmtDlgProc (HWND hwnd, USHORT msg, MPARAM mp1, MPARAM mp2)
     {
     USHORT i;

     switch (msg)
          {
          case WM_INITDLG:

               CenterDialogBox(hwnd);

               /* Begin by setting the Title */
               WinSetDlgItemText(hwnd, IDD_LST_WHAT,
                          (PSZ) PVOIDFROMMP(mp2) );

               /* suggest the last format we used */
               switch (cp.sLastLoadSaveType)
                  {
                  case SUB_LOADSAVE_GIF: i = IDD_LST_GIF; break;
                  case SUB_LOADSAVE_BMP: i = IDD_LST_BMP; break;
                  case SUB_LOADSAVE_MET: i = IDD_LST_MET; break;
                  case SUB_LOADSAVE_WIN3BMP: i = IDD_LST_WIN3BMP; break;
                  case SUB_LOADSAVE_PCX: i = IDD_LST_PCX; break;
                  default: i = IDD_LST_GIF;
                           cp.sLastLoadSaveType = SUB_LOADSAVE_GIF;
                           break;
                  }

               WinSendDlgItemMsg (hwnd, i, BM_SETCHECK,
                    MPFROMSHORT((USHORT) 1), NULL);

               return 0;

          case WM_CONTROL:
               {
               SHORT sWho = SHORT1FROMMP(mp1);
               SHORT sWhat = SHORT2FROMMP(mp1);

               switch (sWhat)
                    {
                    case BN_CLICKED:
                         switch (sWho)
                              {
                              case IDD_LST_GIF:
                                   cp.sLastLoadSaveType = SUB_LOADSAVE_GIF;
                                   return 0;
                              case IDD_LST_BMP:
                                   cp.sLastLoadSaveType = SUB_LOADSAVE_BMP;
                                   return 0;
                              case IDD_LST_MET:
                                   cp.sLastLoadSaveType = SUB_LOADSAVE_MET;
                                   return 0;
                              case IDD_LST_WIN3BMP:
                                   cp.sLastLoadSaveType = SUB_LOADSAVE_WIN3BMP;
                                   return 0;
                              case IDD_LST_PCX:
                                   cp.sLastLoadSaveType = SUB_LOADSAVE_PCX;
                                   return 0;
                              }
                    }
               break;
               }

          case WM_COMMAND:
               switch (COMMANDMSG(&msg)->cmd)
                    {
                    case DID_OK:
                         WinDismissDlg (hwnd, TRUE) ;
                         return 0 ;

                    case DID_CANCEL:
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

/* --------------------------------------------------------------------- */

/*
   The following helper routine sorts the General Selection
   Names after they have been collected one of the following
   funcitons.

   Bubble Sort, Algorithm B, p. 107, Knuth, The Art of Computer
   Programming, Vol 3, Sorting and Searching.
*/

static void sel_sort(PGENSEL gs)
   {

   SHORT bound, j, t;
   char  temp[21];

   /* B1: initialize BOUND.  We correct for zero origin here. */
   bound = gs->sCountNames - 1;

   do
     {
     /* B2: Loop on j */
     for (t = -1, j = 0; j < bound; j++)
         {
         /* B3: Compare/Exchange R[j], R[j+1] */
         /*   if R[j] > R[j+1] ... */
         if (0 < _fstrcmp(gs->szNames[j], gs->szNames[j+1]) )
            {
            /* swap */
            _fstrcpy(temp, gs->szNames[j]);
            _fstrcpy(gs->szNames[j], gs->szNames[j+1]);
            _fstrcpy(gs->szNames[j+1], temp);
            t = j;
            }
         }
         bound = t;  /* B4: set for restart ... */
      }
                     /*     if we swapped anything */
      while (t >= 0);

   }

/* --------------------------------------------------------------------- */

/*
  Read a formula file, picking off the formula names.
  Formulas use the format "  name = { ... }  name = { ... } "
*/

static int get_formula_names(PGENSEL gs)  /* get the fractal formula names */
{
   int numformulas, i;
   FILE *File;
   char msg[81], tempstring[201];

   gs->szSelected[0] = 0;             /* start by declaring failure */
   for (i = 0; i < MAX_SEL; i++) {
      gs->szNames[i][0] = 0;
      }

   _fstrcpy(tempstring, npTempParms.szFormFileName);
   if((File = fopen(tempstring, "rt")) == NULL) {
      sprintf(msg,"I Can't find %s", tempstring);
      stopmsg(1,msg);
      return(-1);
   }

   numformulas = 0;
   while(fscanf(File, " %F20[^ \n\t({]", gs->szNames[numformulas]) != EOF) {
      int c;

      while(c = getc(File)) {
         if(c == EOF || c == '{' || c == '\n')
            break;
      }
      if(c == EOF)
         break;
      else if(c != '\n'){
         numformulas++;
         if (numformulas >= MAX_SEL-1) break;
skipcomments:
         if(fscanf(File, "%200[^}]", tempstring) == EOF) break;
         if (getc(File) != '}') goto skipcomments;
         if (_fstricmp(gs->szNames[numformulas-1],"") == 0 ||
             _fstricmp(gs->szNames[numformulas-1],"comment") == 0)
                 numformulas--;
      }
   }
   fclose(File);

   gs->sCountNames = numformulas;
   /* now sort the names */
   sel_sort(gs);
   return(0);
}

/* --------------------------------------------------------------------- */

static int get_lsys_name(PGENSEL gs)         /* get the Lsystem formula name */
{
   int numformulas, i;
   FILE *File;
   char buf[201], tempstring[201];

   for (i = 0; i < MAX_SEL; i++) {
      gs->szNames[i][0] = 0;
      }

   _fstrcpy(tempstring, npTempParms.szLSysFileName);
   if ((File = fopen(tempstring, "rt")) == NULL) {
      sprintf(buf,"I Can't find %s", tempstring);
      stopmsg(1,buf);
      gs->szSelected[0] = 0;
      return(-1);
      }

   numformulas = 0;
   while (1) {
      int c;
      gs->szNames[numformulas][0] = 0;
      if (fscanf(File, " %F20[^ \n\t({]", gs->szNames[numformulas]) == EOF)
         break;
      while(c = getc(File)) {
         if(c == EOF || c == '{' || c == '\n')
            break;
         }
      if(c == EOF)
         break;
      else if(c != '\n') {
skipcomments:
         if(fscanf(File, "%200[^}]", buf) == EOF) break;
         if (getc(File) != '}') goto skipcomments;
         if (_fstricmp(gs->szNames[numformulas],"") != 0 &&
             _fstricmp(gs->szNames[numformulas],"comment") != 0)
                 if (++numformulas >= MAX_SEL) break;
         }
      }
   fclose(File);

   gs->sCountNames = numformulas;
   /* now sort the names */
   sel_sort(gs);
   return(0);
}
