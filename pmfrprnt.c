/*--------------------------------------------------
   PMFRPRNT.C -- FRACTINT for PM

      Print Processor

   04/07/91      Code by Donald P. Egen (with help)

   This module's first function (PrintDriver) is the
   main print action function.
 ---------------------------------------------------*/

#define INCL_WIN
#define INCL_GPI
#define INCL_DOS
#define INCL_DEV
#include <os2.h>
#include <process.h>
#include <stdlib.h>
#include <stdio.h>
#include <setjmp.h>
#include <float.h>
#include <malloc.h>
#include <string.h>
#include <smplhelp.h>
#include <dos.h>

#include "pmfract.h"
#include "fractint.h"
#include "fractype.h"

extern HAB    habThread;

static int PrintDriverPM(VOID);
static int PrintDriverPJ(VOID);
static int getcolorx(int xdot, int ydot);

/*--------------------------------------------------

 This subroutine does the necessary mucking around to open
 the Control Panel/Print Manager default printer.
 This activity is not nearly as trivial as it ought to be.

 ---------------------------------------------------*/
HDC OpenDefaultPrinterDC( HAB hab,PSZ pszAppName, LONG type)
    {
        #define MAX_STRING_LEN  256

        static CHAR     szPrinter[MAX_STRING_LEN];
        static CHAR     szDetails[MAX_STRING_LEN];
        static DRIVDATA driv;

        DEVOPENSTRUC    dop;
        PCHAR           pchDelimiter;

    /* Get the default printer name, for example, "PRINTER1" */
        WinQueryProfileString( hab,
                "PM_SPOOLER",       /* section name       */
                "PRINTER",          /* keyname            */
                ";",                /* default            */
                szPrinter,          /* profile string     */
                MAX_STRING_LEN );   /* maximum characters */

        if ( NULL != (pchDelimiter = _fstrchr(szPrinter,';')) )
            *pchDelimiter = '\0';

        if (szPrinter[0] == '\0')
              return DEV_ERROR;

    /* Get the printer details.
     *  Fill in a supplied string with substrings:
     *  <physical port>;<driver name>;<queue port>;<network params>;
     */
        WinQueryProfileString( hab,
                "PM_SPOOLER_PRINTER",
                szPrinter,
                ";;;;",
                szDetails,
                MAX_STRING_LEN );

    /* Fill in the DEVOPENSTRUC and DRIVDATA structures */
        if ( NULL == (pchDelimiter = _fstrchr(szDetails,';')) )
            return DEV_ERROR;
        dop.pszDriverName = pchDelimiter + 1;
        if ( NULL == (pchDelimiter = _fstrchr(dop.pszDriverName,';')) )
            return DEV_ERROR;
        dop.pszLogAddress = pchDelimiter + 1;
        *(dop.pszLogAddress + _fstrcspn(dop.pszLogAddress,",;")) = '\0';
        *(dop.pszDriverName + _fstrcspn(dop.pszDriverName,",;")) = '\0';
        dop.pszComment = pszAppName;
        if ( NULL != (pchDelimiter = _fstrchr(dop.pszDriverName,'.')) )
            {
            driv.cb = sizeof(DRIVDATA) - 1;
            driv.lVersion = 0L;
            *pchDelimiter = '\0';
            _fstrncpy( driv.szDeviceName,pchDelimiter+1,
                     sizeof driv.szDeviceName );
            dop.pdriv = &driv;
            }
        else
            dop.pdriv = NULL;
        dop.pszDataType = "PM_Q_RAW";

    /* Now open the device context an return to caller */
        return DevOpenDC( hab,
                          type,
                          "*",
                          5L,         /* use first five fields. */
                          (PDEVOPENDATA) &dop,
                          (HDC) NULL );
    }


void GetPrinterData(HDC hdcPrint)
    {
    /* extract from an open printer DC the global data
       we want.  This is done at program init and also
       just before printing is to commence.
    */

    DevQueryCaps(hdcPrint, CAPS_VERTICAL_RESOLUTION, 1L, &cyPrtRes);
    DevQueryCaps(hdcPrint, CAPS_HORIZONTAL_RESOLUTION, 1L, &cxPrtRes);
    DevQueryCaps(hdcPrint, CAPS_WIDTH, 1L, &cxPrinter);
    DevQueryCaps(hdcPrint, CAPS_HEIGHT, 1L, &cyPrinter);
    DevQueryCaps(hdcPrint, CAPS_COLORS, 1L, &lPrinterColors);

    }

