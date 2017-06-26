/*--------------------------------------------------
   PMFRCALC.C -- FRACTINT for PM

      Fractal Calculation Engine Interface Module

   03/15/91	 Code by Donald P. Egen (with help)

   This module's first function (CalcDriverInit)
   does calculation-engine specific initialization,
   primarily calling the FRACTINT command line and init
   file processing routines, then copying the results
   back where the user interface can find them.

   This module's second function (CalcDriver) is the
   Fractal Calculation Engine interface function.
   Much of its code was extracted from module FRACTINT.C,
   the base module and user interface module of the FRACTINT
   DOS-based fractal calculation program.
   As such, it looks very much like a hack-up job.
   It gets and then updates the background bitmap
   (and memory Device Context and Presentation Space).

   Functions putcolor and getcolor serve as the glue
   modules back from the fractal calculation engine
   to the PM display environment.  putcolor sets
   pixels, getcolor reads pixels.  Each puts or gets
   the background bitmap.

   Other functions tie up loose ends in the fractal
   calculation engine.  They return control to the
   main CalcThread routine and terminate the current
   calculation with an error post to the main thread,
   which will result in an error box.
 ---------------------------------------------------*/

#define PUTTHEMHERE 1		/* stuff common external arrays here */

/* from MSC's <dos.h> */
#define FP_SEG(fp) (*((unsigned _far *)&(fp)+1))
#define FP_OFF(fp) (*((unsigned _far *)&(fp)))

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
#include <search.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#include <smplhelp.h>

#include "pmfract.h"
#include "fractint.h"
#include "fractype.h"

jmp_buf  longjmpbuf;       /* unwind from stubbed-out subroutine */
extern HAB    habThread;

extern	int cputype(void);     /* in PMGENERL.ASM */
extern  int fputype(void);
extern	int fpu, cpu;

int save_system;	   /* tag identifying Fractint for Windows */
int save_release;          /* tag identifying version number */
int win_release = 200;	   /* tag identifying version number */

int cmp_line(), pot_line();

char FileName[128];
char string004[2];

int xdots, ydots, colors, maxiter;
int ytop, ybottom, xleft, xright;

int fractype;
int invert;

double ftemp, xxmin, xxmax, yymin, yymax;
long fudge;

long lm, linitx, linity;
int maxit, bitshift, color, row, col;

int calc_status; /* -1 no fractal		    */
		 /*  0 parms changed, recalc reqd   */
		 /*  1 actively calculating	    */
		 /*  2 interrupted, resumable	    */
		 /*  3 interrupted, not resumable   */
		 /*  4 completed		    */

int reset_periodicity = 1;
extern int kbdcount;
int show_orbit = 0;
int orbit_ptr = 0;
int periodicitycheck;
extern int debugflag;
int integerfractal;
extern int distest;
extern int bitshiftless1;
char usr_stdcalcmode, stdcalcmode;
int  usr_distest, usr_floatflag, usr_periodicitycheck;
long calctime;

long creal, cimag;
long delx, dely, delx2, dely2, delmin;
long xmin, xmax, ymin, ymax, x3rd, y3rd;
double  dxsize, dysize;		/* xdots-1, ydots-1	    */
double delxx, delyy, delxx2, delyy2, ddelmin, xx3rd, yy3rd;
double param[4];
double potparam[3];
int diskvideo;

/*    #define MAXLINE  2048  */
#define MAXLINE 4096

long   far *lx0, far *ly0, far *lx1, far *ly1;
double far *dx0, far *dy0, far *dx1, far *dy1;
   /*  extern double far temp_array[8001];  */
double far *temp_array;
double far *temp_array2;

extern unsigned char trigndx[4];
extern void set_trig_pointers(int which);

unsigned char dacbox[257][3];
double plotmx1, plotmx2, plotmy1, plotmy2;
int MPOverflow;
int dotmode;
extern int pot16bit;
int andcolor, resave_flag, timer_interval;
int active_ovly;

    /*	int extraseg;  */

