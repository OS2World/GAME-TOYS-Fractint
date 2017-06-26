/*--------------------------------------------------
   PMFRLOAD.C -- FRACTINT for PM

      Load Processor

      This module contains both the code to read
      GIF, BMP, MET, and PCX files, it also contains
      the code used to fetch objects from the clipboard.

   05/25/91      Code by Donald P. Egen (with help)

   This module's first function (LoadDriver) is the
   main load action function.

   **** WARNING **** WARNING **** WARNING ****
   The functions in this module dealing with file I/O
   are called as part of the background subthread.

   The functions dealing with Clipboard fetching
   are called as part of the main foreground PM thread.

   Serialization is something we all hope for.

 ---------------------------------------------------*/

#define INCL_WIN
#define INCL_GPI
#define INCL_DOS
#define INCL_PM
#include <os2.h>
#include <process.h>
#include <stdlib.h>
#include <stdio.h>
#include <setjmp.h>
#include <float.h>
#include <malloc.h>
#include <string.h>
#include <smplhelp.h>

#include "pmfract.h"
#include "fractint.h"
#include "fractype.h"

/* routines in this module */

static int LoadDriverGIF(VOID);
static int LoadDriverBMP(VOID);
static int LoadDriverMET(VOID);
static int LoadDriverPCX(VOID);


extern HAB    habThread;

/* engine interface routines and variables */

extern int out_line();
extern int (*outln)();
extern char readname[];

extern int xdots, ydots, colors, maxiter;
extern int ytop, ybottom, xleft, xright;

extern int fractype;
extern int invert;

extern double ftemp, xxmin, xxmax, yymin, yymax;

extern long lm, linitx, linity;
extern int maxit, bitshift, color, row, col;

extern int read_overlay(void);
extern int gifview(void);

extern char usr_stdcalcmode, stdcalcmode;
extern int  usr_distest, usr_floatflag, usr_periodicitycheck;
extern double param[4];
extern unsigned char trigndx[4];
extern void set_trig_pointers(int which);

extern unsigned char dacbox[257][3];
extern int decomp[2];
extern int biomorph, inside, outside;
extern int filexdots, fileydots, filecolors;

extern int reallyega;

/* our driver routine */

int  FAR LoadDriver (VOID)
     {

     int rc;

     /* dispatch based on sub-function - What format? */

     switch (cp.sSubFunction)
            {
            case SUB_LOADSAVE_GIF:
                 rc = LoadDriverGIF();
                 break;

            case SUB_LOADSAVE_WIN3BMP:
            case SUB_LOADSAVE_BMP:
                 rc = LoadDriverBMP();
                 break;

            case SUB_LOADSAVE_MET:
                 rc = LoadDriverMET();
                 break;

            case SUB_LOADSAVE_PCX:
                 rc = LoadDriverPCX();
                 break;

            default:
                 rc = 999;   /* let the caller figure it out */
                 break;

            }

     /* and go back */

     return rc;

     }

/*--------------------------------------------------

 Load a GIF file using the FRACTINT for DOS code.

 ---------------------------------------------------*/
