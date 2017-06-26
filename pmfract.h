/*----------------------
   FRACTINT for PM
   PMFRACT.H header file
   05/24/91   Donald P. Egen
  ----------------------*/

#define ID_RESOURCE  1
#define ID_TIMER     1
#define ID_EMESSAGEBOX 2
#define ID_SYNCMSGBOX 3
#define ID_BADNUMERIC 4

#define IDT_TEXT        1024
#define IDT_HELP_INTRO   1         /* Introduction */
#define IDT_HELP_TYPES   2         /* Fractal Types */
#define IDT_HELP_EXTENT  3         /* Extents dialog help */
#define IDT_HELP_PARAMS  4         /* Set Params dialog help */
#define IDT_HELP_OPERATE 5         /* Operation help */

#define IDM_VIEW_MENU 29
#define IDM_ZIN_MENU  28
#define IDM_ZOUT_MENU 27
#define IDM_ZIN_2    2       /* *** WARNING **** */
#define IDM_ZIN_5    5       /* Code depends on values for IDM_ZIN_x */
#define IDM_ZIN_10   10
#define IDM_ZIN_PICK 9
#define IDM_ZOUT_2   12      /*  .. and for IDM_ZOUT_x */
#define IDM_ZOUT_5   15
#define IDM_ZOUT_10  20
#define IDM_ZOUT_PICK 19
#define IDM_PAN       21
#define IDM_ZIN_WIN   22
#define IDM_ZOUT_WIN  23

#define IDM_FILE_MENU   30
#define IDM_NEW_FRACTAL 31
#define IDM_READ_FILE   32
#define IDM_SAVE_FILE   33
#define IDM_READ_3D     34
#define IDM_READ_3DOVER 35
#define IDM_PRINT_FILE  36
#define IDM_READ_COLORMAP 37
#define IDM_WRITE_COLORMAP 38

#define IDM_EDIT_MENU   40
#define IDM_COPY_BMP    41
#define IDM_COPY_MET    42
#define IDM_PASTE       43
#define IDM_CLEAR_CLPB  44
#define IDM_CLPB_TEXT   45
#define IDM_CLPB_BMP    46
#define IDM_CLPB_MET    47

#define IDM_OPTIONS_MENU 50
#define IDM_SET_EXTENTS  51
#define IDM_SET_SWAP     52
#define IDM_SET_PARAMS   53
#define IDM_SET_OPTIONS  54
#define IDM_SET_RESET    55
#define IDM_SET_IMAGE    56
#define IDM_SET_PALETTE  57

#define IDM_HELP         60
#define IDM_HELP_INTRO   61
#define IDM_HELP_FRACTTYPE 62
#define IDM_HELP_OPERATE 63

#define IDM_FREEZE_HALT  95
#define IDM_GO           96

#define IDM_ABOUT       99

#define IDD_ABOUT    110
#define IDD_NUMBER_PICK 120
#define IDD_NUMBER_PROMPT 121
#define IDD_NUMBER_EF   122

#define IDD_SET_EXTENTS 130
#define IDD_SET_XLEFT   132
#define IDD_SET_XRIGHT  133
#define IDD_SET_YTOP    134
#define IDD_SET_YBOTTOM 135
#define IDD_SET_XCENTER 136
#define IDD_SET_YCENTER 137
#define IDD_SET_XHELP   138
#define IDD_SET_XDEF    139

#define IDD_SET_FRACTTYPE 140
#define IDD_SET_FRACTTYPE_LB 141
#define IDD_SET_THELP   142

#define IDD_SET_PARAMS  150
#define IDD_SET_P0      151     /* *** WARNING *** */
#define IDD_SET_P1      152     /* Code depends on IDD_SET_Px being */
#define IDD_SET_P2      153     /*   contiguous numbers   */
#define IDD_SET_P3      154
#define IDD_SET_PFRACTALTYPE 155
#define IDD_SET_P0TEXT  156
#define IDD_SET_P1TEXT  157
#define IDD_SET_P2TEXT  158
#define IDD_SET_P3TEXT  159
#define IDD_SET_PDEF    160
#define IDD_SET_T1TEXT  161     /* trig function parms */
#define IDD_SET_T2TEXT  162
#define IDD_SET_T3TEXT  163
#define IDD_SET_T4TEXT  164
#define IDD_SET_T1CB    165     /* *** WARNING *** */
#define IDD_SET_T2CB    166     /* Code depends on IDD_SET_TxCB being */
#define IDD_SET_T3CB    167     /*    contiguous numbers    */
#define IDD_SET_T4CB    168
#define IDD_SET_PHELP   169

