/*--------------------------------------------------
   PMFRTHRD.C -- FRACTINT for PM

          Background Processing Control Module
          (typically  called a Thread)

   03/17/91      Code by Donald P. Egen

   This module's main function (PMfrThread) is the
   main function for the background thread.
   It initializes and waits until posted to do
   something by the main (user/PM interface) thread.
   Part of this initialization is to let each possible
   called driver initialize, if necessary.
   One "something" to do is to shut down.
   Other "somethings" are long-running processes,
   such as the actual calculation of the Fractal
   Engine, and printing, saving, and loading.

   This background control used to be part of
   MANDCALC.  It was split off to be able to
   add in print, save, and load actions.
   (And who knows what else eventually).

   Subroutines needed for PM interface by all
   (or at least some) background thread actions
   are also in this module.

   This is architected as a single thread that can
   do one of multiple actions.  This is OK for now
   since you don't want to Print while still Calculating,
   or whatever.  The actions are mutually exclusive
   and not viable running concurrently.
 ---------------------------------------------------*/

#define INCL_WIN
#define INCL_GPI
#define INCL_DOS
#include <os2.h>
#include <process.h>
#include <stdlib.h>
#include <stdio.h>
#include <setjmp.h>
#include <float.h>
#include <malloc.h>
#include <smplhelp.h>

#include "pmfract.h"
#include "fractint.h"
#include "fractype.h"

HAB    habThread;

VOID FAR PMfrThread (VOID)
     {
     int sub_rc;

     habThread = WinInitialize(0);

     WinGetLastError(habThread);

   /* get the memory DC */
   cp.hdcMemory = DevOpenDC (habThread, OD_MEMORY, "*", 0L, NULL, NULL) ;
   /* this says that we don't have a PS yet; not one to destroy */
   cp.hpsMemory = (HPS) NULL;

     /* let each sub-driver have a crack at it now */
     PrintDriverInit();
     CalcDriverInit();

     /* Initialization here is done.
        Post main thread that we are going to work
     */

     WinPostMsg(cp.hwnd, WM_THRD_POST,
                MPFROM2SHORT(SUB_INIT_DONE, 0),
                MPFROMP(NULL) );

     for (;;)
          {  /* wait for something to do */
          DosSemWait (&cp.ulSemTrigger, SEM_INDEFINITE_WAIT) ;

          /* take an early exit for Shutdown */
          if (cp.sSubAction == SUB_ACT_TERM)
             break;

          /* posted out of wait.  do something */

          switch(cp.sSubAction)
                {
                case SUB_ACT_CALC:
                     sub_rc = CalcDriver();
                     break;
                case SUB_ACT_PRINT:
                     sub_rc = PrintDriver();
                     break;
                case SUB_ACT_SAVE:
                     sub_rc = SaveDriver();
                     break;
                case SUB_ACT_LOAD:
                     sub_rc = LoadDriver();
                     break;
                default:
                     sub_rc = 0x9999;   /* let the main task figure it out */
                     break;
                }

          /* test here for shutdown also */
          if (cp.sSubAction == SUB_ACT_TERM)
              break;

          /* not shutdown. */
          /* indicate ready for the next cycle */
          DosSemSet (&cp.ulSemTrigger) ;
          WinPostMsg (cp.hwnd, WM_THRD_POST, MPFROM2SHORT(sub_rc, 0) ,
                         MPFROMP(NULL) ) ;
          }

      /* shutdown has been triggered.
         release resources obtained by us.

         NOTE: Serialize using DosEnterCritSec so that
         the main thread is dormant until we get everything
         cleaned up and go away.  The main thread will be
         re-dispatched following our final DosExit.
      */

      DosEnterCritSec();

      if (cp.hpsMemory != (HPS) 0)
         {  /* protection from a failed WM_CREATE or IDM_GO */
         GpiSetBitmap(cp.hpsMemory, (HBITMAP) NULL);
         GpiDestroyPS(cp.hpsMemory);
         GpiDeleteBitmap (cp.hbmMemory);
         }
      cp.hpsMemory = (HPS) 0;

      if ( cp.pixels != NULL)
         hfree(cp.pixels);
      cp.pixels = NULL;

      if (cp.hdcMemory != (HDC) 0)
         DevCloseDC (cp.hdcMemory);
      cp.hdcMemory = (HDC) 0;

      /* say good-bye to PM */
      WinTerminate(habThread);

      /* flag we done */
      DosSemClear ((HSEM) &cp.ulSemSubEnded);
      /* and go away */
      DosExit(EXIT_THREAD, 0);

      }