extern int extraseg;

int timedsave = 0;
int disk16bit = 0;
extern int initbatch;
float far initifs3d[32][13];
float far initifs[32][7];
extern unsigned char readname[];
unsigned char MAP_name[40] = "";
int rowcount;
int adapter;
extern int showfile;
extern int initmode;
extern int overlay3d;
extern int display3d;
extern int filetype;
int comparegif = 0;
int diskisactive = 0;
extern int initsavetime;
int saveticks = 0;
int savebase = 0;
int zwidth = 0;
extern int (*outln)();
extern int out_line();
extern int outlin16();
static int call_line3d();
extern int line3d();
extern int gifview();
extern int tgaview();
int mapset = 0;
int reallyega = 0;
int loadPalette = 0;
int compiled_by_turboc = 0;

/* Fractint Release 15.1 engine */

int mode7text, Slope, textsafe, Distribution, Offset;
long con;
int Print_To_File, EPSFileType;

/* **** **** **** **** **** **** */

extern char ifsfilename[];
extern char ifs3dfilename[];
extern char FormFileName[];
extern char FormName[];
extern char LFileName[];
extern char LName[];
extern char PrintName[];

int     sxdots,sydots;
int     sxoffs=0,syoffs=0;
int     viewwindow=0;
float   viewreduction=1;
int     viewcrop=1;
float   finalaspectratio;
int     viewxdots=0,viewydots=0;
extern int filexdots, fileydots, filecolors;
int frommandel;
extern int decomp[2];
extern int biomorph, inside, outside;

int debug_fastupdate;    	/* debugging - update every pixel if set */

int Printer_Resolution, Printer_Titleblock, Printer_SFrequency;
int Printer_SetScreen, Printer_SStyle, Printer_Type;
int Printer_ColorXlat, Printer_SAngle, LPTNumber;
int video_type;

int active_system = WINFRAC;	/* running under windows/PM */
extern int win_display3d, win_overlay3d;

#pragma optimize("elg",off)

 /* Local initialization */

