/*---------------------------------------------------
   PMFRACT.C -- FRACTINT for PM
          Main Module

Copyright (C) 1991 The Stone Soup Group.  FRACTINT for
Presentation Manager may be freely copied and distributed,
but may not be sold.

    We are, of course, copyrighting the code we wrote to implement
    Fractint-for-PM, and not the routines we lifted directly
    or indirectly from Microsoft's OS/2 Presentation Manager Toolkit.

GIF and "Graphics Interchange Format" are trademarks of Compuserve
Incorporated, an H&R Block Company.

   05/25/91      Code by Donald P. Egen (with help)
 ---------------------------------------------------*/

#define INCL_WIN
#define INCL_GPI
#define INCL_DOS
#include <os2.h>
#include <process.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <smplhelp.h>

#include "fractint.h"
#include "fractype.h"
/* generate global variables in this module only */
#define EXTERN
#include "pmfract.h"

/*
   **** WARNING **** WARNING **** WARNING ****

   The following is used to disable stack overflow
   checking in the runtime library routines called by
   the subthread.

*/

extern unsigned int _pascal STKHQQ;

/*--------------------------------------------------
   Main routine.

   This is strictly Presentation Manager standard
   except for initializing and terminating
   SMPLHELP.
 ---------------------------------------------------*/
void main (int argc, char *argv[])
     {
     static ULONG flFrameFlags = FCF_TITLEBAR      | FCF_SYSMENU  |
                                 FCF_SIZEBORDER    | FCF_MINMAX   |
                                 FCF_SHELLPOSITION |
                                 FCF_MENU          | FCF_ICON     |
                                 FCF_ACCELTABLE ;
     HMQ          hmq ;
     HWND         hwndFrame, hwndClient ;
     QMSG         qmsg ;

/*
   **** WARNING **** WARNING **** WARNING ****

   The following is used to disable stack overflow
   checking in the runtime library routines called by
   the subthread.

*/

     STKHQQ = 0;

     /* stash the original parameters for analysis later */
     MainArgc = argc;
     MainArgv = argv;

     hab = WinInitialize (0) ;
     hmq = WinCreateMsgQueue (hab, 0) ;

     /*
          Master initialization.
     */

     PMfrGlobalInit();

     WinRegisterClass (hab, szClientClass, ClientWndProc, CS_SIZEREDRAW, 0) ;

     /* Note:
        Subthread is attached during window create.
     */

     hwndFrame = WinCreateStdWindow (HWND_DESKTOP, 0,
                                     &flFrameFlags, szClientClass,
                                     szTitleBar ,
                                     0L, (HMODULE) 0,
                                     ID_RESOURCE, &hwndClient) ;

     if (hwndFrame != (HWND) NULL)
        {
        /* now put the window up */
        WinShowWindow(hwndFrame, TRUE);

        SimpleHelpInit (hab) ;

        SetSwitchEntry (hwndClient, szTitleBar, "");

        /* now go into the message loop */

        while (WinGetMsg (hab, &qmsg, (HWND) NULL, 0, 0))
               WinDispatchMsg (hab, &qmsg) ;

        SimpleHelpExit();

        WinDestroyWindow (hwndFrame) ;
        }
     else
        {
        WinMessageBox(HWND_DESKTOP, HWND_DESKTOP,
                      szInitFatal, szClientClass, ID_EMESSAGEBOX,
                      MB_OK | MB_ICONHAND | MB_APPLMODAL);
        }

     WinDestroyMsgQueue (hmq) ;
     WinTerminate (hab) ;
     DosExit(EXIT_PROCESS, 0) ;
     }

/*--------------------------------------------------
   Main Window Procedure.

   This is strictly Presentation Manager standard.
   Each message will be detailed.
 ---------------------------------------------------*/