#define IDD_SET_IMAGE   170
#define IDD_SET_IHELP   171
#define IDD_IMG_WIN     172
#define IDD_IMG_SCR     173
#define IDD_IMG_PRT     174
#define IDD_IMG_VGA     175
#define IDD_IMG_X       176
#define IDD_IMG_Y       177
#define IDD_IMG_C2      178
#define IDD_IMG_C16     179
#define IDD_IMG_C256    180

#define IDD_SET_PALETTE 190
#define IDD_SET_AHELP   191
#define IDD_PAL_BW      192
#define IDD_PAL_WB      193
#define IDD_PAL_VGA16   194
#define IDD_PAL_VGA256  195
#define IDD_PAL_PHYS    196
#define IDD_PAL_USER    197

#define IDD_SET_OPTIONS 210
#define IDD_SET_OHELP   211
#define IDD_SET_ODEF    212
#define IDD_OPT_PASSES1 213
#define IDD_OPT_PASSES2 214
#define IDD_OPT_PASSESG 215     /* solid-guessing */
#define IDD_OPT_PASSESB 216     /* boundary-trace */
#define IDD_OPT_MATHI   217     /* integer math */
#define IDD_OPT_MATHF   218     /* floating-point math */
#define IDD_OPT_MAXITER 219     /* maximum iterations */
#define IDD_OPT_BIOMORF 210     /* biomorph */
#define IDD_OPT_DECOMP  221     /* decomposition */
#define IDD_OPT_INCOLOR 222     /* inside color */
#define IDD_OPT_OUTCOLR 223     /* outside color */
#define IDD_OPT_DISTEST 224     /* distance estimator */

#define IDD_PRINT       250
#define IDD_PRINT_HELP  251
#define IDD_PRINT_PM    253
#define IDD_PRINT_PJ    254
#define IDD_PRINT_FILE  255
#define IDD_PRINT_PORT  256
#define IDD_PRINT_PORTNAME 257

#define IDD_SEL_LIST      270
#define IDD_SEL_LIST_WHAT 271
#define IDD_SEL_LIST_LB   272
#define IDD_SEL_LIST_HELP 273

#define IDD_LOADSAVE_TYPE 300
#define IDD_LST_WHAT      301
#define IDD_LST_WHAT_HELP 302
#define IDD_LST_GIF       305
#define IDD_LST_BMP       306
#define IDD_LST_MET       307
#define IDD_LST_WIN3BMP   308
#define IDD_LST_PCX       309

#define IDS_ZOOM_IN  1
#define IDS_ZOOM_OUT 2
#define IDS_FREEZE   3
#define IDS_HALT     4
#define IDS_TITLEBAR 5
#define IDS_SWAP_MANDEL 6
#define IDS_SWAP_JULIA  7
#define IDS_MISSING_SUB 8
#define IDS_FLOAT_FORMAT 9
#define IDS_VAL_NOTAPP 10
#define IDS_CLIENT_CLASS 11
#define IDS_BAD_NUMERIC 12
#define IDS_SUBTASK_FATAL 13
#define IDS_INIT_FATAL 14
#define IDS_WORKING 15
#define IDS_PRINT_NOOPEN 16
#define IDS_PRINT_ACTION 17
#define IDS_PRINT_HELP 18
#define IDS_OPEN_TITLE 19
#define IDS_OPEN_HELP  20
#define IDS_FORMULA_FILE 21
#define IDS_FORMULA_FILE_HELP 22
#define IDS_LSYSTEM_FILE 23
#define IDS_LSYSTEM_FILE_HELP 24
#define IDS_IFS_FILE 25
#define IDS_IFS_FILE_HELP 26
#define IDS_IFS3D_FILE 27
#define IDS_IFS3D_FILE_HELP 28
#define IDS_SEL_FORMULA 29
#define IDS_SEL_LSYSTEM 30
#define IDS_COLORMAP_TITLE 31
#define IDS_COLORMAP_HELP  32
#define IDS_LOAD_WHAT  33
#define IDS_SAVE_WHAT  34
#define IDS_EXT_GIF    35
#define IDS_EXT_BMP    36
#define IDS_EXT_MET    37
#define IDS_EXT_PCX    38