static int LoadDriverGIF (VOID)
     {

     unsigned char _far *pcSrc;
     unsigned char _far *pcDest;
     unsigned char _far *fp1;
     unsigned char _far *fp2;
     USHORT sOffset, sReps;
     int i;

     _fstrcpy(readname, cp.szFileName);

     outln = out_line;

     /* read_overlay reads the header and (if present) the FRACTINT-
        specific information in the file.  All appropriate engine
        variables are set on return */
     if (read_overlay())
        { /* problem */
        return SUB_STAT_ABORT;
        }

   cp.cx = xdots = filexdots;
   cp.cy = ydots = fileydots;
   cp.colors = colors = filecolors;
   cp.iFractType = fractype;
   cp.cFloatflag = usr_floatflag;
   cp.XL = xxmin;
   cp.XR = xxmax;
   cp.YT = yymax;
   cp.YB = yymin;
   cp.param[0] = param[0];
   cp.param[1] = param[1];
   cp.param[2] = param[2];
   cp.param[3] = param[3];
   cp.trigndx[0] = trigndx[0];
   cp.trigndx[1] = trigndx[1];
   cp.trigndx[2] = trigndx[2];
   cp.trigndx[3] = trigndx[3];
   cp.stdCalcMode = usr_stdcalcmode;
   cp.distest = usr_distest;
   cp.periodicitycheck = usr_periodicitycheck;
   cp.biomorph = biomorph;
   cp.decomp[0] = decomp[0]; cp.decomp[1] = decomp[1];
   cp.inside = inside;
   cp.outside = outside;

    ytop    = 0;
    ybottom = ydots-1;
    xleft   = 0;
    xright  = xdots-1;

     /* resize the memory bitmap for the load */
     /* fall on sword if can't get memory */
     if (GetMemoryBitmap(habThread))
        return(SUB_STAT_FATAL);

     /* the following turns off the expected palette translation
        in the read code if it thinks it has a physical VGA
        running in 16 color mode.  The flag "reallyega" turns
        of this translation.
     */
     reallyega = (cp.colors == 16) ? 1 : 0;

     /* now read in the bits */
     gifview();

     /* map the colors from the file, now in "dacbox",
        into our "user palette" structure for use.

        Unfortunately, PM uses "BGR", and dacbox uses "RGB"
     */

     cp.pbmiMemory = &bmiColorTableUser;
     cp.sCurrentPalette = IDD_PAL_USER;
     pcDest = (unsigned char _far *) &(cp.pbmiMemory->argbColor[0]);
     pcSrc = (unsigned char _far *) &dacbox[0][0];

     for (i = 0; i < cp.colors; i++)
         {
         pcDest[0] = pcSrc[2] << 2U;
         pcDest[1] = pcSrc[1] << 2U;
         pcDest[2] = pcSrc[0] << 2U;

         pcSrc += 3;
         pcDest += 3;
         }

   /* now replicate the color table to 256 entries, if necessary */
   if (cp.colors < 256)
      {
      fp1 = (unsigned char _far *) &bmiColorTableUser.argbColor[0]; /* source */
      fp2 = pcDest;                                          /* dest */
      sOffset = fp2 - fp1;             /* distance we have come so far */
      sReps = 256 / cp.colors;         /* if it ain't even, forget excess! */
      for (i = 1; i < sReps; i++)
          {
          _fmemcpy(fp2, fp1, sOffset); /* copy a thwak */
          fp2 += sOffset;              /*  then bump dest */
          }
      }

     cp.fHaveUserPalette = TRUE;

     /* and go home */

     return (SUB_STAT_OK);
     }

  /* ------------------------------------------------------------------ */

  /* The following structures map the Windows 3.0 bitmap file format. */

typedef struct tagRGBQUAD {
        CHAR    rgbBlue;
        CHAR    rgbGreen;
        CHAR    rgbRed;
        CHAR    rgbReserved;
} RGBQUAD;

typedef struct tagBITMAPINFOHEADER{
        ULONG      biSize;
        ULONG      biWidth;
        ULONG      biHeight;
        USHORT     biPlanes;
        USHORT     biBitCount;

        ULONG      biCompression;
        ULONG      biSizeImage;
        ULONG      biXPelsPerMeter;
        ULONG      biYPelsPerMeter;
        ULONG      biClrUsed;
        ULONG      biClrImportant;
} W3BITMAPINFOHEADER;

