/*--------------------------------------------------------
   SMPLHELP.C -- a Simple Help Displayer system

   This DLL system is used to display a simple help window
   to go along with a main application window of choice.

   Interface is through three functions, only one of which
   is required to be called:

      SimpleHelpInit (HAB hab);    // optional one-time initialization
      SimpleHelpExit (void);       // optional clean-up call
      SimpleHelp (HAB hab, etc.);  // Required call to display
                                   // a given text resource.

   Help text is stored as user resources in the callers EXE file.
   The type and ID of these user resources is passed to SimpleHelp,
   which causes the help window to be created if it does not
   exist and the described text to be displayed.

   Completed 12/26/89     Code by Donald P. Egen
                           (but much inspired from a variety of sources)

   Version 1.01  01/02/90  DPE
                           Add a little paranoia code at initialization,
                           and do a better job of handling the
			   WinRegisterWindowDestroy processing.

   Version 1.2	 09/03/90  DPE
			   Do the reformatting as a subthread.
			   This improves responsiveness on very
			   large texts being displayed.

   Version 1.3	 09/30/90  DPE
			   Support the OS/2 1.2 Scroll bar
			   thumb size (SBM_SETTHUMBSIZE).

		 10/16/90  DPE
			   SHORTS are negative half the time!!!
			   (ARRRGGHH!!)

   Version 1.4	 03/17/91  DPE
			   Fix running off the end of the input
			   segment if the last line ends with a CR/LF
			   but there is no EOF character.
			   (ARRRGHH!! again)

  --------------------------------------------------------*/

#define INCL_WIN
#define INCL_GPI
#define INCL_DOS
#include <os2.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "smplhelp.hh"
#include "smplhelp.h"

#define SYSVAL(x)  ( (USHORT) WinQuerySysValue(HWND_DESKTOP, x) )

HMODULE hmodHelp = (HMODULE) 0;
HMODULE hmodHelpLast = (HMODULE) 0, hmodHelpNew = (HMODULE) 0;
USHORT	idTypeLast = 0, idNameLast = 0, idTypeNew = 0, idNameNew = 0;
HWND   hwndScroll ;
HWND   hwndHelpFrame = (HWND) 0, hwndHelp = (HWND) 0;
HWND   hwndCaller = (HWND) 0;
TID    tidReformat = 0;
BOOL   flReformatAgain = FALSE;
ULONG  semReformat = 0L;
SHORT  sThreadStack[STACKSIZE];
ULONG  semWindowPS = 0L;
volatile BOOL flReformatStop = FALSE;
PCHAR  pResource, pFormat ;
SEL    selResource, selFormat;
SHORT  cxClient, cyClient, cxChar, cyChar, cyDesc, cyLines,
       sScrollPos, sNumLines, sScrollMax ;
ULONG  ulResourceSize ;
BOOL   fInitOnce = FALSE, fInitWnd = FALSE;
CHAR  szClientClass [10] ;
CHAR  szTitleBarHelp [20] ;
CHAR   szWorking[15];
CHAR   szBadAttach[35];
ULONG flFrameFlags = FCF_TITLEBAR      | FCF_SYSMENU  |
                     FCF_SIZEBORDER    | FCF_MINMAX   |
		     FCF_SHELLPOSITION | FCF_NOMOVEWITHOWNER |
                     FCF_VERTSCROLL    | FCF_ICON |
                     FCF_MENU          | FCF_ACCELTABLE ;

MRESULT EXPENTRY HelpWndProc (HWND, USHORT, MPARAM, MPARAM) ;
MRESULT CALLBACK HelpAboutProc(HWND hDlg, USHORT msg, MPARAM mp1, MPARAM mp2);
static	void	 HelpReformat(void);

static void OneTimeInit (HAB hab)
     {

     if (!fInitOnce)
        {  /* one per process init - get strings, register class */
           WinLoadString (hab, hmodHelp, IDS_CLASS,
                          sizeof szClientClass, szClientClass);
           WinLoadString (hab, hmodHelp, IDS_TITLE,
			  sizeof szTitleBarHelp, szTitleBarHelp) ;
	   WinLoadString (hab, hmodHelp, IDS_WORKING,
			  sizeof szWorking, szWorking);
	   WinLoadString (hab, hmodHelp, IDS_BADATTACH,
			  sizeof szBadAttach, szBadAttach);

           WinRegisterClass (hab, szClientClass, HelpWndProc, CS_SIZEREDRAW, 0) ;
           fInitOnce = TRUE;
        }

     }