/*---------------------------------------------
   The sStatus variable implements a state machine
   that helps remember what we are doing, and it
   is interrogated and defines much of what goes on
   in the code.

   The states basically map as follows:

             [READY]------------->[FROZEN]
               A \   (FREEZE command)  V
               |  \                    | (Go)
       (Final  |   \                   |
        Paint) |    \  (GO)   /--------|
               |     \       /
               |      \     /
               |       V   V    (New Parms)
            [DONE]<---[WORKING]---------->[NEWPARMS]
                 (Subtask A  A               V  (SubTask Aborted)
                  Post)   |   \              |
   (STARTUP)[INIT]--------/    \             |
                 (subtask       \------------|
                  INIT_DONE)
             (goes to GO or LOAD)

 ----------------------------------------------*/
#define STATUS_INIT      0
#define STATUS_READY     1
#define STATUS_FROZEN    2
#define STATUS_WORKING   3
#define STATUS_DONE      4
#define STATUS_NEWPARMS  5

 /* define the codes used as communication between */
 /* the subtask and the main thread */
#define WM_THRD_POST       (WM_USER + 0)     /* main post from subtask */
                       /* SHORT1FROMMP(mp1) is a status code: */
#define SUB_STAT_OK             0  /* successful completion */
#define SUB_STAT_ABORT         -1  /* action aborted */
#define SUB_STAT_START_TIMER   -2  /* post to clear screen, start  */
                             /*  refresh timer (CalcDriver) */
#define SUB_STAT_FATAL         -3  /* fatal error (probably insufficient
                                      memory), subtask is gone */
#define SUB_INIT_DONE          -4  /* subtask init complete */

#define WM_SYNC_MSG        (WM_USER + 1)     /* sync message post from subtask */

 /* action codes to the subtask */
#define SUB_ACT_CALC            1  /* Calculate a Fractal */
#define SUB_ACT_PRINT           2  /* Print */
#define SUB_ACT_SAVE            3  /* Save */
#define SUB_ACT_LOAD            4  /* Load */
#define SUB_ACT_TERM          999  /* shutdown */

  /* sub-function codes for the subtask */
     /* sub-functions for print */
#define SUB_PRINT_PM            1  /* print to PM printer */
#define SUB_PRINT_PJ            2  /* print to a PaintJet */
     /* sub-functions for load and save */
#define SUB_LOADSAVE_GIF        1  /* load/save as GIF */
#define SUB_LOADSAVE_BMP        2  /* load/save as OS/2 PM BMP */
#define SUB_LOADSAVE_MET        3  /* load/save as Metafile */
#define SUB_LOADSAVE_WIN3BMP    4  /* load/save as Windows 3 BMP */
#define SUB_LOADSAVE_PCX        5  /* load/save as PCX file */

#define STACKSIZE   8192                   /* subtask stack size */
#define TIMER_REPAINT 10000

/* in the following TWO structures, like named elements must be */
/* the same types, as partial copies are effected continuously */

