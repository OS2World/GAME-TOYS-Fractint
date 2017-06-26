/*--------------------------------------------------
   PMFRSAVE.C -- FRACTINT for PM

      Save Processor

      This module contains both the code to write
      GIF, BMP, MET, and PCX files, it also contains
      the code used to put objects on the clipboard.

   04/16/91      Code by Donald P. Egen (with help)

   This module's first function (SaveDriver) is the
   main save action function.

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

/* routines in this module */

static int SaveDriverGIF(VOID);
static int SaveDriverBMP(VOID);
static int SaveDriverMET(VOID);
static int SaveDriverWin3BMP(VOID);
static int SaveDriverPCX(VOID);

extern HAB    habThread;

/* engine interface routines and variables */

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

extern int save_system, save_release, win_release;

extern int reallyega;
extern  void updatesavename(char *name);

int savetodisk(char *);

int  FAR SaveDriver (VOID)
     {

     int rc;

     /* dispatch based on sub-function - What format? */

     switch (cp.sSubFunction)
            {
            case SUB_LOADSAVE_GIF:
                 rc = SaveDriverGIF();
                 break;

            case SUB_LOADSAVE_BMP:
                 rc = SaveDriverBMP();
                 break;

            case SUB_LOADSAVE_MET:
                 rc = SaveDriverMET();
                 break;

            case SUB_LOADSAVE_WIN3BMP:
                 rc = SaveDriverWin3BMP();
                 break;

            case SUB_LOADSAVE_PCX:
                 rc = SaveDriverPCX();
                 break;

            default:
                 rc = 999;   /* let the caller figure it out */
                 break;

            }

     /* and go back */

     return rc;

     }

/*--------------------------------------------------

 Save a GIF file using the FRACTINT for DOS code.

 ---------------------------------------------------*/
static int SaveDriverGIF (VOID)
     {

     char achSaveName[128];

     unsigned char _far *pcSrc;
     unsigned char _far *pcDest;
     int i;

   /* ensure the engine save routine is cognizant of the
      description of the current image */

          xdots = filexdots = cp.cx;
          ydots = fileydots = cp.cy;
          usr_floatflag = cp.cFloatflag;
          fractype = cp.iFractType;
          xxmin = cp.XL;
          xxmax = cp.XR;
          yymin = cp.YB;
          yymax = cp.YT;
          xdots = cp.cx;
          ydots = cp.cy;
          param[0] = cp.param[0];
          param[1] = cp.param[1];
          param[2] = cp.param[2];
          param[3] = cp.param[3];
          trigndx[0] = cp.trigndx[0];
          trigndx[1] = cp.trigndx[1];
          trigndx[2] = cp.trigndx[2];
          trigndx[3] = cp.trigndx[3];
          set_trig_pointers(-1);
          colors = cp.colors;
          maxiter = cp.maxiter;
          usr_distest = cp.distest;
          usr_stdcalcmode = cp.stdCalcMode;
          usr_periodicitycheck = cp.periodicitycheck;
          biomorph = cp.biomorph;
          decomp[0] = cp.decomp[0]; decomp[1] = cp.decomp[1];
          inside = cp.inside;
          outside = cp.outside;

     /* map the colors from the current selected palette
        into the "dacbox" structure for saving.

        Unfortunately, PM uses "BGR", and dacbox uses "RGB"
     */

     pcSrc = (unsigned char _far *) &(cp.pbmiMemory->argbColor[0]);
     pcDest = (unsigned char _far *) &dacbox[0][0];

     for (i = 0; i < cp.colors; i++)
         {
         pcDest[0] = pcSrc[2] >> 2U;
         pcDest[1] = pcSrc[1] >> 2U;
         pcDest[2] = pcSrc[0] >> 2U;

         pcSrc += 3;
         pcDest += 3;
         }

     /* the following turns off the expected palette translation
        in the save code if it thinks it has a physical VGA
        running in 16 color mode.  The flag "reallyega" turns
        off this translation.
     */
     reallyega = (cp.colors == 16) ? 1 : 0;

     _fstrcpy(achSaveName, cp.szFileName);

     save_system = 1;
     save_release = win_release;
     savetodisk(achSaveName);

     /* save the updated name for next time */
     _fstrcpy(cp.szFileName, achSaveName);

     return (SUB_STAT_OK);
     }

/*--------------------------------------------------

 Save a BMP file in OS/2 1.1/1.2/1.3 format.

 ---------------------------------------------------*/