MRESULT EXPENTRY ClientWndProc (HWND hwnd, USHORT msg, MPARAM mp1, MPARAM mp2)
     {

     SHORT  cxNew, cyNew;
     LONG errorcode;

     switch (msg)
          {
          case WM_CREATE:
               /*
                *  Call subroutine to do initialization.
                */

               return ((MRESULT) (PMfrWindowInit(hwnd, mp2) ? 0 : 1));


          case WM_INITMENU:
               /*
                *  Call subroutine to do update menu enable/disable.
                */
               if (UpdateMenuStatus ( hwnd, SHORT1FROMMP(mp1) ) )

                  return 0;

               break;

          case WM_TIMER:
               /*
                *  A timer pop says the subthread is calculating
                *  and the screen should be refreshed by a WM_PAINT
                *  Request a WM_PAINT.
                */
               if (sStatus == STATUS_WORKING)
                  {

                  if ( 0 == DosSemRequest((HSEM) &cp.ulSemMemPS, 100L) )
                     {   /* note: if failure at 100 ms,
                           just wait for the next opportunity */
                         /* refresh the memory bitmap from the pixel array */
                      cp.pbmiMemory->cx = cp.cx;
                      cp.pbmiMemory->cy = cp.cy;
                      cp.pbmiMemory->cPlanes = cp.cPlanes;
                      cp.pbmiMemory->cBitCount = cp.cBitCount;
                      errorcode = GpiSetBitmapBits(cp.hpsMemory, 0L, (LONG) cp.cy,
                                       cp.pixels, cp.pbmiMemory);
                      DosSemClear((HSEM) &cp. ulSemMemPS);
                      }
                  else
                      {
                      /* maybe we can get the bitmap later */
                      cp.fNewBits = TRUE;
                      }

                  WinInvalidateRect (hwnd, NULL, FALSE);
                  }
               else /* renegade timer? */
                  {
                  WinStopTimer (hab, hwnd, ID_TIMER);
                  cp.fNewBits = TRUE;   /* better safe than sorry */
                  }
               return 0 ;

          case WM_HELP:
               /* just a press of F1 gets the basic operations help */
               SimpleHelp(hab, hwnd, szTitleBar,
                       (HMODULE) 0, IDT_TEXT, IDT_HELP_OPERATE);
               return 0;

          case WM_SIZE:
               /*
                  if we are getting minimized, then the new window
                  sizes will both be zeros.
                  If so, ignore the message.
                  If the "New" values are the same as the old values,
                  also ignore the message.
               */
               cxNew = SHORT1FROMMP (mp2) ;
               cyNew = SHORT2FROMMP (mp2) ;

               if ( (cxNew == 0) && (cyNew == 0) )
                   /* being minimized */
                   {

                   flAmIcon = TRUE;

                   /* if being minimized and the calc thread is running,
                      turn off the timer interrupts.  No need in triggering
                      repaints if we are an icon. */

                   if ((sStatus == STATUS_WORKING) &&
                       (cp.sSubAction == SUB_ACT_CALC))
                      WinStopTimer(hab, hwnd, ID_TIMER);

                   return 0;
                   }

               if (flAmIcon)
                  {
                  /* if restoring, restart what was turned off,
                     if still appropriate. */
                  flAmIcon = FALSE;

                  if ((sStatus == STATUS_WORKING) &&
                      (cp.sSubAction == SUB_ACT_CALC))
                     {
                     WinStartTimer(hab, hwnd, ID_TIMER, TIMER_REPAINT);
                     cp.fNewBits = TRUE;    /* force immediate update */
                     }
                  }

               cxClient = cxNew ;
               cyClient = cyNew ;

               return 0 ;

          case WM_COMMAND:
               /* commands handled out of line */

               return  ( PMfrCommands(hwnd, msg, mp1, mp2) ) ;

          case WM_SETFOCUS:
               /*
                * On losing focus, make sure we aren't in
                * Window or Pan mode.
                */
               if ( ! ( (BOOL) SHORT1FROMMP (mp2) ) )  /* if loosing focus */
                  {
                  CancelZoomOrPan(hwnd);   /* cancel zoom or pan if going on */
                  fActive = FALSE;
                  return 0;
                  }
               else
                 {
                 fActive = TRUE;
                 }

               break;

          case WM_CHAR:
                /*
                 * Keyboard interface is incomplete
                 */

                if (CHARMSG(&msg)->fs & KC_KEYUP)
                   break;

                if (CHARMSG(&msg)->fs & KC_VIRTUALKEY)
                    {
                    if (CHARMSG(&msg)->fs & KC_CTRL)
                        {
                        switch(CHARMSG(&msg)->vkey)
                        {
                          /* case VK_HOME:
                                 return WinSendMsg (hWnd, WM_VSCROLL,
                                        MPFROMSHORT(1), MPFROM2SHORT(0, SB_SLIDERPOSITION));
                             case VK_END:
                                 return WinSendMsg (hWnd, WM_VSCROLL,
                                        MPFROMSHORT(1), MPFROM2SHORT(iMaxVert, SB_SLIDERPOSITION));
                             case VK_PAGEUP:
                                 return WinSendMsg (hWnd, WM_HSCROLL,
                                        MPFROMSHORT(1), MPFROM2SHORT(0, SB_PAGELEFT));
                             case VK_PAGEDOWN:
                                 return WinSendMsg (hWnd, WM_HSCROLL,
                                        MPFROMSHORT(1), MPFROM2SHORT(0, SB_PAGERIGHT));
                         */
                         }
                         break;
                         }

                    switch (CHARMSG(&msg)->vkey)
                         {
                     /*   case VK_HOME:
                             return WinSendMsg (hWnd, WM_HSCROLL,
                                    MPFROMSHORT(1), MPFROM2SHORT(0, SB_SLIDERPOSITION));
                        case VK_END:
                                 return WinSendMsg (hWnd, WM_HSCROLL,
                                        MPFROMSHORT(1), MPFROM2SHORT(iMaxHorz, SB_SLIDERPOSITION));
                        case VK_LEFT:
                        case VK_RIGHT:
                             return WinSendMsg (hWndHorzSB, msg, mp1, mp2) ;
                        case VK_UP:
                        case VK_DOWN:
                        case VK_PAGEUP:
                        case VK_PAGEDOWN:
                             return WinSendMsg (hWndVertSB, msg, mp1, mp2) ;
                     */
                        case VK_ESC:
                            CancelZoomOrPan(hwnd);
                            return 0;
                        }
                    }
                break ;

          case WM_MOUSEMOVE:
               /*
                * Mouse movements count if panning or window zooming.
                */
               if (fActive)
                  {
                  POINTL ptlHere;

                  ptlHere.x = SHORT1FROMMP(mp1);
                  ptlHere.y = SHORT2FROMMP(mp1);

                  if (fPan)   /* Panning */
                     {
                     if ((cxMouseTol >= labs(ptlHere.x-ptlCrossHairs.x) ) &&
                         (cyMouseTol >= labs(ptlHere.y-ptlCrossHairs.y) ) )
                        {

                        /* ignore if within tolerance */
                        return 0;

                        }
                     else
                        {   /* moved too far, switch to zooming */
                            /* lock down first point */
                        InitZoomBox((SHORT) ptlCrossHairs.x, (SHORT) ptlCrossHairs.y);
                        fGoodPan = FALSE;
                        fPan = FALSE;
                        fZoomWin = TRUE;
                        EraseCrossHairs(hwnd);
                        MoveZoomBox(hwnd, mp1);
                        return 0;
                        }
                     }

                  if (fZoomWin)   /* Maybe Zooming? */
                     {
                     MoveZoomBox (hwnd, mp1);
                     return 0;
                     }
                  }

                break;

          case WM_BUTTON1DOWN:
               /*
                * Button 1 is Pan and Zoom Window.
                */

               /* 1st click on inactive window is ignored */
               if (!fActive)
                  break;

               /* prepare for mouseing */
               if (fGoodZoom)
                  EraseZoomBox(hwnd);
               WinSetCapture (HWND_DESKTOP, hwnd);
               WinShowPointer (HWND_DESKTOP, FALSE);
               InitCrossHairs();
               MoveCrossHairs(hwnd, mp1);
               /* assume panning until we can tell otherwise */
               fPan = TRUE;
               fZoomWin = FALSE;
               fGoodZoom = FALSE;
               return 0;

               break;

          case WM_BUTTON1UP:
               /* This is the "While Mouseing" end signal */

               if (fPan)
               {  /* panning, keep the point */
                  EraseCrossHairs(hwnd);
                  fGoodPan = TRUE;
                  fGoodZoom = FALSE;
                  fPan = FALSE;
                  fZoomWin = FALSE;
                  /* release the mouse */
                  WinSetCapture (HWND_DESKTOP, (HWND) NULL);
                  WinShowPointer (HWND_DESKTOP, TRUE);
                  return 0;
               }

               if (fZoomWin)
               {  /* zooming, keep the zoom box */
                  fGoodZoom = TRUE;
                  fGoodPan = FALSE;
                  fZoomWin = FALSE;
                  fPan = FALSE;
                  /* release the mouse */
                  WinSetCapture (HWND_DESKTOP, (HWND) NULL);
                  WinShowPointer (HWND_DESKTOP, TRUE);
                  return 0;
               }

               break;

          case WM_THRD_POST:
               /* analyze an event from the background task */
               {
               SHORT subrc = SHORT1FROMMP (mp1);

               switch (subrc)
                  {
                  case SUB_STAT_START_TIMER:
                       /* this is normal Calculation start-up */
                       /* clear the screen */
                       cp.fNewBits = TRUE;
                       cp.fSuppressPaint = FALSE;
                       WinInvalidateRect (hwnd, NULL, FALSE);
                       /* and start the refresh timer */
                       WinStartTimer (hab, hwnd, ID_TIMER, TIMER_REPAINT);
                       return 0;

                  case SUB_STAT_OK:
                       /* this is normal termination */
                       {
                       if (sStatus == STATUS_NEWPARMS)
                          {
                          /* We may have aborted because of new parms, but */
                          /* due to a race condition we ended at DONE. */
                          /* in any case,                              */
                          /* fake the GO! button to restart */
                          sStatus = STATUS_READY;
                          WinPostMsg (hwnd, WM_COMMAND,
                                 MPFROMSHORT ( IDM_GO ),
                                 MPFROM2SHORT ( CMDSRC_OTHER , 0) ) ;
                          }
                       else
                          {
                          /* schedule final clean-up at WM_PAINT time */
                          sStatus = STATUS_DONE ;
                          cp.fSuppressPaint = FALSE;
                          WinInvalidateRect (hwnd, NULL, FALSE) ;
                          UpdateMenuText (hwnd, IDM_FREEZE_HALT, szFreeze);
                          EnableMenuItem (hwnd, IDM_GO, FALSE) ;
                          EnableMenuItem (hwnd, IDM_FREEZE_HALT, FALSE) ;
                          if (! flAmIcon)
                             WinStopTimer (hab, hwnd, ID_TIMER);
                          }

                       /* if this was a LOAD, update the title bar */
                       if (cp.sSubAction == SUB_ACT_LOAD)
                              SetSwitchEntry (hwnd, szTitleBar,
                                       GetFractalName(cp.iFractType) );

                       return 0 ;
                       }
                       break;

                  case SUB_INIT_DONE:
                       {
                       /* move us to working state */
                       sStatus = STATUS_WORKING;
                       EnableMenuItem(hwnd, IDM_GO, FALSE);
                       UpdateMenuText(hwnd, IDM_FREEZE_HALT, szHalt);
                       EnableMenuItem(hwnd, IDM_FREEZE_HALT, TRUE);

                       SetSwitchEntry (hwnd, szTitleBar,
                                       GetFractalName(cp.iFractType) );

                       cp.fSuppressPaint = FALSE;

                       return 0;
                       }
                       break;

                  case SUB_STAT_ABORT:
                       {
                       /* aborted by command from the user interface */
                       if (sStatus == STATUS_NEWPARMS)
                          {
                          /* if we aborted because of new parms, */
                          /* fake the GO! button to restart */
                          sStatus = STATUS_READY;
                          WinPostMsg (hwnd, WM_COMMAND,
                                 MPFROMSHORT ( IDM_GO ),
                                 MPFROM2SHORT ( CMDSRC_OTHER , 0) ) ;
                          }
                       else
                          {
                          sStatus = STATUS_DONE ;
                          cp.fSuppressPaint = FALSE;
                          WinInvalidateRect (hwnd, NULL, FALSE) ;
                          UpdateMenuText (hwnd, IDM_FREEZE_HALT, szFreeze);
                          EnableMenuItem (hwnd, IDM_GO, FALSE) ;
                          EnableMenuItem (hwnd, IDM_FREEZE_HALT, FALSE);
                          if (! flAmIcon)
                              WinStopTimer (hab, hwnd, ID_TIMER);
                          }

                       /* if this was a LOAD, update the title bar */
                       if (cp.sSubAction == SUB_ACT_LOAD)
                              SetSwitchEntry (hwnd, szTitleBar,
                                       GetFractalName(cp.iFractType) );

                       return 0 ;
                       }
                       break;

                  case SUB_STAT_FATAL:
                       {
                       /*  the subtask has given up, probably insufficient
                           memory.  we say so then go bye-bye.    */
                       WinMessageBox (HWND_DESKTOP, hwnd,
                            (PSZ) szSubtaskFatal, (PSZ) szClientClass,
                            ID_EMESSAGEBOX,
                            MB_OK | MB_ICONHAND | MB_APPLMODAL);
                       /* schedule final clean-up at WM_DESTROY time */
                       sStatus = STATUS_DONE ;
                       cp.fSuppressPaint = TRUE;
                       UpdateMenuText (hwnd, IDM_FREEZE_HALT, szFreeze);
                       EnableMenuItem (hwnd, IDM_GO, FALSE) ;
                       EnableMenuItem (hwnd, IDM_FREEZE_HALT, FALSE) ;
                       WinPostMsg(hwnd, WM_QUIT, MPFROMP(NULL), MPFROMP(NULL) );
                       return 0;
                       }
                       break;

                  default:
                       {
                       char szErrorMessage[50];
                       char tszMissingSub[35];

                       _fstrcpy(tszMissingSub, szMissingSub);
                       /*  in this case the return code is a code identifying
                           a loose end in the calculation process */
                       if (! flAmIcon)
                          WinStopTimer (hab, hwnd, ID_TIMER);
                       sprintf(szErrorMessage, tszMissingSub, subrc);
                       WinMessageBox (HWND_DESKTOP, hwnd,
                            (PSZ) szErrorMessage, (PSZ) szClientClass,
                            ID_EMESSAGEBOX,
                            MB_OK | MB_ICONASTERISK | MB_APPLMODAL);
                       /* schedule final clean-up at WM_PAINT time */
                       sStatus = STATUS_DONE ;
                       cp.fSuppressPaint = TRUE;
                       WinInvalidateRect (hwnd, NULL, FALSE) ;
                       UpdateMenuText (hwnd, IDM_FREEZE_HALT, szFreeze);
                       EnableMenuItem (hwnd, IDM_GO, FALSE) ;
                       EnableMenuItem (hwnd, IDM_FREEZE_HALT, FALSE) ;
                       return 0;
                       }
                       break;
                  }
               }
               break;

          case WM_SYNC_MSG:
               /* the subthread needs to issue a message and wait for
                  a response.  */
               {
               PSZ szSyncMsg = (PSZ) PVOIDFROMMP(mp1);
               int flags = (int) SHORT1FROMMP(mp2);
               int result;

               if (! (flags & 4)) WinAlarm(HWND_DESKTOP, WA_WARNING);

               result = MBID_OK;

               if (!(flags & 2))
                  result = WinMessageBox (
                           HWND_DESKTOP, hwnd,
                           szSyncMsg,
                           szTitleBar, ID_SYNCMSGBOX,
                           MB_ICONASTERISK | MB_OK | MB_APPLMODAL);
               else
                  result = WinMessageBox (
                           HWND_DESKTOP, hwnd,
                           szSyncMsg,
                           szTitleBar, ID_SYNCMSGBOX,
                           MB_ICONQUESTION | MB_OKCANCEL | MB_APPLMODAL);

               if (result == 0 || result == MBID_OK || result == MBID_YES)
                      cp.SyncMsgAnswer = 0;
               else
                      cp.SyncMsgAnswer = -1;

               /* post out the subtask */
               DosSemClear((HSEM) &cp.ulSemSyncMsgDone);

               /* we done */
               return 0;

               }
               break;

          case WM_PAINT:
               /* what we do is determined by our status */
               switch (sStatus)
                  {
                  case STATUS_DONE:
                     {  /* on a good run, wait till last minute to fix menu */
                     EnableMenuItem (hwnd, IDM_GO, TRUE) ;
                     EnableMenuItem (hwnd, IDM_FREEZE_HALT, TRUE) ;
                     sStatus = STATUS_READY;
                     cp.fNewBits = TRUE;
                     break;
                     }
                  }

               PMfrRepaint (hwnd);

               return 0 ;

          case WM_DESTROY:
               /*
                * Do clean up.
                * Ask the subtask to gracefully shutdown.
                */
               cp.fContinueCalc = FALSE;
               cp.sSubAction = SUB_ACT_TERM;
               DosSemClear ((HSEM) &cp.ulSemTrigger);
               DosSemWait ((HSEM) &cp.ulSemSubEnded, SEM_INDEFINITE_WAIT);

               /* recover subthread resources if it did not */
               if (cp.hpsMemory != (HPS) 0)
                  {  /* protection from a failed WM_CREATE or IDM_GO */
                  GpiSetBitmap(cp.hpsMemory, (HBITMAP) NULL);
                  GpiDestroyPS(cp.hpsMemory);
                  GpiDeleteBitmap (cp.hbmMemory);
                  }
               if ( cp.pixels != NULL)
                    hfree(cp.pixels);

               if (cp.hdcMemory != (HDC) 0)
                  DevCloseDC (cp.hdcMemory);

               GpiDestroyPS(cp.hpsScreen);

               return 0 ;
          }
     return WinDefWindowProc (hwnd, msg, mp1, mp2) ;
     }