/*
 *   Get or Resize the memory bitmap, memory presentation space,
 *   and the pixel array.
 *
 *   return FALSE on OK
 *   return TRUE on ERROR.
 *    This allows
 *        if (GetMemoryBitmap(...) )
 *              { error.... }
 */

BOOL GetMemoryBitmap (HAB hab)
     {
     SIZEL sizl;
     BITMAPINFOHEADER bmp, bmpQQ;
     RECTL rcl;
     LONG   alBmpFormats[12];
     LONG errorcode;
     int  i, j;
     LONG pm_bytes, li;
     unsigned char _huge *phc;

     /* serialize destroy/create */
     DosSemRequest((HSEM) &cp.ulSemMemPS, SEM_INDEFINITE_WAIT);

     /* set the sizl and rcl values */
     sizl.cx = (LONG) cp.cx;
     sizl.cy = (LONG) cp.cy;
     rcl.xLeft = 0;
     rcl.yBottom = 0;
     rcl.xRight = cp.cx;
     rcl.yTop = cp.cy;

     /* start by assuming 256-color */
     cp.pm_xdots = ((ULONG) cp.cx + 3) & 0xFFFFFFFC;
     cp.pixelshift_per_byte = 0;
     cp.pixels_per_byte   = 1;
     cp.pixels_per_bytem1 = 0;
     cp.cPlanes = 1;
     cp.cBitCount = 8;
     cp.sCurrentPalette = IDD_PAL_VGA256;
     cp.pbmiMemory = &bmiColorTableVGA256;

     if (cp.colors == 16) { /* Wrong guess... */
        cp.cPlanes = 1;
        cp.cBitCount = 4;
        cp.pixelshift_per_byte = 1;
        cp.pm_xdots = (((ULONG) cp.cx + 7) & 0xfffffff8);
        cp.pixels_per_byte = 2;
        cp.pixels_per_bytem1 = 1;
        cp.pm_andmask[0] = 0xf0;
        cp.pm_notmask[0] = 0x0f;
        cp.pm_bitshift[0] = 4;
        cp.pm_andmask[1] = 0x0f;
        cp.pm_notmask[1] = 0xf0;
        cp.pm_bitshift[1] = 0;
        cp.sCurrentPalette = IDD_PAL_VGA16;
        cp.pbmiMemory = &bmiColorTableVGA16;
        }

     if (cp.colors == 2) { /* ... or again ... */
        cp.cPlanes = 1;
        cp.cBitCount = 1;
        cp.pixelshift_per_byte = 3;
        cp.pm_xdots = (((ULONG) cp.cx+31) & 0xffffffe0);
        cp.pixels_per_byte = 8;
        cp.pixels_per_bytem1 = 7;
        cp.pm_andmask[0] = 0x80;
        cp.pm_notmask[0] = 0x7f;
        cp.pm_bitshift[0] = 7;
        for (i = 1; i < 8; i++) {
            cp.pm_andmask[i] = cp.pm_andmask[i-1] >> 1;
            cp.pm_notmask[i] = (cp.pm_notmask[i-1] >> 1) + 0x80;
            cp.pm_bitshift[i] = cp.pm_bitshift[i-1] - 1;
            }
        cp.sCurrentPalette = IDD_PAL_BW;
        cp.pbmiMemory = &bmiColorTableBW;
        }

     cp.pm_xbytes = cp.pm_xdots >> cp.pixelshift_per_byte;
     pm_bytes =  (LONG) cp.pm_xbytes * (LONG) cp.cy;

     errorcode = WinGetLastError(hab);

     /* destroy the old memory bitmap and ps if there is one */
     if ( cp.hpsMemory != (HPS) NULL)
        {
        GpiSetBitmap(cp.hpsMemory, (HBITMAP) NULL);
        GpiDestroyPS(cp.hpsMemory);
        GpiDeleteBitmap (cp.hbmMemory);
        }
     if ( cp.pixels != NULL)
        hfree(cp.pixels);

     /* now get the new presentation space, etc. */
     cp.hpsMemory = GpiCreatePS (hab, cp.hdcMemory, &sizl,
                    PU_PELS | GPIF_DEFAULT | GPIT_MICRO | GPIA_ASSOC);

     if (cp.hpsMemory == (HPS) 0)
        return TRUE;

     /* and create the new bitmap */
     GpiQueryDeviceBitmapFormats (cp.hpsMemory, 12L, alBmpFormats) ;
     bmp.cbFix   = sizeof bmp;
     bmp.cx      = cp.cx;
     bmp.cy      = cp.cy;
     bmp.cPlanes = cp.cPlanes;
     bmp.cBitCount = cp.cBitCount;
     cp.hbmMemory = GpiCreateBitmap (cp.hpsMemory, &bmp, 0L, NULL, NULL);
     errorcode = WinGetLastError(hab);

     if (cp.hbmMemory == (HBITMAP) 0)
           return TRUE;

     GpiSetBitmap (cp.hpsMemory, cp.hbmMemory);
     errorcode = WinGetLastError(hab);
     GpiQueryBitmapParameters(cp.hbmMemory, &bmpQQ);
     errorcode = WinGetLastError(hab);

     /* initialize and black out the bitmap */
     GpiSetMix ( cp.hpsMemory, FM_OVERPAINT) ;
     GpiSetBackMix (cp.hpsMemory, BM_LEAVEALONE) ;
     GpiCreateLogColorTable(cp.hpsMemory, LCOL_RESET | LCOL_REALIZABLE,
              LCOLF_CONSECRGB, 0L, lScreenColors, alPhysicalColors);

     WinFillRect (cp.hpsMemory, &rcl, CLR_BLACK);

     /* ** KLUDGE ** */
     /* unfortunately, if size sent to halloc is > 128K,
        then the size of each element must be a multiple of 2.
        halloc doesn't treat 1 as a multiple of 2 (although it is 2 ** 0)
     */
     pm_bytes = ( (pm_bytes + 7) & 0xfffffff8) >> 3;
     cp.pixels = (unsigned char _huge *) halloc(pm_bytes, 8);
     if (cp.pixels == NULL)
          return TRUE;

     /* the book says he clears the memory for us */
     /* for (phc = cp.pixels, li = 0; li <= pm_bytes; li++)  *phc++ = '\0'; */

     /* deserialize destroy/create */
     DosSemClear ( (HSEM) &cp.ulSemMemPS ) ;

     return FALSE;
     }

 /*
  *  Synchronous Message Processing
  *
  *  The Fractint Fractal Calculation Engine needs to issue a message
  *  and get a OK/CANCEL type response from time to time.
  *  This is difficult given that it is a queueless subthread.
  *  This function provides a synchronous interface to the main
  *  user interface thread to present a message box and post
  *  back the subthread with the user's response.
  *
  */

int FAR PMfrSyncMsg(int flags, PSZ msg)
  {

  int result;

  /* ENQ on the first semaphore to ensure that only
     one thread at a time is using this function */
  DosSemRequest( (HSEM) &cp.ulSemSyncMsg, SEM_INDEFINITE_WAIT);

  /* initialize the return semaphore to wait for a response */
  DosSemSet( (HSEM) &cp.ulSemSyncMsgDone);

  /* tell the main thread to ask the question */
  WinPostMsg(cp.hwnd, WM_SYNC_MSG, MPFROMP(msg), MPFROM2SHORT(flags, 0) );

  /* now wait for the response */
  DosSemWait( (HSEM) &cp.ulSemSyncMsgDone, SEM_INDEFINITE_WAIT);

  /* retrieve the response */
  result = cp.SyncMsgAnswer;

  /* DEQ on this resource */
  DosSemClear( (HSEM) &cp.ulSemSyncMsg);

  /* and go back */
  return result;

  }