void FAR CalcDriverInit(void)
    {

	  BYTE	b80287;

int i;
double temp1, temp2;
double dtemp, ccreal, ccimag;

SEL selExtraSeg, selExtraSeg2;

     /* init one time only vars */
     cpu = cputype();

     /* must ask OS/2 if we have a coprocessor before
        asking the coprocessor what kind he is! */
#define DEVINFO_COPROCESSOR 0x0003
     DosDevConfig (&b80287, DEVINFO_COPROCESSOR, 0);
     if (b80287)
         fpu = fputype();
     else
	 fpu = 0;

  /* go for 2*64K memory from OS/2 */
  DosAllocSeg(0, &selExtraSeg, SEG_NONSHARED);
  DosAllocSeg(0, &selExtraSeg2, SEG_NONSHARED);

  extraseg = (int) selExtraSeg;
  temp_array = MAKEP(selExtraSeg, 0);
  temp_array2 = MAKEP(selExtraSeg2, 0);

lx0 = (long far *)&temp_array[0*MAXLINE];
ly0 = (long far *)&temp_array[1*MAXLINE];
lx1 = (long far *)&temp_array2[0*MAXLINE];
ly1 = (long far *)&temp_array2[1*MAXLINE];
dx0 = (double far *)&temp_array[0*MAXLINE];
dy0 = (double far *)&temp_array[1*MAXLINE];
dx1 = (double far *)&temp_array2[0*MAXLINE];
dy1 = (double far *)&temp_array2[1*MAXLINE];
   /*	 extraseg = FP_SEG(dx0);    */

restoredac();		/* ensure that the palette has been initialized */

cmdfiles(MainArgc, MainArgv);		/* SSTOOLS.INI processing */

initmode = 1;                           /* override SSTOOLS.INI */

dotmode = 1;
diskvideo = 0;
usr_distest = 0;

/* ----- */

debug_fastupdate = 0;
calc_status = -1;
resave_flag = 1;
strcpy(FileName,"Fract001");
if (showfile != 0) {
    strcpy(readname, FileName);
    /* we don't have a file name passed to us to load.
       Therefore we begin calculating. */
    DosSemClear((HSEM) &cp.ulSemTrigger);
    cp.sSubAction = SUB_ACT_CALC;
    cp.sSubFunction = 0;
    }
else {
    if (strchr(readname,'.') == NULL)
	 strcat(readname,".gif");
    strcpy(FileName,readname);
    // time_to_load = 1;
    DosSemClear((HSEM) &cp.ulSemTrigger);
    cp.sSubAction = SUB_ACT_LOAD;
    cp.sSubFunction = SUB_LOADSAVE_GIF;
    }

/*
   sprintf(strlocn," %d bytes",stackavail());
   stopmsg(0,"Available Stack",strlocn);
*/
    save_system = 1;
    save_release = win_release;

   /* default to a window-size picture */
   cp.cx = cxClient;
   cp.cy = cyClient;

   /* initialize based on the physical colors the screen can support */
   if (lScreenColors <16)
      {
      cp.colors = 2;
      cp.cBitCount = 1;
      cp.maxiter = 150;
      }
   else if (lScreenColors < 256)
      {
      cp.colors = 16;
      cp.cBitCount = 4;
      cp.maxiter = 150;
      }
   else
      {
      cp.colors = 256;
      cp.cBitCount = 8;
      cp.maxiter = 250;
      }

   /* collect the calc data set by engine init - CMDFILES mostly */
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
   _fstrcpy(cp.szFileName, FileName);
   _fstrcpy(cp.szIfsFileName, ifsfilename);
   _fstrcpy(cp.szIfs3dFileName, ifs3dfilename);
   _fstrcpy(cp.szFormFileName, FormFileName);
   _fstrcpy(cp.szFormName, FormName);
   _fstrcpy(cp.szLSysFileName, LFileName);
   _fstrcpy(cp.szLSysName, LName);
   _fstrcpy(cp.szPrintName, PrintName);
   cp.stdCalcMode = usr_stdcalcmode;
   cp.distest = usr_distest;
   cp.periodicitycheck = usr_periodicitycheck;
   cp.biomorph = biomorph;
   cp.decomp[0] = decomp[0]; cp.decomp[1] = decomp[1];
   cp.inside = inside;
   cp.outside = outside;

     }

 /* Now, the real driver */