VOID PMfrRepaint (HWND hwnd)
     {
     HPS    hps;
     POINTL aptl[4];
     RECTL  rctl;
     LONG   errorcode;

     static BOOL fLastSuppressPaint = FALSE;

     /* zap from memory bitmap to screen */
     aptl[0].x = 0;     aptl[0].y = 0;
     /* aptl[1].x = cp.cx; aptl[1].y = cp.cy;  */
     aptl[1].x = cxClient; aptl[1].y = cyClient;
     aptl[2].x = 0;     aptl[2].y = 0;
     aptl[3].x = cp.cx; aptl[3].y = cp.cy;

     /* If the cp.fSuppressPaint toggle has changed, we
        must change the appearance of the entire window.
        Therefore, invalidate the entire window on a change.
     */
     if (fLastSuppressPaint != cp.fSuppressPaint)
        {
        fLastSuppressPaint = cp.fSuppressPaint;
        WinInvalidateRect(hwnd, NULL, FALSE);
        }

     hps = WinBeginPaint(hwnd, cp.hpsScreen, &rctl) ;

     if (! cp.fSuppressPaint)
        {
        /* if cross hairs or zoom window are displayed, flash
           them off temporarily while blitting */
        if (fPan)
           DrawCrossHairsInner (hwnd);
        if (fZoomWin || fGoodZoom)
           DrawZoomBoxInner (hwnd);

        /* serialize with the subthread */
        if ( 0 == DosSemRequest((HSEM) &cp.ulSemMemPS, 100L) )
           {   /* note: if failure at 100 ms, just wait for the next PAINT */

           /* refresh the memory bitmap from the pixel array if needed */
           if (cp.fNewBits)
              {
              cp.pbmiMemory->cx = cp.cx;
              cp.pbmiMemory->cy = cp.cy;
              cp.pbmiMemory->cPlanes = cp.cPlanes;
              cp.pbmiMemory->cBitCount = cp.cBitCount;
              errorcode = GpiSetBitmapBits(cp.hpsMemory, 0L, (LONG) cp.cy,
                                    cp.pixels, cp.pbmiMemory);
              cp.fNewBits = FALSE;
              }

           GpiErase (hps);           /* black out screen first */
           /* refresh screen */
           GpiBitBlt (hps, cp.hpsMemory, 4L, aptl, ROP_SRCCOPY, BBO_IGNORE);
           errorcode = WinGetLastError(hab);

           DosSemClear((HSEM) &cp.ulSemMemPS);

           }

        if (fPan)
           DrawCrossHairsInner (hwnd);
        if (fZoomWin || fGoodZoom)
           DrawZoomBoxInner (hwnd);
        }
     else
        {
        WinQueryWindowRect(hwnd, &rctl);
        WinFillRect(hps, &rctl,
              GpiQueryColorIndex(hps, LCOLOPT_REALIZED, 0x00000000L)) ;
        WinDrawText(hps, 0xFFFF, szWorking, &rctl,
              GpiQueryColorIndex(hps, LCOLOPT_REALIZED, 0x00FFFFFFL) ,
              GpiQueryColorIndex(hps, LCOLOPT_REALIZED, 0x00000000L) ,
              DT_CENTER | DT_VCENTER);
        }

     WinEndPaint(hps);

     }