VOID CALLBACK SimpleHelpInit (HAB hab)
     {  /* Externalize the One Time Init function */

     OneTimeInit (hab);

     }

VOID CALLBACK SimpleHelp (HAB hab, HWND hwndUser, PSZ achUser,
              HMODULE hmodUser, USHORT idType, USHORT idName)
     {
     /*
        This function is the primary interface to the Simple Help System.
        It is assumed to be called from a user Client Window Proc.
        We do what is necessary to get us up and the focus of attention,
        and ready to display the requested help text.
        On return from this function, the Simple Help window is an independent
        top-level window in the same thread as the original caller.

        We can be recalled to change the help text even if we are already
        open.  This allows a user routine to have possibly multiple submenu
        entries in his "Help" menu, and the text displayed here can be
        switched by the Help menu in the original window.
     */

     CHAR szTitleText [40];
     SWP  swpFrame;
     USHORT cxScreen, cyScreen, cyIcon, cxMe, cyMe, xMe, yMe;

     /* do the One Time Initialization if it has not been done already */
     OneTimeInit(hab);

     if (!fInitWnd)
        {  /* if we don't have a window at the moment, create an empty one */
	   hwndHelpFrame = WinCreateStdWindow (HWND_DESKTOP, 0,
                                           &flFrameFlags, szClientClass, NULL,
					   0L, hmodHelp, ID_RESOURCE, &hwndHelp) ;

	   cxScreen = SYSVAL(SV_CXSCREEN);
	   cyScreen = SYSVAL(SV_CYSCREEN);
	   cyIcon = SYSVAL(SV_CYICON);

	   cxMe = cxScreen / 3;
	   cyMe = cyScreen - 2 * cyIcon;
	   xMe = cxScreen - cxMe;
	   yMe = 2 * cyIcon;
	   WinSetWindowPos (hwndHelpFrame, HWND_TOP,
		xMe, yMe, cxMe, cyMe,
		SWP_ACTIVATE | SWP_ZORDER | SWP_MOVE | SWP_SIZE | SWP_SHOW);
	   /* now put the window up */
	   WinShowWindow(hwndHelpFrame, TRUE);
           fInitWnd = TRUE;
        }

     /* Register so that that we get called when our caller goes away */
     if (hwndCaller != hwndUser)
        {  /* our caller changed.  Unregister the previous caller */
	   if (hwndCaller != (HWND) 0)
                WinRegisterWindowDestroy (hwndCaller, FALSE);
           /* now register the new guy */
           hwndCaller = hwndUser;
           WinRegisterWindowDestroy (hwndCaller, TRUE);
        }

     /* If we are minimized, restore and activate us */
     WinQueryWindowPos (hwndHelpFrame, &swpFrame);
     if ( (swpFrame.fs & SWP_MINIMIZE) && (!(swpFrame.fs & SWP_MAXIMIZE)) )
        {
        WinSetWindowPos (hwndHelpFrame, HWND_TOP,
                0, 0, 0, 0,
                SWP_RESTORE | SWP_ACTIVATE | SWP_ZORDER);
        }
     else    /* just activate and bring us to the top */
        {
        WinSetWindowPos (hwndHelpFrame, HWND_TOP,
                   0, 0, 0, 0,
                   SWP_ACTIVATE | SWP_ZORDER);
        }

     /* set up the title bar by concatenating the resource string with the
        user's argument string.  The idea is to create "Help for <ZXCVBNM>" */
     strcpy(szTitleText, szTitleBarHelp);
     strcat(szTitleText, achUser);
     WinSetWindowText (hwndHelpFrame, szTitleText);

     /* now display the text resource requested */
     WinSendMsg (hwndHelp, WM_NEW_TEXT, MPFROM2SHORT(hmodUser, 0),
                 MPFROM2SHORT(idType, idName) );

     }