int  FAR CalcDriver (VOID)
     {
int i;
double temp1, temp2;
double dtemp, ccreal, ccimag;

     int jmp_rc;
     double ftemp;

	  /* resize the memory bitmap for the current calc */
	  /* fall on sword if can't get memory */
	  if (GetMemoryBitmap(habThread))
	     return(SUB_STAT_FATAL);

          WinPostMsg (cp.hwnd, WM_THRD_POST,
                  MPFROM2SHORT(SUB_STAT_START_TIMER,0), MPFROMP(NULL) ) ;

          /* we are directed by the main task to calculate a fractal */
          /* prepare the variable mass for the calculation engine */

	  usr_floatflag = cp.cFloatflag;
	  fractype = cp.iFractType;			/* specify the fractal */
	  _fstrcpy(FileName,cp.szFileName);
	  _fstrcpy(ifsfilename, cp.szIfsFileName);
	  _fstrcpy(ifs3dfilename, cp.szIfs3dFileName);
	  _fstrcpy(FormFileName, cp.szFormFileName);
	  _fstrcpy(FormName, cp.szFormName);
	  _fstrcpy(LFileName, cp.szLSysFileName);
	  _fstrcpy(LName, cp.szLSysName);
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

	  /* a little (in)sanity check before we crank up */

	  if ((xdots < 2) || (ydots < 2) )
	      return(33);

	  /* If we are about to calculate an IFS or IFS3D fractal,
	     we need to load the data from the file.
	     This is a proper job for the subthread.
	  */

	  if (fractype == IFS)
	       ifsgetfile();

	  if (fractype == IFS3D)
	       ifs3dgetfile();

reinit:

    for (i = 0; i < 4; i++) {
        if(param[i] != FLT_MAX)
            fractalspecific[fractype].paramvalue[i] = param[i];
        else
            param[i] = fractalspecific[fractype].paramvalue[i];
        }
    ccreal = param[0]; ccimag = param[1]; /* default C-values */
    frommandel = 0;

    if (xxmin > xxmax) {
       dtemp = xxmin; xxmin = xxmax; xxmax = dtemp;} 
    if (yymin > yymax) {
       dtemp = yymin; yymin = yymax; yymax = dtemp;} 

    ytop    = 0;
    ybottom = ydots-1;
    xleft   = 0;
    xright  = xdots-1;
    filexdots = xdots;
    fileydots = ydots;
    filecolors = colors;

restart:

    if (calc_status == -99)
        calc_status = 2;			/* force a recalc */
    else
        calc_status = 0;			/* force a restart */

    maxit = maxiter;

    if (colors == 2 && (fractype == PLASMA || usr_stdcalcmode == 'b'))
        colors = 16;         /* 2-color mode just doesn't work on these */

    andcolor = colors-1;

    debug_fastupdate = 0;    /* turn off debug-mode screen updates */
    if (debugflag == 6666)
        debug_fastupdate = 1;

    /* compute the (new) screen co-ordinates */
    /* correct a possibly munged-up zoom-box outside the image range */
    if (ytop    >= ydots) ytop    = ydots-1;
    if (ybottom >= ydots) ybottom = ydots-1;
    if (xleft   >= xdots) xleft   = xdots-1;
    if (xright  >= xdots) xright  = xdots-1;
    if (xleft == xright || ytop == ybottom) {
        }
    if (xleft > xright)
        { i = xleft; xleft = xright; xright = i;}
    if (ytop > ybottom)
        { i = ybottom; ybottom = ytop; ytop = i;}
    temp1 = xxmin;
    temp2 = xxmax - xxmin;
    xxmin = temp1 + (temp2 * xleft )/(xdots-1);
    xxmax = temp1 + (temp2 * xright)/(xdots-1);
    temp1 = yymin;
    temp2 = yymax - yymin;
    yymin = temp1 + (temp2 * (ydots - 1 - ybottom)/(ydots-1));
    yymax = temp1 + (temp2 * (ydots - 1 - ytop   )/(ydots-1));
    xx3rd = xxmin;
    yy3rd = yymin;
    xleft   = 0;
    xright  = xdots-1;
    ytop    = 0;
    ybottom = ydots-1;

/*
    delxx = (xxmax - xxmin) / (xdots-1);
    delyy = (yymax - yymin) / (ydots-1);
    delxx2 = delyy2 = 0.0;
    ddelmin = fabs(delxx);
    if (fabs(delyy) < ddelmin)
        ddelmin = fabs(delyy);
*/

    dxsize = xdots - 1;  dysize = ydots - 1;

    if (calc_status != 2 && !overlay3d)
        if (!clear_screen(1)) {
            stopmsg(0,"Screen-Clearing Function","Can't free and re-allocate the image");
            return(0);
            }

    dxsize = xdots - 1;  dysize = ydots - 1;
    sxdots = xdots;  sydots = ydots;
    finalaspectratio = ((float)ydots)/xdots;

    calcfracinit();

    bitshiftless1 = bitshift - 1;

    /* delete the following file read processing */
#if 0
      if(showfile == 0) {		/* loading an image */
	 if (display3d) 		/* set up 3D decoding */
	    outln = call_line3d;
	 else if(filetype >= 1) 	/* old .tga format input file */
	    outln = outlin16;
	 else if(comparegif)		/* debug 50 */
	    outln = cmp_line;
	 else if(pot16bit) {		/* .pot format input file */
	    pot_startdisk();
	    outln = pot_line;
	    }
	 else				/* regular gif/fra input file */
	    outln = out_line;
	 if(filetype == 0)
	    i = funny_glasses_call(gifview);
	 else
	    i = funny_glasses_call(tgaview);
	 if(i == 0)
	    buzzer(0);
	 else {
	    calc_status = -1;
	    }
	 }
#endif

      if(showfile == 0) {		/* image has been loaded */
	 showfile = 1;
	 if (initbatch == 1 && calc_status == 2)
	    initbatch = -1; /* flag to finish calc before save */
	 if (calc_status == 2) goto try_to_resume;
	 }
      else {				/* draw an image */

try_to_resume:

	 diskisactive = 1;		/* flag for disk-video routines */
	 if (initsavetime != 0		/* autosave and resumable? */
	   && (fractalspecific[fractype].flags&NORESUME) == 0) {
	    savebase = readticker(); /* calc's start time */
	    saveticks = (long)initsavetime * 1092; /* bios ticks/minute */
	    if ((saveticks & 65535) == 0)
	       ++saveticks; /* make low word nonzero */
	    }
	 kbdcount = 30; 		/* ensure that we check the keyboard */

	 }

	  /* set up to catch a longjmp from a subroutine loose end */
          jmp_rc = setjmp(longjmpbuf);

          if (0 == jmp_rc)
             {  /* ready for a calculation */
             jmp_rc = calcfract();
             /* jmp_rc == 0 for done, 1 for interrupted */
	     saveticks = 0;	 /* turn off autosave timer */
	     diskisactive = 0;	 /* flag for disk-video routines */
	     overlay3d = 0;	 /* turn off overlay3d */
	     display3d = 0;	 /* turn off display3d */

	     zwidth = 0;

	     }
          /* else jmp_rc is a locator code from a loose end */

	  /* translate Aborted from Fractint-eze to PMfract-eze */
	  if (jmp_rc==1) jmp_rc = SUB_STAT_ABORT;
          /* in any case, pass it back to the main task */

          /* return our final status back to the subtask driver */
          return (jmp_rc);
     }