/*---------------------------------------------
   Subroutines to manage the Calcparm and Newparms structures.
 ---------------------------------------------*/
VOID InitNewParms(PNEWPARAM npTempParms)
     {
     /* if we are frozen, we already copied once at entry to Frozen state. */
     /* since we want to play repeatedly with that copy, we will not */
     /* copy again. */
     /* if we are in pending state (STATUS_NEWPARMS), we don't want */
     /* to copy parms either, because we will overlay what is scheduled. */
     /* However, let the user see what is scheduled anyhow, for now. */

     if (! ((sStatus == STATUS_FROZEN) || (sStatus == STATUS_NEWPARMS)) )
        CopyParmsToNew();

     /* if we had a temp parms structure passed to us, update it */
     /* from the npNewParms structure */

     if (npTempParms != NULL)
        *npTempParms = npNewParms;

     }

VOID    ScheduleNewParms(HWND hwnd)
     {
     /* if we are frozen, we don't want to come out until an explicit GO! */
     /* we don't have anything to do at this time */
     if (sStatus == STATUS_FROZEN)
         return;

     /* if we are currently running, we enter a Change Pending state, */
     /* and signal the subthread to find a stopping place. */
     /* we will actually restart with the new parms after the subthread */
     /* has signalled it is ready to restart (at SUB_STAT_ABORT processing). */
     /* this is also true if we are already in Change Pending state. */

     if ( (sStatus == STATUS_WORKING) || (sStatus == STATUS_NEWPARMS) )
        {
        cp.fContinueCalc = FALSE;
        EnableMenuItem (hwnd, IDM_FREEZE_HALT, FALSE);
        sStatus = STATUS_NEWPARMS;
        return;
        }

     /* otherwize, we want to start up with these new parms. */
     /* simulate hitting the GO! menu button */
     WinPostMsg (hwnd, WM_COMMAND,
                 MPFROMSHORT ( IDM_GO ),
                 MPFROM2SHORT ( CMDSRC_OTHER , 0) ) ;

     }