static int LoadDriverBMP(VOID)
     {

     /* NOTE: This function attempts to read both OS/2 1.1/1.2 PM
        Bitmaps as well as Windows 3.0 Bitmaps.
        The reason is that a user can't, from the outside, tell
        one from the other ( at least I can't).                    */

     BITMAPFILEHEADER bfhIt;
     HFILE hfileIt;
     W3BITMAPINFOHEADER bmpIt3;

     /* note: we will be using direct DOS reads rather than C file I/O
        because we need to deal with far (and huge) addresses */
     LONG cbColorTable, cbBitSize;
     unsigned char _far *fp1;
     unsigned char _far *fp2;
     unsigned char _huge *hp;
     RGB _far *prgb1;
     USHORT sOffset, sReps;
     ULONG  ulNewPtr;
     USHORT usIOAction;
     int i;
     FILESTATUS fstsIt;
     char achMsg[65];
     USHORT cxReal, cyReal, bitsReal, planesReal, cbColors;
     BOOL OS2fmt = TRUE;

   _fstrcpy(readname, cp.szFileName);

   /* begin by opening the file */
   if ( 0 != DosOpen(cp.szFileName, &hfileIt, &usIOAction, 0L, 0,
             FILE_OPEN, OPEN_ACCESS_READONLY | OPEN_SHARE_DENYNONE, 0L) )
      {
      return SUB_STAT_ABORT;
      }

   /* read in the file header.  This will give us format and color table size */
   if (0 != DosRead(hfileIt, &bfhIt, sizeof(BITMAPFILEHEADER),
                    &usIOAction) )
      {
      DosClose(hfileIt);
      return SUB_STAT_ABORT;
      }

   /* check that this is a "BM" file.  We could get into trouble
      if we arbitrarily try to read something else.                 */
   if (bfhIt.usType != BFT_BMAP)
      {
      DosClose(hfileIt);
      sprintf(achMsg, "%s is not a Bitmap File!", readname);
      PMfrSyncMsg(0, achMsg);    /* flag the new information */
      return SUB_STAT_OK;
      }

   /* *** WARNING *** DON'T DEPEND ON THE cbSize figure from
      the Bitmap header.  It may be wrong.  Get the file size from
      the file system.                                              */
   DosQFileInfo(hfileIt, FIL_STANDARD, &fstsIt, sizeof(fstsIt));

   /* determine if this is OS/2 1.1/1.2/1.3 format or Windows 3.0 format.
      If it is a Windows bitmap, reposition and reread the BITMAPINFOHEADER
      part of the file.  Copy width, heigth, planes, and bits-per-pixel
      to common variables for use later.  Let the file positioned
      at the beginning of the color table.                          */
   if (bfhIt.bmp.cbFix == sizeof(BITMAPINFOHEADER) )
      {   /* its OS/2 */

      cxReal = bfhIt.bmp.cx;
      cyReal = bfhIt.bmp.cy;
      bitsReal = bfhIt.bmp.cBitCount;
      planesReal = bfhIt.bmp.cPlanes;
      OS2fmt = TRUE;
      /* calculate the color table size */
      cbColorTable = bfhIt.offBits - (ULONG) sizeof(BITMAPFILEHEADER);
      }
   else
      {   /* it's Windows 3.0 */
      /* rewind the file to the beginning of the BitmapInfoHeader area */
      if (0 != DosChgFilePtr(hfileIt,
                    sizeof(BITMAPFILEHEADER) - sizeof(BITMAPINFOHEADER),
                    FILE_BEGIN, &ulNewPtr) )
         {
         DosClose(hfileIt);
         return SUB_STAT_ABORT;
         }
      /* now read it */
      if ( 0 != DosRead(hfileIt, &bmpIt3,
                  sizeof(W3BITMAPINFOHEADER), &usIOAction) )
         {
         DosClose(hfileIt);
         return SUB_STAT_ABORT;
         }

      OS2fmt = FALSE;
      cxReal = bmpIt3.biWidth;
      cyReal = bmpIt3.biHeight;
      bitsReal = bmpIt3.biBitCount;
      planesReal = bmpIt3.biPlanes;

      cbColorTable = bfhIt.offBits - (ULONG) (sizeof(BITMAPFILEHEADER)
                           - sizeof(BITMAPINFOHEADER) )
                           - (ULONG) sizeof(W3BITMAPINFOHEADER);
      }

   /* calculate the actual bit values size */
   cbBitSize = fstsIt.cbFile - bfhIt.offBits;

   /* ensure cp and engine variables have reasonable values */

   cp.cx = xdots = filexdots = cxReal;
   cp.cy = ydots = fileydots = cyReal;
            /* note: standard bitmap assumed : cPlanes = 1 */
   cp.colors = colors = filecolors = cbColors = 1 << bitsReal;
   cp.iFractType = fractype = PLASMA;
   cp.cFloatflag = usr_floatflag = 0;
   cp.XL = xxmin = fractalspecific[PLASMA].xmin;
   cp.XR = xxmax = fractalspecific[PLASMA].xmax;
   cp.YT = yymax = fractalspecific[PLASMA].ymax;
   cp.YB = yymin = fractalspecific[PLASMA].ymin;
   cp.param[0] = param[0] = fractalspecific[PLASMA].paramvalue[0];
   cp.param[1] = param[1] = fractalspecific[PLASMA].paramvalue[1];
   cp.param[2] = param[2] = fractalspecific[PLASMA].paramvalue[2];
   cp.param[3] = param[3] = fractalspecific[PLASMA].paramvalue[3];;
   cp.trigndx[0] = trigndx[0] = SIN;
   cp.trigndx[1] = trigndx[1] = SQR;
   cp.trigndx[2] = trigndx[2] = SINH;
   cp.trigndx[3] = trigndx[3] = COSH;
   cp.stdCalcMode = usr_stdcalcmode = 'g';
   cp.distest = usr_distest = 0;
   cp.periodicitycheck = usr_periodicitycheck = 1;
   cp.biomorph = biomorph = -1;
   cp.decomp[0] = decomp[0] = 0; cp.decomp[1] = decomp[1] = 0;
   cp.inside = inside = 1;
   cp.outside = outside = -1;

    ytop    = 0;
    ybottom = ydots-1;
    xleft   = 0;
    xright  = xdots-1;

     /* resize the memory bitmap for the load */
     /* fall on sword if can't get memory */
     if (GetMemoryBitmap(habThread))
        return(SUB_STAT_FATAL);

     /* switch to the User (loaded) Palette */
     cp.pbmiMemory = &bmiColorTableUser;
     cp.sCurrentPalette = IDD_PAL_USER;

     bmiColorTableUser.cx = cxReal;
     bmiColorTableUser.cy = cyReal;
     bmiColorTableUser.cPlanes = planesReal;
     bmiColorTableUser.cBitCount = bitsReal;

     /* suck the color table out of the file */
     if (OS2fmt)
        {
        if (0 != DosRead(hfileIt,
                   (PVOID) &bmiColorTableUser.argbColor[0],
                    cbColorTable,
                    &usIOAction) )
           {
           DosClose(hfileIt);
           return SUB_STAT_ABORT;
           }
        }
     else
        {
        /* need to go from 4 bytes/entry to 3 bytes/entry */
        cbColors = cbColorTable / sizeof(RGBQUAD);
        prgb1 = &bmiColorTableUser.argbColor[0];
        for (i = 0; i < cbColors; i++)
            {
                     /* get R G B */
            if (0 != DosRead(hfileIt, &prgb1[i], sizeof(RGB), &usIOAction) )
               {
               DosClose(hfileIt);
               return SUB_STAT_ABORT;
               }
            /* skip over the next byte */
            DosChgFilePtr(hfileIt, 1L, FILE_CURRENT, &ulNewPtr);
            }
        /* recompute the amount of the palette table that we have
           now actually filled, for the replication routine next */
        cbColorTable = cbColors * sizeof(RGB);
        }

   /* now replicate the color table to 256 entries, if necessary */
   if (cp.colors < 256)
      {
      fp1 = (unsigned char _far *) &bmiColorTableUser.argbColor[0]; /* source */
      fp2 = fp1 + cbColorTable;        /* dest */
      sOffset = fp2 - fp1;             /* distance we have come so far */
      sReps = 256 / cp.colors;         /* if it ain't even, forget excess! */
      for (i = 1; i < sReps; i++)
          {
          _fmemcpy(fp2, fp1, sOffset); /* copy a thwak */
          fp2 += sOffset;              /*  then bump dest */
          }
      }

     cp.fHaveUserPalette = TRUE;

     /* now suck in the actual bits */
     /* read in (up to) 32K at a time */
     hp = cp.pixels;
     while(cbBitSize > 0)
        {
        if (0 != DosRead(hfileIt,
                    (PVOID) hp,
                    (cbBitSize > 0x8000L) ? 0x8000 : (USHORT) cbBitSize,
                    &usIOAction) )
           {
           DosClose(hfileIt);
           return SUB_STAT_ABORT;
           }
           hp += 0x8000L;
           cbBitSize -= 0x8000L;
        }

     DosClose(hfileIt);
     /* that's all, folks */
     return (SUB_STAT_OK);
     }