typedef struct _CALCPARM
     {
     HWND  hwnd ;
     volatile  BOOL  fContinueCalc ; /* subthread stop request */
     volatile  SHORT sSubAction ;    /* subthread command to process */
     volatile  SHORT sSubFunction ;  /* subthread sub-function of that command */
     ULONG ulSemTrigger ;     /* subthread start request */
     ULONG ulSemMemPS;        /* memory PS serialization */
     ULONG ulSemSubEnded;     /* subthread terminated post */
     HDC   hdcScreen;         /* screen window DC */
     HPS   hpsScreen;         /* screen window PS */
     HPS   hpsMemory;         /* memory DC/PS/BitMap for background drawing */
     HDC   hdcMemory;
     HBITMAP hbmMemory;
     PBITMAPINFO pbmiMemory;  /* current BITMAPINFO structure pointer */
                              /*   for Engine Pixel Array */
     USHORT sCurrentPalette;  /* current palette - IDD_PAL_xxx value */
     unsigned char _huge *pixels;   /* the Engine Pixel Array (pointer) */
     USHORT pm_xdots;         /* cx rounded to mult of 4-byte boundary, in pixels */
     USHORT pm_xbytes;        /* cx rounded to mult of 4-byte boundary, in bytes */
     USHORT cPlanes;          /* number of planes in Bitmap */
     USHORT cBitCount;        /* bits per plane in Bitmap */
     USHORT pixels_per_byte;  /* bitmap configuration data */
     LONG   pixels_per_bytem1;
     USHORT pixelshift_per_byte;
     unsigned char pm_andmask[8];
     unsigned char pm_notmask[8];
     unsigned char pm_bitshift[8];

     volatile  BOOL fNewBits;  /* flag to say rebuild mem bitmap from pixel
                                  array */
     volatile  BOOL fSuppressPaint;  /* flag to say to paint function:
                                        "Don't Touch Mem Bitmap or
                                         pixel Array"    */

     ULONG ulSemSyncMsg;      /* for Sync Message processing */
     ULONG ulSemSyncMsgDone;
     int   SyncMsgAnswer;

     BOOL  fNewParms;
     int   iFractType;
     char  cFloatflag;        /* float flag */
     SHORT cx, cy;            /* target image size in pixels */
     double XL, XR, YT, YB;   /* x left, x right, y top, y bottom values */
     double param[4];         /* parameters for fractal calc */
     unsigned char trigndx[4]; /* up to 4 trig fnct in fractal */
     char  szFileName[128];   /* load/save file name */
     char  szIfsFileName[128];/* ifs file name */
     char  szIfs3dFileName[128]; /* 3-d ifs file name */
     char  szFormFileName[128];  /* Forumla file name */
     char  szFormName[40];    /* formula selected from above */
     char  szLSysFileName[128]; /* L-System file name */
     char  szLSysName[40];    /* L-System formula selected from above */
     char  szPrintName[128];  /* name to print to */
     int   colors;            /* colors in the display */
     int   maxiter;           /* max iteration count */
     char  stdCalcMode;       /* '1', '2', 'b', 'g' */
     int   distest;           /* distance estimator parm */
     int   periodicitycheck;  /* RYFM */
     int   biomorph;          /* biomorph */
     int   decomp[2];         /* decomposition colors */
     int   inside;            /* inside color; 1=blue */
     int   outside;           /* outside color; -1=ignore */
     BOOL  fHaveUserPalette;  /* TRUE if we have read a user palette map */
     SHORT sLastLoadSaveType; /* remember load/save format between calls */
                              /* values are SUB_LOADSAVE_xxx values */
     }
    CALCPARAM ;

typedef CALCPARAM FAR *PCALCPARAM ;

typedef struct _NEWPARAM
     {
     BOOL  fNewParms;
     int   iFractType;        /* index into Fractal type array */
     char  cFloatflag;        /* float flag */
     SHORT cx, cy;            /* target image size in pixels */
     double XL, XR, YT, YB;   /* x left, x right, y top, y bottom values */
     double mxXL, mxXR, mxYT, mxYB;  /* left, right, top, bottom limits */
     double XCenter, YCenter;
     double param[4];         /* parameters for fractal calc */
     unsigned char trigndx[4]; /* up to 4 trig fnct in fractal */
     char  szFileName[128];   /* load/save file name */
     char  szIfsFileName[128];/* ifs file name */
     char  szIfs3dFileName[128]; /* 3-d ifs file name */
     char  szFormFileName[128];  /* Forumla file name */
     char  szFormName[40];    /* formula selected from above */
     char  szLSysFileName[128]; /* L-System file name */
     char  szLSysName[40];    /* L-System formula selected from above */
     char  szPrintName[128];  /* name to print to */
     int   colors;            /* colors in the display */
     int   maxiter;           /* max iteration count */
     char  stdCalcMode;       /* like he said */
     int   distest;           /* distance estimator parm */
     int   periodicitycheck;  /* like he said */
     int   biomorph;          /* biomorph */
     int   decomp[2];         /* decomposition colors */
     int   inside;            /* inside color; 1=blue */
     int   outside;           /* outside color; -1 = ignore */
     }
     NEWPARAM ;

typedef NEWPARAM FAR *PNEWPARAM ;

/* the following structure is the parameter to the General Selection
   dialog box and supporting routines */

#define MAX_SEL 101           /* maximum number of items selectable at once */

typedef struct _GENSEL
    {
    PSZ  pszTitle;             /* title to use in Dialog Box */
    char szSelected[21];       /* the name of what was selected now or last */
    SHORT sCountNames;         /* how many in the array of names */
    char szNames[MAX_SEL][21]; /* the actual array of names to select from */
    }
    GENSEL;

typedef GENSEL FAR *PGENSEL;

/*  routine prototypes */