VOID CopyParmsToNew(VOID)
     {
     /* refresh the npNewParms from the cp base fields */
     npNewParms.fNewParms = FALSE;
     npNewParms.iFractType = cp.iFractType;
     npNewParms.cFloatflag = cp.cFloatflag;
     npNewParms.cx = cp.cx;
     npNewParms.cy = cp.cy;
     npNewParms.XL = cp.XL; npNewParms.XR = cp.XR;
     npNewParms.YT = cp.YT; npNewParms.YB = cp.YB;
     npNewParms.param[0] = cp.param[0];
     npNewParms.param[1] = cp.param[1];
     npNewParms.param[2] = cp.param[2];
     npNewParms.param[3] = cp.param[3];
     npNewParms.trigndx[0] = cp.trigndx[0];
     npNewParms.trigndx[1] = cp.trigndx[1];
     npNewParms.trigndx[2] = cp.trigndx[2];
     npNewParms.trigndx[3] = cp.trigndx[3];
     _fstrcpy(npNewParms.szFileName, cp.szFileName);
     _fstrcpy(npNewParms.szIfsFileName, cp.szIfsFileName);
     _fstrcpy(npNewParms.szIfs3dFileName, cp.szIfs3dFileName);
     _fstrcpy(npNewParms.szFormFileName, cp.szFormFileName);
     _fstrcpy(npNewParms.szFormName, cp.szFormName);
     _fstrcpy(npNewParms.szLSysFileName, cp.szLSysFileName);
     _fstrcpy(npNewParms.szLSysName, cp.szLSysName);
     _fstrcpy(npNewParms.szPrintName, cp.szPrintName);
     npNewParms.colors = cp.colors;
     npNewParms.maxiter = cp.maxiter;
     npNewParms.stdCalcMode = cp.stdCalcMode;
     npNewParms.distest = cp.distest;
     npNewParms.periodicitycheck = cp.periodicitycheck;
     npNewParms.biomorph = cp.biomorph;
     npNewParms.decomp[0] = cp.decomp[0]; npNewParms.decomp[1] = cp.decomp[1];
     npNewParms.inside = cp.inside;
     npNewParms.outside = cp.outside;

     /* always recalculate a valid center point */
     npNewParms.XCenter = (npNewParms.XL + npNewParms.XR)/2.0;
     npNewParms.YCenter = (npNewParms.YT + npNewParms.YB)/2.0;

     /* obtain the max extents from the engine table for
        reference of the selection dialogs.      */
     npNewParms.mxXL = fractalspecific[npNewParms.iFractType].xmin;
     npNewParms.mxXR = fractalspecific[npNewParms.iFractType].xmax;
     npNewParms.mxYT = fractalspecific[npNewParms.iFractType].ymax;
     npNewParms.mxYB = fractalspecific[npNewParms.iFractType].ymin;
     }