void FAR PrintDriverInit(void)
     {
     /* inquire the dimensions of the current default printer */
     HDC hdcPrint;

     /* first open an INFO device context to the default PM Printer */
     if ( NULL == (hdcPrint = OpenDefaultPrinterDC(habThread, szClientClass,
             OD_INFO) ) )
        return;

     /* now get what we came for */
     GetPrinterData(hdcPrint);

     /* and close it up */
     DevCloseDC (hdcPrint);

     }

int  FAR PrintDriver (VOID)
     {

     int rc;

     /* dispatch based on sub-function - do we do PM or PJ? */

     switch (cp.sSubFunction)
            {
            case SUB_PRINT_PM:
                 rc = PrintDriverPM();
                 break;

            case SUB_PRINT_PJ:
                 rc = PrintDriverPJ();
                 break;

            default:
                 rc = 999;   /* let the caller figure it out */
                 break;

            }

     /* and go back */

     return rc;

     }

static int PrintDriverPM(VOID)
     {
     /* Print to the Default Presentation Manger Printer

        The strategy for printing is to
        (1) construct DC and PS for printing.
        (2) set printer PS logical color table to the physical palette
            of the printer.  This should let us handle both
            monochrome and (hopefully) color printers with the
            same code.
        (3) build a memory DC, PS, and Bitmap of the same
            x and y dimensions as the in-core pixel map, but
            with the format requested by the printer.
        (4) load the memory Bitmap from the pixel array using
            an appropriate color table.
        (5) BitBlt the memory bitmap to the printer.
        (6) Clean up.
     */

     /* start with bunches of data */
     HDC hdcPrinter;
     HPS hpsPrinter;
     HDC hdcPrMem;
     HPS hpsPrMem;
     HBITMAP hbmPrMem;
     PBITMAPINFO pbmiPrMem;
     SIZEL sizl;
     ULONG _far *alPrPhysClrs;
     BITMAPINFOHEADER bmp;
     LONG   alBmpFormats[12];
     LONG errorcode;
     char FractName[32];
     POINTL aptl[4];

     /* obtain the name for use as the printer title */

     _fstrcpy(FractName, GetFractalName(cp.iFractType));

     /* open up a device context for the printer */
     if (NULL == (hdcPrinter = OpenDefaultPrinterDC(habThread, szTitleBar,
                 OD_QUEUED) ) )
        return(SUB_STAT_ABORT) ;

     /* ensure we are working with current information about
        the printer we are using */
     GetPrinterData(hdcPrinter);

     /* now get the PS */
     sizl.cx = 0; sizl.cy = 0;
     hpsPrinter = GpiCreatePS (habThread, hdcPrinter, &sizl,
                   PU_PELS | GPIF_DEFAULT | GPIT_MICRO | GPIA_ASSOC);
     /* and set the logical palette to the same as the physical palette */
     alPrPhysClrs = (ULONG _far *) _fmalloc(lPrinterColors * sizeof(ULONG) ) ;
     GpiQueryRealColors(hpsPrinter, LCOLOPT_REALIZED,
                      0L,   lPrinterColors, alPrPhysClrs);
     GpiCreateLogColorTable(hpsPrinter, LCOL_RESET | LCOL_REALIZABLE,
              LCOLF_CONSECRGB, 0L, lPrinterColors, alPrPhysClrs);

     /* now the memory DC, PS, and Bitmap */
     hdcPrMem = DevOpenDC (habThread, OD_MEMORY, "*", 0L, NULL, hdcPrinter) ;
     sizl.cx = (LONG) cp.cx; sizl.cy = (LONG) cp.cy;
     hpsPrMem = GpiCreatePS (habThread, hdcPrMem, &sizl,
                    PU_PELS | GPIF_DEFAULT | GPIT_MICRO | GPIA_ASSOC);

     GpiQueryDeviceBitmapFormats (hpsPrMem, 12L, alBmpFormats) ;
     bmp.cbFix   = sizeof bmp;
     bmp.cx      = cp.cx;
     bmp.cy      = cp.cy;
     bmp.cPlanes = (USHORT) alBmpFormats[0];
     bmp.cBitCount = (USHORT) alBmpFormats[1];
     hbmPrMem = GpiCreateBitmap (hpsPrMem, &bmp, 0L, NULL, NULL);
     errorcode = WinGetLastError(habThread);

     GpiSetBitmap (hpsPrMem, hbmPrMem);
     errorcode = WinGetLastError(habThread);
     GpiSetMix ( hpsPrMem, FM_OVERPAINT) ;
     GpiSetBackMix (hpsPrMem, BM_LEAVEALONE) ;
     GpiCreateLogColorTable(hpsPrMem, LCOL_RESET | LCOL_REALIZABLE,
              LCOLF_CONSECRGB, 0L, lPrinterColors, alPrPhysClrs);

     /* Now load the memory bit map from the master pixel array */
     pbmiPrMem = &bmiColorTableBW;
     pbmiPrMem->cx = cp.cx;
     pbmiPrMem->cy = cp.cy;
     pbmiPrMem->cPlanes = cp.cPlanes;
     pbmiPrMem->cBitCount = cp.cBitCount;
     errorcode = GpiSetBitmapBits(hpsPrMem, 0L, (LONG) cp.cy,
                     cp.pixels, pbmiPrMem);

     /* now its off to the printer */
     DevEscape (hdcPrinter, DEVESC_STARTDOC,
               _fstrlen(FractName) , FractName, NULL, NULL);
     aptl[0].x = 0;     aptl[0].y = 0;
     aptl[1].x = cxPrinter; aptl[1].y = cyPrinter;
     aptl[2].x = 0;     aptl[2].y = 0;
     aptl[3].x = cp.cx; aptl[3].y = cp.cy;

     GpiBitBlt (hpsPrinter, hpsPrMem, 4L, aptl, ROP_SRCCOPY, BBO_IGNORE);
     errorcode = WinGetLastError(habThread);

     /* now clean up */
     DevEscape (hdcPrinter, DEVESC_ENDDOC, 0L, NULL, NULL, NULL) ;

     GpiSetBitmap(hpsPrMem, (HBITMAP) NULL);
     GpiDeleteBitmap (hbmPrMem);
     GpiDestroyPS(hpsPrMem);
     DevCloseDC (hdcPrMem);

     GpiDestroyPS (hpsPrinter) ;
     DevCloseDC (hdcPrinter) ;

     _ffree (alPrPhysClrs);

     return SUB_STAT_OK;

     }