VOID    PMfrGlobalInit(VOID);
BOOL    PMfrWindowInit(HWND hwnd, MPARAM mp2);
void FAR CalcDriverInit(void);
int  FAR CalcDriver (VOID) ;
void FAR PrintDriverInit(void);
int  FAR PrintDriver (VOID);
int  FAR SaveDriver (VOID) ;
int  FAR LoadDriver (VOID) ;
VOID FAR PMfrThread (VOID) ;
int  FAR PMfrSyncMsg (int flags, PSZ msg);
PSZ     GetFractalName(int iFractType);
VOID    EnableMenuItem (HWND hwnd, SHORT sMenuItem, BOOL fEnable) ;
VOID    UpdateMenuText (HWND hwnd, SHORT sMenuItem, PSZ szNewText);
BOOL    UpdateMenuStatus (HWND hwnd, SHORT sMenuID);
BOOL    GetMemoryBitmap (HAB);
VOID    InitNewParms(PNEWPARAM npTempParms);
VOID    ScheduleNewParms(HWND hwnd);
VOID    CopyParmsToNew(VOID);
VOID    CopyParmsToBase(VOID);
VOID    CalcZoomValues (PNEWPARAM npTempParms, double dFactor, BOOL InOrOut);
VOID    InitCrossHairs (VOID);
VOID    DrawCrossHairs (HWND hwnd, PPOINTL pptlMouse);
VOID    MoveCrossHairs (HWND hwnd, MPARAM mp1);
VOID    EraseCrossHairs (HWND hwnd);
VOID    DrawCrossHairsInner(HWND hwnd);
VOID    DrawZoomBoxInner (HWND hwnd);
VOID    InitZoomBox (SHORT x, SHORT y);
VOID    DrawZoomBox (HWND hwnd, PPOINTL pptlMouse);
VOID    MoveZoomBox (HWND hwnd, MPARAM mp1);
VOID    EraseZoomBox (HWND hwnd);
VOID    PanNewCenter (HWND hwnd);
VOID    ZoomNewWindow (HWND hwnd, BOOL InOrOut);
VOID    CancelZoomOrPan (HWND hwnd);
VOID    PMfrRepaint (HWND hwnd);
MRESULT PMfrCommands (HWND hwnd, USHORT msg, MPARAM mp1, MPARAM mp2);
VOID    SetSwitchEntry (HWND hwnd, PSZ szTitleBar, PSZ szFract);
VOID    CenterDialogBox (HWND hwnd);
VOID    LoadColorMap (PSZ szFileName);
VOID    SaveColorMap (PSZ szFileName);
VOID    FileFmtExt (PSZ far *ppszFmt);
VOID    PMfrFetchClipbrdBmp(HAB hab);
VOID    PMfrWriteClipbrdBmp(HAB hab);
MRESULT EXPENTRY ClientWndProc (HWND, USHORT, MPARAM, MPARAM) ;
MRESULT EXPENTRY AboutDlgProc  (HWND, USHORT, MPARAM, MPARAM) ;
MRESULT EXPENTRY SetExtentsDlgProc (HWND, USHORT, MPARAM, MPARAM) ;
MRESULT EXPENTRY ZoomValueDlgProc (HWND, USHORT, MPARAM, MPARAM) ;
MRESULT EXPENTRY SelFractalDlgProc (HWND, USHORT, MPARAM, MPARAM) ;
MRESULT EXPENTRY SetParametersDlgProc (HWND, USHORT, MPARAM, MPARAM) ;
MRESULT EXPENTRY SetImageDlgProc (HWND, USHORT, MPARAM, MPARAM) ;
MRESULT EXPENTRY SetPaletteDlgProc (HWND, USHORT, MPARAM, MPARAM) ;
MRESULT EXPENTRY SetOptionsDlgProc (HWND, USHORT, MPARAM, MPARAM) ;
MRESULT EXPENTRY PrintOptionsDlgProc (HWND, USHORT, MPARAM, MPARAM) ;
MRESULT EXPENTRY SelWhatDlgProc (HWND, USHORT, MPARAM, MPARAM) ;
MRESULT EXPENTRY LoadSaveFmtDlgProc (HWND, USHORT, MPARAM, MPARAM) ;

/* global variables */
/* this provides definition only where desired (main module) */
#if !defined(EXTERN)
#define EXTERN extern
#endif

EXTERN HAB     hab ;              /* main thread HAB */

EXTERN CALCPARAM _far cp ;        /* shared data with subtask thread */
EXTERN NEWPARAM _far npNewParms ; /* delayed copy for change of parms while working */
EXTERN NEWPARAM _far npTempParms ; /* copy of copy for dialogs to play with */
EXTERN POINTL  ptlCrossHairs;
EXTERN POINTL  ptlZBox1, ptlZBox2;
EXTERN HPOINTER hptrWait;         /* Hour-glass pointer */
EXTERN HPOINTER hptrArrow;        /* normal Arrow pointer */