VOID CALLBACK SimpleHelpExit (void)
     {  /* if we are still active, close us down */
     if (fInitWnd)
        WinSendMsg (hwndHelp, WM_CLOSE, 0L, 0L);
     }

static void ReFormat (HWND hwnd)
     {

     USHORT rc;
     CHAR szErrorMessage[35];

     if (tidReformat)
	{   /* already active, queue for another try */
	flReformatStop = TRUE;
	flReformatAgain = TRUE;
	return;
	}

     /* fire up the sub thread */
     DosSemSet((HSEM) &semReformat);
     flReformatAgain = FALSE;
     flReformatStop = FALSE;
     WinEnableWindow (hwndScroll, FALSE);
     rc = DosCreateThread (HelpReformat, &tidReformat,
			     (PBYTE) &sThreadStack[STACKSIZE]);
     if (rc == 0)
	/* chap down the  priority */
	rc = DosSetPrty(PRTYS_THREAD, PRTYC_NOCHANGE, -5, tidReformat);
     else
	{ /* clean up */
	DosSemClear((HSEM) &semReformat);
	tidReformat = 0;
	}
     if (rc != 0)
	{   /* report the bad news */
	sprintf(szErrorMessage, szBadAttach, rc);
	WinMessageBox (HWND_DESKTOP, hwnd,
	     (PSZ) szErrorMessage, (PSZ) szClientClass,
	     ID_EMESSAGEBOX,
	     MB_OK | MB_ICONASTERISK | MB_APPLMODAL);
	}

     }