/*
     Print to an HP-PaintJet ( ... sure wish there was a real driver ...)

     Code initially stolen from FRACTINT For DOS.

     Rescaling code put in as an afterthought (those 1.5" by 1.8"
     pictures were cute but not what I originally had in mind).

*/

FILE *PRFILE;
extern int extraseg;

static unsigned char _huge *pixels;
static int xleftx, xrightx, ybottomy, ytopy;
static SHORT ycy, xpm_xdots;
static USHORT xpixels_per_byte;  /* bitmap configuration data */
static LONG   xpixels_per_bytem1;
static USHORT xpixelshift_per_byte;
static unsigned char xpm_andmask[2];
static unsigned char xpm_notmask[2];
static unsigned char xpm_bitshift[2];

void updatesavename(char *name);

static int PrintDriverPJ(VOID)
{
    register int y,j;           /* indices                        */
    char buff[192];             /* buffer for 192 sets of pixels  */
                                /* This is very large so that we can*/
                                /* get reasonable times printing  */
                                /* from modes like 2048x2048 disk-*/
                                /* video.  When this was 24, a 2048*/
                                /* by 2048 pic took over 2 hours to*/
                                /* print.  It takes about 15 min now*/
    int BuffSiz;                /* how much of buff[] we'll use   */
    char far *es;               /* pointer to extraseg for buffer */
    int i,x,                    /* more indices                   */
        res,                    /* resolution we're gonna' use    */
                                /************************************/
        ptrid;                  /* Printer Id code.               */
                                /* Currently, the following are   */
                                /* assigned:                      */
                                /*            1. HPLJ (all)       */
                                /*               Toshiba PageLaser*/
                                /*            2. IBM Graphics     */
                                /*            3. Color Printer    */
                                /*            4. HP PaintJet      */
                                /*            5. PostScript       */
                                /************************************/
    char prfname[128];
    char fmtbuf[128];

     /* we want to stretch the bitmap to full page Paintjet size */
     HDC hdcPjMem;
     HPS hpsPjMem;
     HBITMAP hbmPjMem;
     PBITMAPINFO pbmiPjMem;
     SIZEL sizl;
     BITMAPINFOHEADER bmp;
     LONG errorcode;
     POINTL aptl[4];
     LONG pm_bytes;
     static ULONG _far alPjPhysClrs[16] = {  0x00000000, 0x00000080,
                                             0x00008000, 0x00008080,
                                             0x00800000, 0x00800080,
                                             0x00808000, 0x00808080,
                                             0x00CCCCCC, 0x000000FF,
                                             0x0000FF00, 0x0000FFFF,
                                             0x00FF0000, 0x00FF00FF,
                                             0x00FFFF00, 0x00FFFFFF };

    for (j=0;j<192;j++) { buff[j]=0; }          /* Clear buffer  */

    _fstrcpy(prfname, cp.szPrintName);

    if ((PRFILE = fopen(prfname,"wb"))==NULL)
          {
          PMfrSyncMsg(4, szPrintNoOpen);
          return (SUB_STAT_ABORT);
          }

    if (extraseg) {
        #ifdef __TURBOC__
            es=MK_FP(extraseg,0);
        #else
            FP_SEG(es)=extraseg;
            FP_OFF(es)=0;
        #endif
        }

    /* stretch the bitmap into a larger bitmap that is full page on the Paintjet */
     hdcPjMem = DevOpenDC (habThread, OD_MEMORY, "*", 0L, NULL, cp.hdcScreen) ;
     sizl.cx = 1800L; sizl.cy = 1440L;
     ycy = 1440;
     xpm_xdots = (((ULONG) sizl.cx + 7) & 0xfffffff8);
     xleftx = 0; xrightx = 1799; ybottomy = 1439; ytopy = 0;
     xpixelshift_per_byte = 1;
     xpixels_per_byte = 2;
     xpixels_per_bytem1 = 1;
     xpm_andmask[0] = 0xf0;
     xpm_notmask[0] = 0x0f;
     xpm_bitshift[0] = 4;
     xpm_andmask[1] = 0x0f;
     xpm_notmask[1] = 0xf0;
     xpm_bitshift[1] = 0;
     hpsPjMem = GpiCreatePS (habThread, hdcPjMem, &sizl,
                    PU_PELS | GPIF_DEFAULT | GPIT_MICRO | GPIA_ASSOC);

     bmp.cbFix   = sizeof bmp;
     bmp.cx      = sizl.cx;
     bmp.cy      = sizl.cy;
     bmp.cPlanes = 1;   /* paintjet is now at most 16 color as we use it */
     bmp.cBitCount = 4;
     hbmPjMem = GpiCreateBitmap (hpsPjMem, &bmp, 0L, NULL, NULL);
     errorcode = WinGetLastError(habThread);

     GpiSetBitmap (hpsPjMem, hbmPjMem);
     errorcode = WinGetLastError(habThread);
     GpiSetMix ( hpsPjMem, FM_OVERPAINT) ;
     GpiSetBackMix (hpsPjMem, BM_LEAVEALONE) ;
     GpiCreateLogColorTable(hpsPjMem, LCOL_RESET,
              LCOLF_CONSECRGB, 0L, 16L, alPjPhysClrs);

     aptl[0].x = 0;     aptl[0].y = 0;
     aptl[1].x = 1800L; aptl[1].y = 1440;
     aptl[2].x = 0;     aptl[2].y = 0;
     aptl[3].x = cp.cx; aptl[3].y = cp.cy;

     GpiBitBlt (hpsPjMem, cp.hpsMemory, 4L, aptl, ROP_SRCCOPY, BBO_IGNORE);

     pm_bytes =  (LONG) (xpm_xdots >> xpixelshift_per_byte) * (LONG) sizl.cy;

     pm_bytes = ( (pm_bytes + 7) & 0xfffffff8) >> 3;
     pixels = (unsigned char _huge *) halloc(pm_bytes, 8);

     pbmiPjMem = (PBITMAPINFO) _fmalloc( sizeof(BITMAPINFOHEADER) +
                                         16 * sizeof(RGB) );

     pbmiPjMem->cbFix = sizeof(BITMAPINFOHEADER);
     pbmiPjMem->cPlanes = 1;
     pbmiPjMem->cBitCount = 4;
     pbmiPjMem->cx = sizl.cx;
     pbmiPjMem->cy = sizl.cy;

     GpiQueryBitmapBits(hpsPjMem, 0L, sizl.cy-1, pixels, pbmiPjMem);

     /* end of rescale */

    ptrid=4;
    res=180;
        /****** PaintJet  *****/
    if ( res < 150 ) res = 90;      /* an arbitrary boundary */
    if ( res >= 150 ) res = 180;
    /*****  Set up buffer size for immediate user gratification *****/
    /*****    AKA, if we don't have to, don't buffer the data   *****/
    BuffSiz=8;
    if (sizl.cx>1024) BuffSiz=192;     /*****   Initialize printer  *****/

                                /******  INITIALIZE GRAPHICS MODES  ******/
    /****** PaintJet *****/
    /* set resolution, 4 colour planes for 90dpi or 3 for 180dpi,
              start raster graphics   */
    sprintf(fmtbuf,"\x1B" "E" "\x1B*t%dR\x1B*r%dU\x1B*r0A",
                       res,res==90?4:3);
    fputs(fmtbuf, PRFILE);

    for (j=0;j<192;j++)
         { buff[j]=0; } /* Clear buffer            */

                                /*****  Get And Print Screen **** */
                               /* HP PaintJet       */
         /* DPE mod - Scan it for Landscape printing */
         /* Note: must scan Y from bottom (max) up for the
            printed picture to come out top up.             */
         for (x=0;x<sizl.cx;x++)
         {

             sprintf(fmtbuf,"\x1B*b%dV",ycy/8);
             fputs(fmtbuf, PRFILE);
             for (y=ycy-8;y>=0;y-=8)
             {
                 buff[0]=0;
                 for (i=7;i>=0;i--)
                 {
                      buff[0]<<=1;
                      if ( getcolorx(x,y+i)&4 )  /* r */
                               buff[0]++;
                 }
                 fputc(buff[0],PRFILE);
             }

             sprintf(fmtbuf,"\x1B*b%dV",ycy/8);
             fputs(fmtbuf, PRFILE);
             for (y=ycy-8;y>=0;y-=8)
             {
                 buff[0]=0;
                 for (i=7;i>=0;i--)
                 {
                     buff[0]<<=1;
                     if ( getcolorx(x,y+i)&2 )  /* g */
                            buff[0]++;
                 }
                 fputc(buff[0],PRFILE);
             }

             sprintf(fmtbuf,"\x1B*b%dV",ycy/8);
             fputs(fmtbuf, PRFILE);
             for (y=ycy-8;y>=0;y-=8)
             {
                 buff[0]=0;
                 for (i=7;i>=0;i--)
                 {
                     buff[0]<<=1;
                     if ( getcolorx(x,y+i)&1 )  /* b */
                            buff[0]++;
                 }
                 fputc(buff[0],PRFILE);
             }

             if (res == 90)
             {
                sprintf(fmtbuf,"\x1B*b%dV",ycy/8);
                fputs(fmtbuf, PRFILE);
                for (y=ycy-8;y>=0;y-=8)
                {
                    buff[0]=0;
                    for (i=7;i>=0;i--)
                    {
                        buff[0]<<=1;
                        if ( getcolorx(x,y+i)&8 )  /* i */
                                buff[0]++;
                    }
                    fputc(buff[0],PRFILE);
                }
             }

             sprintf(fmtbuf,"\x1B*b0W"); /* end colour planes   */
             fputs(fmtbuf, PRFILE);
         }

         sprintf(fmtbuf,"\x1B*r0B");  /* end raster graphics */
         fputs(fmtbuf, PRFILE);
         fputs("\x1B" "E",PRFILE);    /* reset printer */

    fclose(PRFILE);

    /* increment the save name for next time */
    updatesavename(prfname);
    _fstrcpy(cp.szPrintName, prfname);

    _ffree(pbmiPjMem);
    hfree(pixels);
    GpiSetBitmap(hpsPjMem, (HBITMAP) NULL);
    GpiDeleteBitmap (hbmPjMem);
    GpiDestroyPS(hpsPjMem);
    DevCloseDC (hdcPjMem);

    return (SUB_STAT_OK);

}

static int getcolorx (int xdot, int ydot)
{
     ULONG pm_y, offset;

    if ( ! ((xdot >= xleftx) && (xdot <= xrightx) &&
            (ydot >= ytopy)  && (ydot <= ybottomy)) )
       return 0;        /* arbitrary value */

     pm_y = (ULONG) (ycy - ydot - 1);
     offset = ((ULONG) xpm_xdots * pm_y) + (ULONG) xdot;

     if (xpixelshift_per_byte == 0)
       {
          return(pixels[offset]);
       }
     else
       {
        USHORT j;
        j = offset & xpixels_per_bytem1;
        offset = offset >> xpixelshift_per_byte;
        return((int)((pixels[offset] & xpm_andmask[j]) >> xpm_bitshift[j]));
       }
}