VOID CopyParmsToBase(VOID)
     {
     /* copy npNewParms to the cp base fields */
     cp.fNewParms  = npNewParms.fNewParms;
     cp.iFractType = npNewParms.iFractType;
     cp.cFloatflag = npNewParms.cFloatflag;
     cp.cx         = npNewParms.cx;
     cp.cy         = npNewParms.cy;
     cp.XL = npNewParms.XL; cp.XR = npNewParms.XR;
     cp.YT = npNewParms.YT; cp.YB = npNewParms.YB;
     cp.param[0] = npNewParms.param[0];
     cp.param[1] = npNewParms.param[1];
     cp.param[2] = npNewParms.param[2];
     cp.param[3] = npNewParms.param[3];
     cp.trigndx[0] = npNewParms.trigndx[0];
     cp.trigndx[1] = npNewParms.trigndx[1];
     cp.trigndx[2] = npNewParms.trigndx[2];
     cp.trigndx[3] = npNewParms.trigndx[3];
     _fstrcpy(cp.szFileName, npNewParms.szFileName);
     _fstrcpy(cp.szIfsFileName, npNewParms.szIfsFileName);
     _fstrcpy(cp.szIfs3dFileName, npNewParms.szIfs3dFileName);
     _fstrcpy(cp.szFormFileName, npNewParms.szFormFileName);
     _fstrcpy(cp.szFormName, npNewParms.szFormName);
     _fstrcpy(cp.szLSysFileName, npNewParms.szLSysFileName);
     _fstrcpy(cp.szLSysName, npNewParms.szLSysName);
     _fstrcpy(cp.szPrintName, npNewParms.szPrintName);
     cp.colors = npNewParms.colors;
     cp.maxiter = npNewParms.maxiter;
     cp.stdCalcMode = npNewParms.stdCalcMode;
     cp.distest = npNewParms.distest;
     cp.periodicitycheck = npNewParms.periodicitycheck;
     cp.biomorph = npNewParms.biomorph;
     cp.decomp[0] = npNewParms.decomp[0]; cp.decomp[1] = npNewParms.decomp[1];
     cp.inside = npNewParms.inside;
     cp.outside = npNewParms.outside;
     }

/*---------------------------------------------
   General use subroutines.
 ---------------------------------------------*/

/* Given a fractalspecific index, return (pointer to the)
       appropriate fractal name.
*/

PSZ GetFractalName(int iFractType)
    {

    return ((PSZ)
             (('*' == *fractalspecific[iFractType].name)
               ? fractalspecific[iFractType].name + 1
               : fractalspecific[iFractType].name )
            );
    }