MRESULT EXPENTRY HelpWndProc (HWND hwnd, USHORT msg, MPARAM mp1, MPARAM mp2)
     {
     HPS           hps ;
     POINTL        ptl;
     SHORT	   sLineLength, sLine, sScrollInc, sPaintBeg, sPaintEnd;
     RECTL         rclInvalid ;
     PCHAR         outp;
     SHORT	   cxNew, cyNew;

     switch (msg)
          {
          case WM_CREATE:

                    /*-----------------------------------------
                       Prime the pump by allocating an initial dummy
                       resource segment
                      -----------------------------------------*/

               DosAllocSeg (25, &selResource, SEG_NONSHARED);
               ulResourceSize = 20L;
               pResource = MAKEP (selResource, 0) ;
	       *pResource = AEOF;    /* set EOF at front of segment */

                    /*-----------------------------------------------
                      Allocate a segment for formatting the text into.
                      Note: Zero length means Max length (65536 bytyes)
                      -----------------------------------------------*/

               DosAllocSeg(0, &selFormat, SEG_NONSHARED) ;
               pFormat = MAKEP (selFormat, 0) ;

                    /*----------------------
                       Query character size
                      ----------------------*/

               {
                   FONTMETRICS   fm ;

                   hps = WinGetPS (hwnd) ;

                   GpiQueryFontMetrics (hps, (LONG) sizeof fm, &fm) ;
                   cxChar = (SHORT) fm.lAveCharWidth ;
                   cyChar = (SHORT) fm.lMaxBaselineExt ;
                   cyDesc = (SHORT) fm.lMaxDescender ;

                   WinReleasePS (hps) ;
               }
                    /*------------------------------------------
                       Get scroll bar handle while the getting is good
                      ------------------------------------------*/

               hwndScroll = WinWindowFromID (
                                   WinQueryWindow (hwnd, QW_PARENT, FALSE),
                                   FID_VERTSCROLL) ;

                    /*------------------------------------------
                       Get the client window size from the CREATESTRUCT
                      ------------------------------------------*/

               {
                   PCREATESTRUCT pcrst = (PCREATESTRUCT) PVOIDFROMMP (mp2);

                   cxClient = pcrst->cx;
		   cyClient = pcrst->cy;
		   cyLines  = cyClient / cyChar;
               }

               return 0 ;

          case WM_NEW_TEXT:
               /* switch to a new incoming text resource */
	       /* free the old resource segment */

	       /* we must wait for the subthread if it is running */
	       if (tidReformat)
		  {
		  flReformatStop = TRUE;
		  DosSemWait ((HSEM) &semReformat, SEM_INDEFINITE_WAIT);
		  }

               DosFreeSeg (selResource);
               hmodHelpLast = hmodHelpNew;
               idTypeLast = idTypeNew;
               idNameLast = idNameNew;

                    /*-----------------------------------------
                       Load the resource, get size and address
                      -----------------------------------------*/

               DosGetResource ( hmodHelpNew = (HMODULE) SHORT1FROMMP(mp1),
                               idTypeNew = SHORT1FROMMP(mp2),
                               idNameNew = SHORT2FROMMP(mp2),
                               &selResource) ;
               DosSizeSeg (selResource, &ulResourceSize) ;
               pResource = MAKEP (selResource, 0) ;

	       /* Reformat to initial state. */
	       /* If we waited at the top of this message,
		  this will enter queued state, and we will restart
		  when we process the WM_SUB_DONE message
		  (which should be in the message queue by now). */

               ReFormat(hwnd);

               /* force an entire screen update */
               WinInvalidateRect (hwnd, NULL, FALSE);

	       return 0 ;

	  case WM_SUB_DONE:
	       /* Reformatting is complete.
		  If there is another reformat queued, start it.
		  Else, clean up and schedule repaint */

	       tidReformat = 0;
	       flReformatStop = FALSE;

	       if (flReformatAgain)
		  {  /* fire up again */
		  flReformatAgain = FALSE;
		  ReFormat(hwnd);
		  return 0;
		  }

	       /* fix up the scroll bar */
	       sScrollMax = max (0, sNumLines - cyLines) ;
	       sScrollPos = min (0, sScrollMax) ;

	       WinSendMsg (hwndScroll, SBM_SETSCROLLBAR,
				  MPFROM2SHORT (sScrollPos, 0),
				  MPFROM2SHORT (0, sScrollMax)) ;

	       WinSendMsg (hwndScroll, SBM_SETTHUMBSIZE,
				  MPFROM2SHORT(cyLines, sNumLines), 0L);

	       WinEnableWindow (hwndScroll, sScrollMax ? TRUE : FALSE) ;

	       /* trigger screen update */

	       WinInvalidateRect (hwnd, NULL, FALSE);

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
		   return 0;

	       if ( (cxNew == cxClient) && (cyNew == cyClient) )
		   return 0;

	       /* nope, must handle the real change in window size */
	       cxClient = cxNew;
	       cyClient = cyNew;
	       cyLines	= cyClient / cyChar;

               /* reformat everything to the new size */
	       ReFormat(hwnd);

	       WinInvalidateRect (hwnd, NULL, FALSE);

               return 0 ;

          case WM_CHAR:
                if (CHARMSG(&msg)->fs & KC_KEYUP)
                   break;

                if (CHARMSG(&msg)->fs & KC_VIRTUALKEY)
                {
                    if (CHARMSG(&msg)->fs & KC_CTRL)
                    {
                        switch(CHARMSG(&msg)->vkey)
                        {    /* Note: Cntl- optional for Top/Bottom movement */
                             case VK_HOME:
                                 return WinSendMsg (hwnd, WM_VSCROLL,
                                        MPFROMSHORT(1), MPFROM2SHORT(0, SB_SLIDERPOSITION));
                             case VK_END:
                                 return WinSendMsg (hwnd, WM_VSCROLL,
                                        MPFROMSHORT(1), MPFROM2SHORT(sNumLines - 1, SB_SLIDERPOSITION));
                         }
                    break;
                    }

                    switch (CHARMSG(&msg)->vkey)
                    {   /* Note: Home/End don't need Cntl- for vert movement */
                        case VK_HOME:
                            return WinSendMsg (hwnd, WM_VSCROLL,
                                   MPFROMSHORT(1), MPFROM2SHORT(0, SB_SLIDERPOSITION));
                        case VK_END:
                            return WinSendMsg (hwnd, WM_VSCROLL,
                                   MPFROMSHORT(1), MPFROM2SHORT(sNumLines - 1, SB_SLIDERPOSITION));
                        case VK_UP:
                        case VK_DOWN:
                        case VK_PAGEUP:
                        case VK_PAGEDOWN:
                             return WinSendMsg (hwndScroll, msg, mp1, mp2) ;
                    }
                }
                break ;

          case WM_VSCROLL:
               switch (SHORT2FROMMP (mp2))
               {
                    case SB_LINEUP:
                         sScrollInc = -1 ;
                         break ;

                    case SB_LINEDOWN:
                         sScrollInc = 1 ;
                         break ;

                    case SB_PAGEUP:
			 sScrollInc = min (-1, -cyLines) ;
                         break ;

                    case SB_PAGEDOWN:
			 sScrollInc = max (1, cyLines) ;
                         break ;

                    case SB_SLIDERTRACK:
                    case SB_SLIDERPOSITION:
                         sScrollInc = SHORT1FROMMP (mp2) - sScrollPos;
                         break ;

                    default:
                         sScrollInc = 0 ;
                         break ;
               }

               sScrollInc = max (-sScrollPos,
                             min (sScrollInc, sScrollMax - sScrollPos)) ;

               if (sScrollInc != 0) ;
               {
                    sScrollPos += sScrollInc ;
                    WinScrollWindow (hwnd, 0, cyChar * sScrollInc,
				   NULL, NULL, (HRGN) 0, NULL, SW_INVALIDATERGN) ;

                    if (SHORT2FROMMP(mp2) != SB_SLIDERTRACK)
                        WinSendMsg (hwndScroll, SBM_SETPOS,
                                    MPFROMSHORT (sScrollPos), NULL) ;
                    WinUpdateWindow (hwnd) ;
               }
               return 0 ;

          case WM_SYSCOLORCHANGE:
               /* Global color change.  Trigger Paint to handle it */
               WinInvalidateRect (hwnd, NULL, FALSE);
               return 0;

           case WM_COMMAND:
                switch (COMMANDMSG(&msg)->cmd) {
                     case IDM_ABOUT:
                          {
                             HWND          hwndAbout;
                             USHORT        rc;

                             /*
                                The following kludge is required because of APAR PJ00398.
                                If ICON static controls in a dialog resource are compiled
                                into a DLL, it will not load and will cause the entire
                                dialog box to not display.  ICON resources will not load
                                into dialog boxes which are in DLLs.  The ICON, dialog box,
                                and the calling code are all in the same DLL.
                             */

			     hwndAbout = WinLoadDlg (HWND_DESKTOP, hwnd,   /* load em up */
                                         HelpAboutProc, hmodHelp, IDD_ABOUT, NULL);
                             /* fix the ICON */
                             WinSendMsg (WinWindowFromID (hwndAbout, IDC_MYICON),
                                        SM_SETHANDLE,
                                        MPFROMLONG( (LONG) WinSendMsg (hwndHelpFrame,
                                                           WM_QUERYICON, 0L, 0L) ),
                                        0L);
			     rc = WinProcessDlg(hwndAbout);   /* do the dialog box    */
			     WinDestroyWindow (hwndAbout);    /*   then zilch it      */
                          }

                          return 0;

                     case IDM_RESTORE:
                          /* flip-flop between current and last displayed
                             help text.  This allows flip-flop between
                             "Help for <ZXCVBNM>" and "Help for Help" */

                          WinPostMsg (hwnd, WM_NEW_TEXT,
                               MPFROM2SHORT(hmodHelpLast, 0),
                               MPFROM2SHORT(idTypeLast, idNameLast) );

                          return 0;

                default: return 0;
            }
            break;

          case WM_HELP:
               WinPostMsg (hwnd, WM_NEW_TEXT,
                           MPFROM2SHORT(hmodHelp, 0),
                           MPFROM2SHORT(IDT_TEXT, IDT_HELPONHELP) );
               return 0 ;

          case WM_PAINT:
               /* It's showtime, folks */

	       /* synchronize with the subthread if it is active */

	       DosSemRequest ((HSEM) &semWindowPS, SEM_INDEFINITE_WAIT);

               hps = WinBeginPaint (hwnd, NULL, &rclInvalid) ;

               /* set the foreground and background colors to the
                  system color values SYSCLR_HELPTEXT and
                  SYSCLR_HELPBACKGROUND, respectively.             */

               GpiCreateLogColorTable (hps, LCOL_RESET, LCOLF_RGB,
                              0L, 0L, NULL);
               GpiSetColor (hps, WinQuerySysColor (HWND_DESKTOP,
                                   SYSCLR_HELPTEXT, 0L) );
               WinFillRect (hps, &rclInvalid,
                                 WinQuerySysColor (HWND_DESKTOP,
                                 SYSCLR_HELPBACKGROUND, 0L) );

	       if (tidReformat)
	       {   /* reformat is active.
		      put out a "Working..." message */

		   WinQueryWindowRect(hwnd, &rclInvalid);
		   WinDrawText(hps, -1, szWorking, &rclInvalid,
			 0L, 0L, DT_CENTER | DT_VCENTER | DT_TEXTATTRS);

	       }
	       else
	       {   /* put out the real text */
		  sPaintBeg = max (0, sScrollPos +
                              (cyClient - (SHORT) rclInvalid.yTop) / cyChar) ;
		  sPaintEnd = min (sNumLines, sScrollPos +
                              (cyClient - (SHORT) rclInvalid.yBottom)
                                   / cyChar + 1) ;

		  /* Skip down to the first line to be emitted */

		  outp = pFormat;
		  for (sLine = 0; sLine < sPaintBeg; sLine++)
		  {
		      outp += strlen(outp) + 1;
		  }

		  /* Now draw what changed */

		  for (sLine = sPaintBeg ; sLine < sPaintEnd ; sLine++)
		  {
		       ptl.x = cxChar ;
		       ptl.y = cyClient - cyChar * (sLine + 1 - sScrollPos)
					+ cyDesc ;

		       sLineLength = strlen (outp);
		       GpiCharStringAt (hps, &ptl,
				 (LONG) sLineLength,
				 outp) ;

		       outp += sLineLength + 1;
		  }
		  /* That's all, folks */
	       }

	       WinEndPaint (hps) ;

	       DosSemClear ((HSEM) &semWindowPS);

               return 0 ;

          case WM_OTHERWINDOWDESTROYED:
               /* if the Other Window Destroyed is our caller, and
                  we are still around, close us down. */
               if ( fInitWnd && (hwndCaller == HWNDFROMMP(mp1)) )
                  WinSendMsg (hwnd, WM_CLOSE, 0L, 0L);
               return 0;

          case WM_CLOSE:
               /* close us down */
	       WinRegisterWindowDestroy (hwndCaller, FALSE);  /* deregister caller */
               WinDestroyWindow (hwndHelpFrame);
	       return 0;

	  case WM_DESTROY:
	       /* if the reformat thread is active, wait for it to finish */

	       if (tidReformat)
		  {
		  flReformatStop = TRUE;
		  DosSemWait ( (HSEM) &semReformat, SEM_INDEFINITE_WAIT);
		  tidReformat = 0;
		  }

               DosFreeSeg (selResource) ;
               DosFreeSeg (selFormat) ;
	       hwndHelpFrame = (HWND) 0;
	       hwndHelp = (HWND) 0;
	       hwndScroll = (HWND) 0;
	       hwndCaller = (HWND) 0;
               fInitWnd = FALSE;
	       return 0 ;
          }
     return WinDefWindowProc (hwnd, msg, mp1, mp2) ;
     }