/* displays differences between current image file and new image */
/* Bert - suggest add this to video.asm */
int cmp_line(unsigned char *pixels, int linelen)
{
   static errcount;
   static FILE *fp = NULL;
   extern int rowcount;
   int row,col;
   int oldcolor;
   char *timestring;
   time_t ltime;
   if(fp == NULL)
      fp = fopen("cmperr",(initbatch)?"a":"w");
   if((row = rowcount++) == 0)
      errcount = 0;
   if(pot16bit) { /* 16 bit info, ignore odd numbered rows */
      if((row & 1) != 0) return(0);
      row >>= 1;
      }
   for(col=0;col<linelen;col++) {
      oldcolor=getcolor(col,row);
      if(oldcolor==pixels[col])
	 putcolor(col,row,0);
      else {
	 if(oldcolor==0)
	    putcolor(col,row,1);
	 ++errcount;
	 if(initbatch == 0)
	    fprintf(fp,"#%5d col %3d row %3d old %3d new %3d\n",
	       errcount,col,row,oldcolor,pixels[col]);
	 }
      }
   if(row+1 == ydots && initbatch) {
      time(&ltime);
      timestring = ctime(&ltime);
      timestring[24] = 0; /*clobber newline in time string */
      fprintf(fp,"%s compare to %s has %5d errs\n",timestring,readname,errcount);
      }
   return(0);
}

int pot_line(unsigned char *pixels, int linelen)
{
   extern int rowcount;
   int row,col,saverowcount;
   if (rowcount == 0)
      pot_startdisk();
   saverowcount = rowcount;
   row = (rowcount >>= 1);
   if ((saverowcount & 1) != 0) /* odd line */
      row += ydots;
   else 			/* even line */
      if (dotmode != 11) /* display the line too */
	 out_line(pixels,linelen);
   for (col = 0; col < xdots; ++col)
      writedisk(col+sxoffs,row+syoffs,*(pixels+col));
   rowcount = saverowcount + 1;
   return(0);
}