static int LoadDriverMET(VOID)
     {

     /* and go home */
     return (SUB_STAT_FATAL);
     }

static int LoadDriverPCX(VOID)
     {

     /* and go home */
     return (SUB_STAT_FATAL);
     }

/* --------------------------------------------------------------------- */

/* --------------------------------------------------------
   This routine fetches a bitmap from the PM Clipboard
   and puts it into our pixel array.  From there we will
   display it as a PLASMA.
   -------------------------------------------------------- */

VOID PMfrFetchClipbrdBmp(HAB hab)
   {

   HDC hdcClip;         /* memory DC and PS to extract from the clipboard */
   HPS hpsClip;
   HBITMAP hbmClip;
   BITMAPINFOHEADER bmiClip;
   SIZEL sizl;
   unsigned char _far *fp1;
   unsigned char _far *fp2;
   USHORT sOffset, sReps;
   int i;

   if (WinOpenClipbrd(hab))
      {
      if (hbmClip = (HBITMAP) WinQueryClipbrdData(hab, CF_BITMAP))
         {
         /* find the size of the bitmap */
         bmiClip.cbFix = sizeof(BITMAPINFOHEADER);

         GpiQueryBitmapParameters (hbmClip, &bmiClip);

         /* get the memory DC and PS to extract the bits */
         hdcClip = DevOpenDC (hab, OD_MEMORY, "*", 0L, NULL, NULL) ;

         sizl.cx = bmiClip.cx; sizl.cy = bmiClip.cy;

         hpsClip = GpiCreatePS (hab, hdcClip, &sizl,
                       PU_PELS | GPIF_DEFAULT | GPIT_MICRO | GPIA_ASSOC);

         /* set the bitmap in our PS */
         GpiSetBitmap(hpsClip, hbmClip);

         /* ensure cp and engine variables have reasonable values */

         cp.cx = xdots = filexdots = bmiClip.cx;
         cp.cy = ydots = fileydots = bmiClip.cy;
            /* note: standard bitmap assumed : cPlanes = 1 */
         cp.colors = colors = filecolors = 1 << bmiClip.cBitCount;
         cp.iFractType = fractype = PLASMA;
         cp.cFloatflag = usr_floatflag = 0;
         cp.XL = xxmin = fractalspecific[PLASMA].xmin;
         cp.XR = xxmax = fractalspecific[PLASMA].xmax;
         cp.YT = yymax = fractalspecific[PLASMA].ymax;
         cp.YB = yymin = fractalspecific[PLASMA].ymin;
         cp.param[0] = param[0] = fractalspecific[PLASMA].paramvalue[0];
         cp.param[1] = param[1] = fractalspecific[PLASMA].paramvalue[1];
         cp.param[2] = param[2] = fractalspecific[PLASMA].paramvalue[2];
         cp.param[3] = param[3] = fractalspecific[PLASMA].paramvalue[3];;
         cp.trigndx[0] = trigndx[0] = SIN;
         cp.trigndx[1] = trigndx[1] = SQR;
         cp.trigndx[2] = trigndx[2] = SINH;
         cp.trigndx[3] = trigndx[3] = COSH;
         cp.stdCalcMode = usr_stdcalcmode = 'g';
         cp.distest = usr_distest = 0;
         cp.periodicitycheck = usr_periodicitycheck = 1;
         cp.biomorph = biomorph = -1;
         cp.decomp[0] = decomp[0] = 0; cp.decomp[1] = decomp[1] = 0;
         cp.inside = inside = 1;
         cp.outside = outside = -1;

         ytop    = 0;
         ybottom = ydots-1;
         xleft   = 0;
         xright  = xdots-1;

         /* resize the memory bitmap for the load */
         /* fall on sword if can't get memory */
         if (GetMemoryBitmap(hab))
             goto stopit;

         /* switch to the User (loaded) Palette */
         cp.pbmiMemory = &bmiColorTableUser;
         cp.sCurrentPalette = IDD_PAL_USER;

         bmiColorTableUser.cx = bmiClip.cx;
         bmiColorTableUser.cy = bmiClip.cy;
         bmiColorTableUser.cPlanes = bmiClip.cPlanes;
         bmiColorTableUser.cBitCount = bmiClip.cBitCount;

         /* now grab the actual bits */
         GpiQueryBitmapBits(hpsClip, 0L, (LONG) bmiClip.cy,
                    cp.pixels, &bmiColorTableUser);

         /* now replicate the color table to 256 entries, if necessary */
         if (cp.colors < 256)
            {
            fp1 = (unsigned char _far *) &bmiColorTableUser.argbColor[0]; /* source */
            fp2 = fp1 + (cp.colors * sizeof(RGB) );        /* dest */
            sOffset = fp2 - fp1;             /* distance we have come so far */
            sReps = 256 / cp.colors;         /* if it ain't even, forget excess! */
            for (i = 1; i < sReps; i++)
                {
                _fmemcpy(fp2, fp1, sOffset); /* copy a thwak */
                fp2 += sOffset;              /*  then bump dest */
                }
            }

         cp.fHaveUserPalette = TRUE;
         cp.fNewBits = TRUE;

         /* now clean up */

 stopit:
         GpiSetBitmap(hpsClip, (HBITMAP) NULL);
         GpiDestroyPS(hpsClip);
         DevCloseDC(hdcClip);
         }

      WinCloseClipbrd(hab);
      }

   }