MRESULT CALLBACK HelpAboutProc(HWND hDlg, USHORT msg, MPARAM mp1, MPARAM mp2)
/*
    About... dialog procedure
*/
{
    switch(msg) {
        case WM_COMMAND:
            switch(COMMANDMSG(&msg)->cmd) {
                case DID_OK: WinDismissDlg(hDlg, TRUE); break;
                default: break;
            }
        break;

        default: return WinDefDlgProc(hDlg, msg, mp1, mp2);
    }
    return FALSE;
}

static void HelpReformat(void)
    {

     HAB	   habThread;
     HPS	   hps ;
     PCHAR         inp, outp, olast, tptr;
     POINTL        aptlBox[TXTBOX_COUNT];
     SHORT         sLineLength,
                   sFormatWidth ;

     /* Reformat the Original Resource (Text) lines into
        lines that will display at full width in the
        current window size.                             */


     habThread = WinInitialize(0);

     olast = outp = pFormat;
     inp = pResource;
     sNumLines = sLineLength = 0;
     sFormatWidth = cxClient - 2 * cxChar;

     while ( ( ! flReformatStop )
	     && ( ( (USHORT) (inp - pResource) ) < (USHORT) ulResourceSize )
	     && (*inp != '\0') && (*inp != AEOF) )
     {
         switch (*inp)
         {
             case '\r':
                  inp++;
                  break;

             case '\n':
	     case '\t': 	     /* white space  */
             case ' ':
		  /* Look for line overflow first */

		  DosSemRequest((HSEM) &semWindowPS, SEM_INDEFINITE_WAIT);
		  hps = WinGetPS (hwndHelp);
                  GpiQueryTextBox (hps, (LONG) sLineLength,
                                   (PCH) olast,
				   TXTBOX_COUNT, aptlBox);

		  WinReleasePS(hps);
		  DosSemClear((HSEM) &semWindowPS);

                  if ((aptlBox[TXTBOX_BOTTOMRIGHT].x -
                       aptlBox[TXTBOX_BOTTOMLEFT].x) > sFormatWidth)
                  {
		     tptr = outp;    /* try to backscan for last blank	*/
                     while (sLineLength > 0)
                           {
                           sLineLength--;
                           if (*--tptr == ' ') break;
                           }
                     if (sLineLength == 0)
		     {		  /* only one word on line, take it */
			*outp++ = '\0';
                        sNumLines++;
                        olast = outp;
                        sLineLength = 0;
                     }
                     else
		     {		  /* overlay space, term line */
                        *tptr = '\0';
                        sNumLines++;
                        olast = tptr + 1;
			*outp++ = ' ';	 /* deposit blank after remainder */
                        sLineLength = outp - olast;
                      }
                  }
                  else
		  {		       /* no overflow */
                     if ( ! ( (outp == olast) && (*inp == '\n') ) )
                     {
			*outp++ = ' ';	  /* deposit the space */
                        sLineLength++;
                     }
                  }

		  if (*inp == '\n')	 /* if it was new line */
                  {
		     inp++;		 /*  consume the new line  */
					 /* check special front of */
					 /*   line cases	   */

		     if ((USHORT)(inp - pResource) >= (USHORT) ulResourceSize )
					 /* exit if at end -- no line */
					 /* exists to be at the front of */
		     {
			break;
		     }
		     else
                     if ((*inp == '#') || (*inp == '\r'))
                     {
			*outp++ = '\0';  /* force new line, double space */
                        sNumLines++;
                        *outp++ = ' ';
                        *outp++ = '\0';
                        sNumLines++;
                        olast = outp;
                        sLineLength = 0;
                        inp++;
                     }
                     else
                     if (*inp == '\\')
                     {
			*outp++ = '\0';   /* force new line, single space */
                        sNumLines++;
                        olast = outp;
                        sLineLength = 0;
                        inp++;
                     }
                     else
                     if ( (*inp == ' ') || (*inp == '\t') )
		     {			  /* leading space */
                        if (sLineLength != 0)
                        {
			   *outp++ = '\0';   /* force new line, single space */
                           sNumLines++;
                           olast = outp;
                        }
                        sLineLength = 1;
			*outp++ = ' ';	  /* and keep the space */
                        inp++;
                     }
                  }
                  else
		     inp++;	  /* just consume the white space */
                  break;

             default:
		  *outp++ = *inp++;    /* deposit the nonblank character */
                  sLineLength++;
	 }		    /* end switch */
     }		     /* end while  */

     if (*(outp-1) != '\0')
     {
	*outp++ = '\0';        /* terminate the last line if necessary */
	sNumLines++;
     }

     WinTerminate (habThread);

     /* indicate we done */
     DosEnterCritSec();
     DosSemClear(&semReformat);
     WinPostMsg (hwndHelp, WM_SUB_DONE, 0L, 0L);

     /* and exit thread */

    }
