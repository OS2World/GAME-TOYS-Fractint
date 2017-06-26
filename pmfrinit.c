/*--------------------------------------------------
   PMFRINIT.C - FRACTINT for PM
          Initialization Routines

   04/26/91	 Code by Donald P. Egen (with help)

   These subroutines handle initialization for the
   process as a whole, or the initialization of the
   window proc.
   Out of line so that they can be in a different
   segment, and therefore discarded after start-up.
 ---------------------------------------------------*/

#define INCL_WIN
#define INCL_GPI
#define INCL_DOS
#include <os2.h>
#include <process.h>
#include <stdio.h>
#include <smplhelp.h>
#include <malloc.h>
#include <string.h>

#include "pmfract.h"
#include "fractint.h"
#include "fractype.h"

int level = 0;

  /* Internal Quick-Sort Procedure.
     No, I can't use the one in the library because I need
     to sort a _FAR array in a medium model program.	   */

int compare_fractalnames( int _far *element1, int _far *element2)
{
int i, j, k;
for (i = 0; i < 100; i++) {
    j = *element1;
    k = *element2;
    if (fractalspecific[j].name[i] < fractalspecific[k].name[i])
        return(-1);
    if (fractalspecific[j].name[i] > fractalspecific[k].name[i])
        return(1);
    if (fractalspecific[j].name[i] == 0)
        return(0);
    }
return(0);
}

void Swap(int _far *i, int _far *j )

  {
      /* Swap the i-th and j-th elements */

   int x;

    /*	writeln('swapping ',i,' and ',j);  */
       x = *i;
       *i = *j;
       *j = x;
  }

int _far *Rearr(int _far *lb, int _far *ub)
  {

    /* Portion of Quick Sort - Find the Pivot value */

     /* writeln('level=',level,' rearr ub=',ub,' lb=',lb); */
       do {
	     while ((ub > lb) && (compare_fractalnames(ub, lb) >= 0))  ub = ub - 1;
	     if (ub != lb)
		    {
			 Swap(ub, lb);
			 while ((lb < ub) && (compare_fractalnames(lb, ub) <= 0))
				lb = lb + 1;
			 if (lb != ub)	Swap(lb, ub);
		    }
	   }
       while (! (lb == ub));

       return(lb);
  }

void fr_Qsort(int _far *lb, int _far *ub)
  {
      /* The actual Quick Sort procedure */

  int _far *j;

    /*	writeln('level=',level,' lb=',lb,' ub=',ub); */
       level = level+1;
       if (lb < ub)
	  {
	       j = Rearr(lb, ub);
	       if (j != &asFracTypeList[0])  fr_Qsort(lb, j-1);
	       fr_Qsort(j+1, ub);
	  }
      level = level-1;
  }