static int call_line3d(unsigned char *pixels, int linelen)
{
   /* this routine exists because line3d might be in an overlay */
   return(line3d(pixels,linelen));
}

/* This function draws on the bitmap for the calculation engine */

void putcolor (int xdot, int ydot, int color)
{
    ULONG pm_y, offset;

    /* some calculation routines run [0..xdots] and/or [0..ydots],
       rather than [0..xdots-1] and [0..ydots-1] for a total of
       xdots by ydots.	We clip coordinates that will blow the
       pixel array quietly here.				  */
    if ( ! ((xdot >= xleft) && (xdot <= xright) &&
	    (ydot >= ytop)  && (ydot <= ybottom)) )
       return;

    pm_y = (ULONG) (cp.cy - ydot - 1);
    offset = ((ULONG) cp.pm_xdots * pm_y) + (ULONG) xdot;

    if (cp.pixelshift_per_byte == 0)
	{
	cp.pixels[offset] = (unsigned char) (color % cp.colors) ;
	}
     else
	{
	USHORT j;
	j = offset & cp.pixels_per_bytem1;
	offset = offset >> cp.pixelshift_per_byte;
	cp.pixels[offset] = (cp.pixels[offset] & cp.pm_notmask[j]) +
	    (((unsigned char)(color % cp.colors)) << cp.pm_bitshift[j]);
	}
}

int getcolor (int xdot, int ydot)
{
     ULONG pm_y, offset;

    if ( ! ((xdot >= xleft) && (xdot <= xright) &&
	    (ydot >= ytop)  && (ydot <= ybottom)) )
       return 0;	/* arbitrary value */

     pm_y = (ULONG) (cp.cy - ydot - 1);
     offset = ((ULONG) cp.pm_xdots * pm_y) + (ULONG) xdot;

     if (cp.pixelshift_per_byte == 0)
       {
	  return(cp.pixels[offset]);
       }
     else
       {
	USHORT j;
	j = offset & cp.pixels_per_bytem1;
	offset = offset >> cp.pixelshift_per_byte;
	return((int)((cp.pixels[offset] & cp.pm_andmask[j]) >> cp.pm_bitshift[j]));
       }
}

int keypressed (void)
{
    return (! cp.fContinueCalc) ;
}

/* Fractint-for-DOS memory get/free.
   Let's go directly to OS/2 */
unsigned char far *farmemalloc(long howmuch)
  {
  USHORT usSize;     /* howmuch as an unsigned short */
  SEL selGot;
  char far *pGot;

  if (howmuch > 65535L)  /* if asking for too much ... */
    return (char far *) NULL;  /* sorry */

  usSize = (USHORT) howmuch;
  if (DosAllocSeg(usSize, &selGot, SEG_NONSHARED) )
    return (char far *) NULL;	/* sorry, OS/2 don't have it */
  pGot = MAKEP(selGot,0);      /* cvt to pointer */
  return pGot;		       /* and go back */
  }

void   farmemfree(unsigned char far * it)
  {
  /* we assume what is farmemfree'd has been farmemalloc'd */
  SEL selIt;	    /* selector to free */

  selIt = SELECTOROF(it);
  DosFreeSeg(selIt);	 /* bye bye */

  return;		 /* and go back */
  }

int put_line(int rownum, int leftpt, int rightpt, unsigned char *localvalues)
{
int i;
  for (i = leftpt; i <= rightpt; i++)
       putcolor(i, rownum, localvalues[i-leftpt]);
  return 0;
}

int get_line(int rownum, int leftpt, int rightpt, unsigned char *localvalues)
{
int i;

   for (i = leftpt; i <= rightpt; i++)
      localvalues[i-leftpt] = (unsigned char) getcolor(i, rownum);
   return 0;
}

extern int rowcount;

int out_line(unsigned char *localvalues, int numberofdots)
{
    put_line(rowcount++, 0, numberofdots, localvalues);
    return 0;
}

int clear_screen(int forceclear)
{
  GpiErase(cp.hpsMemory);
  return(1);
}