EXTERN SHORT   sStatus;
EXTERN BOOL    flAmIcon;
EXTERN BYTE    bThreadStack [STACKSIZE];
EXTERN TID     tidCalc ;
EXTERN BOOL    fPan, fZoomWin, fActive, fGoodPan, fGoodZoom;
EXTERN ULONG   cxMouseTol, cyMouseTol;      /* x, y mouse move tolerances (SYS_DBLCLK) */
EXTERN ULONG   cxScreenRes, cyScreenRes ;   /* screen resolution */
EXTERN ULONG   cxScreen, cyScreen;          /* monitor x,y size */
EXTERN ULONG   cyIcon, cyMenu;              /* icon depth, menu depth */
EXTERN ULONG   cxPrtRes, cyPrtRes;          /* default printer resolution */
EXTERN ULONG   cxFullScreen, cyFullScreen;  /* max client window size */
EXTERN ULONG   cxPrinter, cyPrinter;        /* printer page size, pels */
EXTERN SHORT   cxClient, cyClient;          /* current client window size */
EXTERN ULONG   lScreenColors;               /* color capability of screen */
EXTERN ULONG   lPrinterColors;              /* color capability of printer */
EXTERN GENSEL _far gs;                      /* interface to General Selection
                                               dialog                       */

EXTERN CHAR _far szZoomIn[25];     /* Strings to fill in from resource file */
EXTERN CHAR _far szZoomOut[25];
EXTERN CHAR _far szFreeze[10];
EXTERN CHAR _far szHalt[10];
EXTERN CHAR _far szTitleBar[35];
EXTERN CHAR _far szSwapMandel[35];
EXTERN CHAR _far szSwapJulia[35];
EXTERN CHAR _far szMissingSub[35];
EXTERN CHAR _far szFloatFormat[10];
EXTERN CHAR _far szValueNotApplicable[10];
EXTERN CHAR _far szClientClass[10];
EXTERN CHAR _far szBadNumeric[50];
EXTERN CHAR _far szSubtaskFatal[50];
EXTERN CHAR _far szInitFatal[50];
EXTERN CHAR _far szWorking[20];
EXTERN CHAR _far szPrintNoOpen[35];
EXTERN CHAR _far szPrintAction[35];
EXTERN CHAR _far szPrintHelp[35];
EXTERN CHAR _far szOpenTitle[35];
EXTERN CHAR _far szOpenHelp[35];
EXTERN CHAR _far szFormTitle[35];
EXTERN CHAR _far szFormHelp[35];
EXTERN CHAR _far szLsysTitle[35];
EXTERN CHAR _far szLsysHelp[35];
EXTERN CHAR _far szIFSTitle[35];
EXTERN CHAR _far szIFSHelp[35];
EXTERN CHAR _far szIFS3DTitle[35];
EXTERN CHAR _far szIFS3DHelp[35];
EXTERN CHAR _far szSelFormula[35];
EXTERN CHAR _far szSelLsystem[35];
EXTERN CHAR _far szColorMapTitle[40];
EXTERN CHAR _far szColorMapHelp[40];
EXTERN CHAR _far szLoadWhatFmt[35];
EXTERN CHAR _far szSaveWhatFmt[35];
EXTERN CHAR _far szExtGIF[10];
EXTERN CHAR _far szExtBMP[10];
EXTERN CHAR _far szExtMET[10];
EXTERN CHAR _far szExtPCX[10];

EXTERN int     MainArgc;
EXTERN char  **MainArgv;

  /* for color table switching */
extern BITMAPINFO _far bmiColorTableVGA16;  /* color tables are in external */
extern BITMAPINFO _far bmiColorTableVGA256;   /*  assembler file */
extern BITMAPINFO _far bmiColorTableBW;
extern BITMAPINFO _far bmiColorTableWB;
extern BITMAPINFO _far bmiColorTableUser;
extern BITMAPINFO _far bmiColorTablePhys;
EXTERN ULONG      _far *alPhysicalColors;

EXTERN SHORT     CountFractalList;          /* names of fractals in the */
EXTERN SHORT far *asFracTypeList;        /*  engine table -fractalspecific- */

#define SYSVAL(x)  ( (USHORT) WinQuerySysValue(HWND_DESKTOP, x) )

#define MAX_TRIG   6   /* SQR  -   should be in fractype.h but isn't */