VOID	PMfrGlobalInit(VOID)
   {
     SHORT k;

     /* get some significant screen dimensions */
     cxScreen = SYSVAL(SV_CXSCREEN);
     cyScreen = SYSVAL(SV_CYSCREEN);
     cyIcon = SYSVAL(SV_CYICON);
     cxFullScreen = SYSVAL(SV_CXFULLSCREEN);
     cyFullScreen = SYSVAL(SV_CYFULLSCREEN);
     cyMenu = SYSVAL(SV_CYMENU);
     cxMouseTol = SYSVAL(SV_CXDBLCLK);
     cyMouseTol = SYSVAL(SV_CYDBLCLK);

     /* Load string values from resource file */
     WinLoadString (hab, (HMODULE) 0, IDS_ZOOM_IN,  sizeof szZoomIn,   szZoomIn );
     WinLoadString (hab, (HMODULE) 0, IDS_ZOOM_OUT, sizeof szZoomOut,  szZoomOut);
     WinLoadString (hab, (HMODULE) 0, IDS_FREEZE,   sizeof szFreeze,   szFreeze );
     WinLoadString (hab, (HMODULE) 0, IDS_HALT,     sizeof szHalt,     szHalt   );
     WinLoadString (hab, (HMODULE) 0, IDS_TITLEBAR, sizeof szTitleBar, szTitleBar);
     WinLoadString (hab, (HMODULE) 0, IDS_SWAP_MANDEL, sizeof szSwapMandel, szSwapMandel);
     WinLoadString (hab, (HMODULE) 0, IDS_SWAP_JULIA,  sizeof szSwapJulia,  szSwapJulia );
     WinLoadString (hab, (HMODULE) 0, IDS_MISSING_SUB, sizeof szMissingSub, szMissingSub);
     WinLoadString (hab, (HMODULE) 0, IDS_FLOAT_FORMAT, sizeof szFloatFormat, szFloatFormat);
     WinLoadString (hab, (HMODULE) 0, IDS_VAL_NOTAPP, sizeof szValueNotApplicable,
						      szValueNotApplicable);
     WinLoadString (hab, (HMODULE) 0, IDS_CLIENT_CLASS, sizeof szClientClass,
						      szClientClass);
     WinLoadString (hab, (HMODULE) 0, IDS_BAD_NUMERIC, sizeof szBadNumeric,
						      szBadNumeric);
     WinLoadString (hab, (HMODULE) 0, IDS_SUBTASK_FATAL, sizeof szSubtaskFatal,
						      szSubtaskFatal);
     WinLoadString (hab, (HMODULE) 0, IDS_INIT_FATAL, sizeof szInitFatal,
						      szInitFatal);
     WinLoadString (hab, (HMODULE) 0, IDS_WORKING, sizeof szWorking,
						      szWorking);
     WinLoadString (hab, (HMODULE) 0, IDS_PRINT_NOOPEN, sizeof szPrintNoOpen,
						      szPrintNoOpen);
     WinLoadString (hab, (HMODULE) 0, IDS_PRINT_ACTION, sizeof szPrintAction,
						      szPrintAction);
     WinLoadString (hab, (HMODULE) 0, IDS_PRINT_HELP, sizeof szPrintHelp,
						      szPrintHelp);
     WinLoadString (hab, (HMODULE) 0, IDS_OPEN_TITLE, sizeof szOpenTitle,
						      szOpenTitle);
     WinLoadString (hab, (HMODULE) 0, IDS_OPEN_HELP, sizeof szOpenHelp,
						      szOpenHelp);
     WinLoadString (hab, (HMODULE) 0, IDS_FORMULA_FILE, sizeof szFormTitle,
						      szFormTitle);
     WinLoadString (hab, (HMODULE) 0, IDS_FORMULA_FILE_HELP, sizeof szFormHelp,
						      szFormHelp);
     WinLoadString (hab, (HMODULE) 0, IDS_LSYSTEM_FILE, sizeof szLsysTitle,
						      szLsysTitle);
     WinLoadString (hab, (HMODULE) 0, IDS_LSYSTEM_FILE_HELP, sizeof szLsysHelp,
						      szLsysHelp);
     WinLoadString (hab, (HMODULE) 0, IDS_IFS_FILE, sizeof szIFSTitle,
						      szIFSTitle);
     WinLoadString (hab, (HMODULE) 0, IDS_IFS_FILE_HELP, sizeof szIFSHelp,
						      szIFSHelp);
     WinLoadString (hab, (HMODULE) 0, IDS_IFS3D_FILE, sizeof szIFS3DTitle,
						      szIFS3DTitle);
     WinLoadString (hab, (HMODULE) 0, IDS_IFS3D_FILE_HELP, sizeof szIFS3DHelp,
						      szIFS3DHelp);
     WinLoadString (hab, (HMODULE) 0, IDS_SEL_FORMULA, sizeof szSelFormula,
						      szSelFormula);
     WinLoadString (hab, (HMODULE) 0, IDS_SEL_LSYSTEM, sizeof szSelLsystem,
						      szSelLsystem);
     WinLoadString (hab, (HMODULE) 0, IDS_COLORMAP_TITLE, sizeof szColorMapTitle,
						      szColorMapTitle);
     WinLoadString (hab, (HMODULE) 0, IDS_COLORMAP_HELP, sizeof szColorMapHelp,
						      szColorMapHelp);
     WinLoadString (hab, (HMODULE) 0, IDS_LOAD_WHAT,	 sizeof szLoadWhatFmt,
						      szLoadWhatFmt);
     WinLoadString (hab, (HMODULE) 0, IDS_SAVE_WHAT,	 sizeof szSaveWhatFmt,
						      szSaveWhatFmt);
     WinLoadString (hab, (HMODULE) 0, IDS_EXT_GIF,	 sizeof szExtGIF,
						      szExtGIF);
     WinLoadString (hab, (HMODULE) 0, IDS_EXT_BMP,	 sizeof szExtBMP,
						      szExtBMP);
     WinLoadString (hab, (HMODULE) 0, IDS_EXT_MET,	 sizeof szExtMET,
						      szExtMET);
     WinLoadString (hab, (HMODULE) 0, IDS_EXT_PCX,	 sizeof szExtPCX,
						      szExtPCX);

     /* interogate the fractal engine driver table to find the number
	and names (in alphabetical order) of the fractals (today) */

     CountFractalList = 0;	   /* get a count the first pass */
     for (k = 0; fractalspecific[k].name != NULL; k++)
	if (fractalspecific[k].name[0] != '*' &&
	    (fractalspecific[k].flags & WINFRAC) != 0 )
		 CountFractalList++;

     asFracTypeList = (SHORT _far *) _fmalloc(CountFractalList * sizeof(SHORT) );

     CountFractalList = 0;
     for (k = 0; fractalspecific[k].name != NULL; k++)
	if (fractalspecific[k].name[0] != '*' &&
	    (fractalspecific[k].flags & WINFRAC) != 0 )
		 asFracTypeList[CountFractalList++] = k;

     fr_Qsort(&asFracTypeList[0],
	      &asFracTypeList[CountFractalList-1]);

     /* Initialize key workarea variables. */

     cp.ulSemMemPS = 0L;
     cp.ulSemTrigger = 0L;
     cp.ulSemSubEnded = 0L;
     DosSemClear(&cp.ulSemMemPS) ;	 /* indicate unowned */
     DosSemSet (&cp.ulSemTrigger) ;	 /* must wait for trigger */
     DosSemSet (&cp.ulSemSubEnded) ;	 /* not started, so not ended */
     cp.ulSemSyncMsg = 0L;
     cp.ulSemSyncMsgDone = 0L;
     cp.SyncMsgAnswer = 0;
     DosSemClear(&cp.ulSemSyncMsg);	 /* indicate unowned */
     DosSemClear(&cp.ulSemSyncMsgDone);

     cp.sSubAction = 0;
     cp.fNewParms = FALSE;
     cp.fContinueCalc = TRUE;
     cp.fNewBits = FALSE;
     cp.fSuppressPaint = TRUE;

   }