/****************************************************************************

    FUNCTION: buzzer(int buzzertype)
			 0 = normal completion of task
			 1 = interrupted task
			 2 = error

    PURPOSE:  make some noise depending on the situation

****************************************************************************/

void buzzer(int i)
{

   switch(i)
	 {
	 case 1:
	       DosBeep(1047, 100);
	       DosBeep(1109, 100);
	       DosBeep(1175, 100);
	       break;

	 case 2:
	       DosBeep(2093, 100);
	       DosBeep(1976, 100);
	       DosBeep(1857, 100);
	       break;

	 case 3:
	       DosBeep(40, 500);
	       break;

	 default:
	       DosBeep(1000, 250);
	       break;
	 }

}

texttempmsg(char *msg1)
{

    PMfrSyncMsg(0, (PSZ) msg1);

}

stopmsg(int flags, char *msg1)
{

   return PMfrSyncMsg(flags, (PSZ) msg1);

}

int get_video_mode(struct fractal_info *info)
{
   viewwindow = viewxdots = viewydots = 0;
// fileaspectratio = .75;
// skipxdots = skipydots = 0;
   return(0);
}

restoredac()
{

}

/* The following procs tie up loose calc ends.	They are all dummy */

int getakey(void)
{
     longjmp (longjmpbuf, 5);
     return(999);
}

void helpmessage(unsigned char far *q)
{
     longjmp (longjmpbuf, 8);
}

void t16_flush(void)
{
     longjmp (longjmpbuf, 9);
}

void spindac(int a, int b)
{
  // longjmp (longjmpbuf, 10);
}

void ValidateLuts(void)
{
     longjmp (longjmpbuf, 13);
}

void t16_putline(void)
{
     longjmp (longjmpbuf, 17);
}

void t16_create(void)
{
     longjmp (longjmpbuf, 19);
}

void CustomLut(void)
{
     longjmp (longjmpbuf, 20);
}

void fullscreen_prompt(void)
{
     longjmp (longjmpbuf, 25);
}

/* Fractint Release 15.1 engine */

void setattr(void)
{
     longjmp (longjmpbuf, 26);
}

void _bios_serialcom(void)
{

}

void dvid_status(void)
{
     longjmp (longjmpbuf, 27);
}

void stackscreen(void)
{
     longjmp (longjmpbuf, 28);
}

void unstackscreen(void)
{
     longjmp (longjmpbuf, 29);
}

int thinking(int a, char * b)
{
return (keypressed());
}

int    putstringcenter(int a,int b,int c,int d,char far * e)
{

}

int helptitle()
{

}
/* **** **** **** **** **** **** */

#pragma optimize("",on)

/* other loose-end routines (from WINFRACT) */

goodbye()
{
/* naah - let's stick around */
}
void   putstring(int a,int b,int c,unsigned char far * d)
{
/* CMDFILES calls this in case of an error */
}
tovideotable()
{
/* CMDFILES calls this */
}

/* fake/not-yet-implemented subroutines */

int kbhit() { return(keypressed()); }
int getch() {return(13);}

void farmessage(unsigned char far *foo) {}
void setvideomode(int foo1, int foo2, int foo3, int foo4) {}
int fromvideotable() {}
int setforgraphics() {}
int setfortext() {}
int movecursor() {}
int home() {}
int _FAR_ _cdecl printf() {longjmp (longjmpbuf, 21);}
int help_overlay() {}
int prompts_overlay() {}
int rotate_overlay() {}
int printer_overlay() {}
int miscovl_overlay() {}
int pot_startdisk() {}
int SetTgaColors() {}
int startdisk() {}
int enddisk() {}
int readdisk() {}
int writedisk() {}
int nosnd(){}
int snd(){}
int targa_startdisk(){}
int targa_writedisk(){}
int SetColorPaletteName() {}
int get_3d_params() { return(0);}
int findfont() {return(0);}
int readticker(){return(0);}
int EndTGA(){}