static int SaveDriverBMP(VOID)
     {

     BITMAPFILEHEADER bfhIt;
     HFILE hfileIt;
     USHORT usIOAction;
     LONG cbBitSize;
     unsigned char _huge *hp;
     char achSaveName[128];

     cbBitSize = (LONG) cp.pm_xbytes * (LONG) cp.cy;
     _fstrcpy(achSaveName, cp.szFileName);

     /* initialize the new bitmapfileheader */
     bfhIt.usType = BFT_BMAP;
     bfhIt.offBits = sizeof(BITMAPFILEHEADER) + cp.colors * sizeof(RGB);
     bfhIt.xHotspot = 0;
     bfhIt.yHotspot = 0;
     bfhIt.cbSize = bfhIt.offBits + cbBitSize;

     /* Now open the file */
     if ( 0 != DosOpen(cp.szFileName, &hfileIt, &usIOAction,
                           bfhIt.cbSize, FILE_NORMAL,
                           FILE_TRUNCATE | FILE_CREATE,
                           OPEN_ACCESS_WRITEONLY | OPEN_SHARE_DENYNONE, 0L) )
        {
        return SUB_STAT_ABORT;
        }

     /* write the file header */
     if (0 != DosWrite(hfileIt, &bfhIt,
                       sizeof(BITMAPFILEHEADER) - sizeof(BITMAPINFOHEADER),
                       &usIOAction) )
        {
        DosClose(hfileIt);
        return SUB_STAT_ABORT;
        }

     /* next, the bitmap infoheader and color table */
     if (0 != DosWrite(hfileIt, cp.pbmiMemory,
                       sizeof(BITMAPINFOHEADER) + cp.colors * sizeof(RGB),
                       &usIOAction) )
        {
        DosClose(hfileIt);
        return SUB_STAT_ABORT;
        }

     /* now blow out the actual bits */
     /* write out (up to) 32K at a time */
     hp = cp.pixels;
     while(cbBitSize > 0)
        {
        if (0 != DosWrite(hfileIt,
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

     /* save the updated name for next time */
     updatesavename(achSaveName);
     _fstrcpy(cp.szFileName, achSaveName);

     /* and go home */
     return (SUB_STAT_OK);
     }

static int SaveDriverMET(VOID)
     {

     /* and go home */
     return (SUB_STAT_FATAL);
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

/*--------------------------------------------------

 Save a BMP file in Windows 3.0 format (maybe also
 the OS/2 2.0 format?)

 ---------------------------------------------------*/
static int SaveDriverWin3BMP(VOID)
     {

     BITMAPFILEHEADER bfhIt;
     W3BITMAPINFOHEADER bmpIt3;
     HFILE hfileIt;
     USHORT usIOAction;
     LONG cbBitSize;
     RGB _far *prgb1;
     unsigned char _huge *hp;
     char achSaveName[128];
     int i;
     char Zero = '\0';

     cbBitSize = (LONG) cp.pm_xbytes * (LONG) cp.cy;
     _fstrcpy(achSaveName, cp.szFileName);

     /* initialize the new bitmapfileheader */
     bfhIt.usType = BFT_BMAP;
     bfhIt.offBits = sizeof(BITMAPFILEHEADER) - sizeof(BITMAPINFOHEADER)
                    + sizeof(W3BITMAPINFOHEADER)
                    + cp.colors * sizeof(RGBQUAD);
     bfhIt.xHotspot = 0;
     bfhIt.yHotspot = 0;
     bfhIt.cbSize = bfhIt.offBits + cbBitSize;

     /* initialize the Windows 3.0 bitmapinfoheader */
     bmpIt3.biSize = sizeof(W3BITMAPINFOHEADER);
     bmpIt3.biWidth = cp.cx;
     bmpIt3.biHeight = cp.cy;
     bmpIt3.biPlanes = cp.pbmiMemory->cPlanes;
     bmpIt3.biBitCount = cp.pbmiMemory->cBitCount;
     bmpIt3.biCompression = 0;  /* default the other trash */
     bmpIt3.biSizeImage = 0;
     bmpIt3.biXPelsPerMeter = 0;
     bmpIt3.biYPelsPerMeter = 0;
     bmpIt3.biClrUsed = 0;
     bmpIt3.biClrImportant = 0;

     /* Now open the file */
     if ( 0 != DosOpen(cp.szFileName, &hfileIt, &usIOAction,
                           bfhIt.cbSize, FILE_NORMAL,
                           FILE_TRUNCATE | FILE_CREATE,
                           OPEN_ACCESS_WRITEONLY | OPEN_SHARE_DENYNONE, 0L) )
        {
        return SUB_STAT_ABORT;
        }

     /* write the file header */
     if (0 != DosWrite(hfileIt, &bfhIt,
                       sizeof(BITMAPFILEHEADER) - sizeof(BITMAPINFOHEADER),
                       &usIOAction) )
        {
        DosClose(hfileIt);
        return SUB_STAT_ABORT;
        }

     /* next, the bitmap infoheader */
     if (0 != DosWrite(hfileIt, &bmpIt3,
                       sizeof(W3BITMAPINFOHEADER),
                       &usIOAction) )
        {
        DosClose(hfileIt);
        return SUB_STAT_ABORT;
        }

     /* now the color table.
        need to go from 3 bytes/entry to 4 bytes/entry */

     prgb1 = &cp.pbmiMemory->argbColor[0];

     for (i = 0; i < cp.colors; i++)
         {
                    /* write R G B */
         if (0 != DosWrite(hfileIt, &prgb1[i], sizeof(RGB), &usIOAction) )
            {
            DosClose(hfileIt);
            return SUB_STAT_ABORT;
            }
         /* then insert a zero byte */
         if (0 != DosWrite(hfileIt, &Zero, 1, &usIOAction) )
            {
            DosClose(hfileIt);
            return SUB_STAT_ABORT;
            }
         }

     /* now blow out the actual bits */
     /* write out (up to) 32K at a time */
     hp = cp.pixels;
     while(cbBitSize > 0)
        {
        if (0 != DosWrite(hfileIt,
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

     /* save the updated name for next time */
     updatesavename(achSaveName);
     _fstrcpy(cp.szFileName, achSaveName);

     /* and go home */
     return (SUB_STAT_OK);
     }

static int SaveDriverPCX(VOID)
     {

     /* and go home */
     return (SUB_STAT_FATAL);
     }

/* --------------------------------------------------------------------- */

/* --------------------------------------------------------
   This routine puts a bitmap on the PM Clipboard from
   our internal bitmap.
   -------------------------------------------------------- */

VOID PMfrWriteClipbrdBmp(HAB hab)
   {

   HDC hdcClip;         /* memory DC and PS to extract from the clipboard */
   HPS hpsClip;
   HBITMAP hbmClip;
   SIZEL sizl;
   BITMAPINFOHEADER bmp;
   ULONG _far *alRGBColors;
   LONG errorcode;
   char _far *fp1;
   char _far *fp2;
   int i;

   if (WinOpenClipbrd(hab))
      {
      /* get the memory DC and PS to copy the bitmap */
      hdcClip = DevOpenDC (hab, OD_MEMORY, "*", 0L, NULL, NULL) ;

      sizl.cx = cp.cx; sizl.cy = cp.cy;

      hpsClip = GpiCreatePS (hab, hdcClip, &sizl,
                   PU_PELS | GPIF_DEFAULT | GPIT_MICRO | GPIA_ASSOC);

      bmp.cbFix   = sizeof bmp;
      bmp.cx      = cp.cx;
      bmp.cy      = cp.cy;
      bmp.cPlanes = cp.cPlanes;
      bmp.cBitCount = cp.cBitCount;
      hbmClip = GpiCreateBitmap (hpsClip, &bmp, 0L, NULL, NULL);

      GpiSetBitmap(hpsClip, hbmClip);

      /* initialize and black out the bitmap */
      alRGBColors = (ULONG _far *) _fmalloc(sizeof(ULONG) * cp.colors);
      /* beginning of source array */
      fp2 = (char _far *) &cp.pbmiMemory->argbColor[0];
      /* beginning of dest array */
      fp1 = (char _far *) &alRGBColors[0];
      for (i = 0; i < cp.colors; i++)
          {   /* copy base bytes for number of screen colors */
          alRGBColors[i] = 0;
          _fmemcpy(fp1, fp2, sizeof(RGB) );
          fp1 += sizeof(ULONG);
          fp2 += sizeof(RGB);
          }

      GpiSetMix ( hpsClip, FM_OVERPAINT) ;
      GpiSetBackMix (hpsClip, BM_LEAVEALONE) ;
      GpiCreateLogColorTable(hpsClip, LCOL_RESET | LCOL_REALIZABLE,
              LCOLF_CONSECRGB, 0L, cp.colors, alRGBColors);

      /* now copy the bits */
      cp.pbmiMemory->cx = cp.cx;
      cp.pbmiMemory->cy = cp.cy;
      cp.pbmiMemory->cPlanes = cp.cPlanes;
      cp.pbmiMemory->cBitCount = cp.cBitCount;
      errorcode = GpiSetBitmapBits(hpsClip, 0L, (LONG) cp.cy,
                                cp.pixels, cp.pbmiMemory);

      /* unlink the new bitmap */
      GpiSetBitmap(hpsClip, (HBITMAP) NULL);

      /* write to the clipboard */
      WinEmptyClipbrd (hab);
      WinSetClipbrdData (hab, (ULONG) hbmClip, CF_BITMAP, CFI_HANDLE);

      /* now clean up */

      _ffree(alRGBColors);
      GpiDestroyPS(hpsClip);
      DevCloseDC(hdcClip);

      WinCloseClipbrd(hab);
      }

   }