BOOL	PMfrWindowInit(HWND hwnd, MPARAM mp2)
   {
   PCREATESTRUCT pcrst = (PCREATESTRUCT) PVOIDFROMMP(mp2);
   SIZEL sizl;
   char _far *fp1;
   char _far *fp2;
   USHORT i, sOffset, sReps;
   USHORT cxMe, cyMe, xMe, yMe;
   SHORT rc;
   HWND hwndMyFrame;

   hwndMyFrame = WinQueryWindow(hwnd, QW_PARENT, FALSE);

   cp.hdcScreen = WinOpenWindowDC (hwnd);
   /* get required device characteristics */
   DevQueryCaps ( cp.hdcScreen , CAPS_VERTICAL_RESOLUTION , 1L , & cyScreenRes ) ;
   DevQueryCaps ( cp.hdcScreen , CAPS_HORIZONTAL_RESOLUTION , 1L , & cxScreenRes ) ;
   DevQueryCaps ( cp.hdcScreen , CAPS_COLORS, 1L, & lScreenColors ) ;
   sizl.cx = 0; sizl.cy = 0;
   cp.hpsScreen = GpiCreatePS (hab, cp.hdcScreen, &sizl,
		    PU_PELS | GPIF_DEFAULT | GPIT_MICRO | GPIA_ASSOC);

   /* read the physical palette.  set the screen PS log color table */
   /* to that palette.	the subthread will set the log color table */
   /* of the memory PS to the same. */
   alPhysicalColors = (ULONG _far *) _fmalloc(lScreenColors * sizeof(ULONG) ) ;
   GpiQueryRealColors(cp.hpsScreen, LCOLOPT_REALIZED,
		      0L,   lScreenColors, alPhysicalColors);
   GpiCreateLogColorTable(cp.hpsScreen, LCOL_RESET | LCOL_REALIZABLE,
	      LCOLF_CONSECRGB, 0L, lScreenColors, alPhysicalColors);

   /* set the forground and background colors so that
      displaying a 2-color image (1-bit image) gives us black and white */

   GpiSetColor(cp.hpsScreen,
		GpiQueryColorIndex(cp.hpsScreen,    /* white */
			     LCOLOPT_REALIZED, 0x00FFFFFF) );
   GpiSetBackColor(cp.hpsScreen,
		GpiQueryColorIndex(cp.hpsScreen,    /* black */
			     LCOLOPT_REALIZED, 0x00000000) );

   /* propogate the actual physical palette into the palette arrays.
      This will allow this palette to be selected later. */
   /* beginning of dest array */
   fp2 = (char _far *) &bmiColorTablePhys.argbColor[0];
   /* beginning of source array */
   fp1 = (char _far *) &alPhysicalColors[0];
   for (i = 0; i < lScreenColors; i++)
       {   /* copy base bytes for number of screen colors */
       _fmemcpy(fp2, fp1, sizeof(RGB) );
       fp1 += sizeof(ULONG);
       fp2 += sizeof(RGB);
       }
   /* now replicate the squeezed color table to 256 entries, if necessary */
   if (lScreenColors < 256L)
      {
      fp1 = (char _far *) &bmiColorTablePhys.argbColor[0];   /* source */
      sOffset = fp2 - fp1;	       /* distance we have come so far */
      sReps = 256L / lScreenColors;    /* if it ain't even, forget excess! */
      for (i = 1; i < sReps; i++)
	  {
	  _fmemcpy(fp2, fp1, sOffset); /* copy a thwak */
	  fp2 += sOffset;	       /*  then bump dest */
	  }
      }

   cp.hwnd = hwnd ;

   /* size us full width, at the top of the screen, with about
	one line of Icons showing at the bottom. */

   cxClient = cxMe = (USHORT) cxScreen - 2;
   cyClient = cyMe = (USHORT) cyScreen - 2 * ((USHORT) cyIcon);
   xMe = 0;
   yMe = 2 * ((USHORT) cyIcon);
   WinSetWindowPos (hwndMyFrame,
       HWND_TOP, xMe, yMe, cxMe, cyMe,
       /* SWP_ACTIVATE | SWP_ZORDER | */ SWP_MOVE | SWP_SIZE | SWP_SHOW);

   /* We are Active if our frame is active; else we are not active */

   fActive = (hwndMyFrame == WinQueryActiveWindow(HWND_DESKTOP, FALSE) );

   /*
      Get the hourglass and arrow pointers
   */
   hptrWait  = WinQuerySysPointer(HWND_DESKTOP, SPTR_WAIT,  FALSE);
   hptrArrow = WinQuerySysPointer(HWND_DESKTOP, SPTR_ARROW, FALSE);

   sStatus = STATUS_INIT;
   fPan = FALSE; fZoomWin = FALSE;
   fGoodPan = FALSE; fGoodZoom = FALSE;
   cp.fHaveUserPalette = FALSE;
   cp.sLastLoadSaveType = SUB_LOADSAVE_GIF;

   /* Now attach the subthread, and away we go */

   rc = DosCreateThread (PMfrThread, &tidCalc,
		    (PBYTE) &bThreadStack[STACKSIZE]);
   /* if attach was successful, set priority to low (Idle) */
   if (rc == 0)
      {
      rc = DosSetPrty(PRTYS_THREAD, PRTYC_IDLETIME,
	   0, tidCalc);
      }

   return (BOOL) ( (rc==0) ? TRUE : FALSE);

   }
