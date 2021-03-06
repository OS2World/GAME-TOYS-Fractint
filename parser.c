/* Parser.c (C) 1990, Mark C. Peterson, CompuServe [70441,3353]
     All rights reserved.

   Code may be used in any program provided the author is credited
     either during program execution or in the documentation.  Source
     code may be distributed only in combination with public domain or
     shareware source code.  Source code may be modified provided the
     copyright notice and this message is left unchanged and all
     modifications are clearly documented.

     I would appreciate a copy of any work which incorporates this code,
     however this is optional.

     Mark C. Peterson
     253 West St., H
     Plantsville, CT 06479
     (203) 276-9474
*/

#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include "mpmath.h"

void findpath(char *filename, char *fullpathname);
void far *farmemalloc(long bytestoalloc);
void farmemfree(void far *farptr);
int  stopmsg(int,unsigned char far *);

MATH_TYPE MathType = D_MATH;
/* moved struct lcomplex and union ARg to mpmath.h -6-20-90 TIW */

struct ConstArg {
   char *s;
   int len;
   union Arg a;
};

struct PEND_OP {
   void (far *f)(void);
   int p;
};

/* PB 901103 made some of the following static for safety */
static struct PEND_OP far *o;

union Arg *Arg1, *Arg2;
static union Arg s[20], far *a, far * far *Store, far * far *Load;
static int StoPtr, LodPtr, OpPtr;

static void (far * far *f)(void) = (void(far * far *)(void))0;

static unsigned n, ErrPtr, posp, vsp, NextOp, LastOp, InitN;
static int paren, SyntaxErr, ExpectingArg;
static struct ConstArg far *v = (struct ConstArg far *)0;
static int InitLodPtr, InitStoPtr, InitOpPtr, LastInitOp, NumVar;
static int Delta16;
static double fgLimit;
static double fg;
static int ShiftBack;     /* TIW 06-18-90 */

extern int bitshift;
extern int bitshiftless1;
extern long multiply(long x, long y, int bitshift);
extern long divide(long x, long y, int bitshift);
extern int symmetry;          /* symmetry flag for calcmand()  */
extern double param[];

extern int debugflag;         /* BDT for debugging */
extern int row, col, overflow, cpu, fpu;
extern struct complex old, new;
extern double far *dx0, far *dy0;
extern long far *lx0, far *ly0;     /* BDT moved these to FAR */

#ifndef TESTING_MATH
   extern double far *dx1, far *dy1;
   extern long far *lx1, far *ly1;
   #define dShiftx dx1[row]
   #define dShifty dy1[col]
   #define lShiftx lx1[row]
   #define lShifty ly1[col]
#else
   #define dShiftx 0.0
   #define dShifty 0.0
   #define lShiftx 0L
   #define lShifty 0L
#endif

extern struct lcomplex lold, lnew;
extern char FormName[];

#define LastSqr v[4].a

static char * ErrStrings[] = {
   "Should be an Argument",
   "Should be an Operator",
   "')' needs a matching '('",
   "Need more ')'",
   "Undefined Operator",
   "Undefined Function",
   "More than one ','",
   "Table overflow"
};

void dStkAbs(void) {
   Arg1->d.x = fabs(Arg1->d.x);
   Arg1->d.y = fabs(Arg1->d.y);
}

void mStkAbs(void) {
   if(Arg1->m.x.Exp < 0)
      Arg1->m.x.Exp = -Arg1->m.x.Exp;
   if(Arg1->m.y.Exp < 0)
      Arg1->m.y.Exp = -Arg1->m.y.Exp;
}

void lStkAbs(void) {
   Arg1->l.x = labs(Arg1->l.x);
   Arg1->l.y = labs(Arg1->l.y);
}

void (*StkAbs)(void) = dStkAbs;

void dStkSqr(void) {
   LastSqr.d.x = Arg1->d.x * Arg1->d.x;
   LastSqr.d.y = Arg1->d.y * Arg1->d.y;
   Arg1->d.y = Arg1->d.x * Arg1->d.y * 2.0;
   Arg1->d.x = LastSqr.d.x - LastSqr.d.y;
   LastSqr.d.x += LastSqr.d.y;
}

void mStkSqr(void) {
   LastSqr.m.x = MPmul(Arg1->m.x, Arg1->m.x);
   LastSqr.m.y = MPmul(Arg1->m.y, Arg1->m.y);
   Arg1->m.y = MPmul(Arg1->m.x, Arg1->m.y);
   Arg1->m.y.Exp++;
   Arg1->m.x = MPsub(LastSqr.m.x, LastSqr.m.y);
   LastSqr.m.x = MPadd(LastSqr.m.x, LastSqr.m.y);
}

void lStkSqr(void) {
   LastSqr.l.x = multiply(Arg1->l.x, Arg1->l.x, bitshift);
   LastSqr.l.y = multiply(Arg1->l.y, Arg1->l.y, bitshift);
   Arg1->l.y = multiply(Arg1->l.x, Arg1->l.y, bitshift) << 1;
   Arg1->l.x = LastSqr.l.x - LastSqr.l.y;
   LastSqr.l.x += LastSqr.l.y;
}

void (*StkSqr)(void) = dStkSqr;

void dStkAdd(void) {
   Arg2->d.x += Arg1->d.x;
   Arg2->d.y += Arg1->d.y;
   Arg1--;
   Arg2--;
}

void mStkAdd(void) {
   Arg2->m = MPCadd(Arg2->m, Arg1->m);
   Arg1--;
   Arg2--;
}

void lStkAdd(void) {
   Arg2->l.x += Arg1->l.x;
   Arg2->l.y += Arg1->l.y;
   Arg1--;
   Arg2--;
}

void (*StkAdd)(void) = dStkAdd;

void dStkSub(void) {
   Arg2->d.x -= Arg1->d.x;
   Arg2->d.y -= Arg1->d.y;
   Arg1--;
   Arg2--;
}

void mStkSub(void) {
   Arg2->m = MPCsub(Arg2->m, Arg1->m);
   Arg1--;
   Arg2--;
}

void lStkSub(void) {
   Arg2->l.x -= Arg1->l.x;
   Arg2->l.y -= Arg1->l.y;
   Arg1--;
   Arg2--;
}

void (*StkSub)(void) = dStkSub;

void dStkConj(void) {
   Arg1->d.y = -Arg1->d.y;
}

void mStkConj(void) {
   Arg1->m.y.Exp ^= 0x8000;
}

void lStkConj(void) {
   Arg1->l.y = -Arg1->l.y;
}

void (*StkConj)(void) = dStkConj;

void dStkReal(void) {
   Arg1->d.y = 0.0;
}

void mStkReal(void) {
   Arg1->m.y.Mant = (long)(Arg1->m.y.Exp = 0);
}

void lStkReal(void) {
   Arg1->l.y = 0l;
}

void (*StkReal)(void) = dStkReal;

void dStkImag(void) {
   Arg1->d.x = 0.0;
}

void mStkImag(void) {
   Arg1->m.x.Mant = (long)(Arg1->m.x.Exp = 0);
}

void lStkImag(void) {
   Arg1->l.x = 0l;
}

void (*StkImag)(void) = dStkImag;

void dStkNeg(void) {
   Arg1->d.x = -Arg1->d.x;
   Arg1->d.y = -Arg1->d.y;
}

void mStkNeg(void) {
   Arg1->m.x.Exp ^= 0x8000;
   Arg1->m.y.Exp ^= 0x8000;
}

void lStkNeg(void) {
   Arg1->l.x = -Arg1->l.x;
   Arg1->l.y = -Arg1->l.y;
}

void (*StkNeg)(void) = dStkNeg;

void dStkMul(void) {
   FPUcplxmul(&Arg2->d, &Arg1->d, &Arg2->d);
   Arg1--;
   Arg2--;
}

void mStkMul(void) {
   Arg2->m = MPCmul(Arg2->m, Arg1->m);
   Arg1--;
   Arg2--;
}

void lStkMul(void) {
   long x, y;

   x = multiply(Arg2->l.x, Arg1->l.x, bitshift) -
       multiply(Arg2->l.y, Arg1->l.y, bitshift);
   y = multiply(Arg2->l.y, Arg1->l.x, bitshift) +
       multiply(Arg2->l.x, Arg1->l.y, bitshift);
   Arg2->l.x = x;
   Arg2->l.y = y;
   Arg1--;
   Arg2--;
}

void (*StkMul)(void) = dStkMul;

void dStkDiv(void) {
   FPUcplxdiv(&Arg2->d, &Arg1->d, &Arg2->d);
   Arg1--;
   Arg2--;
}

void mStkDiv(void) {
   Arg2->m = MPCdiv(Arg2->m, Arg1->m);
   Arg1--;
   Arg2--;
}

void lStkDiv(void) {
   long x, y, mod, x2, y2;

   mod = multiply(Arg1->l.x, Arg1->l.x, bitshift) +
         multiply(Arg1->l.y, Arg1->l.y, bitshift);
   x = divide(Arg1->l.x, mod, bitshift);
   y = -divide(Arg1->l.y, mod, bitshift);
   /* pb 900617 changed next 4 lines to use x2,y2 instead of x,y */
   x2 = multiply(Arg2->l.x, x, bitshift) - multiply(Arg2->l.y, y, bitshift);
   y2 = multiply(Arg2->l.y, x, bitshift) + multiply(Arg2->l.x, y, bitshift);
   Arg2->l.x = x2;
   Arg2->l.y = y2;
   Arg1--;
   Arg2--;
}

void (*StkDiv)(void) = dStkDiv;

void StkSto(void) {
   *Store[StoPtr++] = *Arg1;
}

void StkLod(void) {
   Arg1++;
   Arg2++;
   *Arg1 = *Load[LodPtr++];
}

void dStkMod(void) {
   Arg1->d.x = (Arg1->d.x * Arg1->d.x) + (Arg1->d.y * Arg1->d.y);
   Arg1->d.y = 0.0;
}

void mStkMod(void) {
   Arg1->m.x = MPCmod(Arg1->m);
   Arg1->m.y.Mant = (long)(Arg1->m.y.Exp = 0);
}

void lStkMod(void) {
   Arg1->l.x = multiply(Arg2->l.x, Arg1->l.x, bitshift) +
                 multiply(Arg2->l.y, Arg1->l.y, bitshift);
   if(Arg1->l.x < 0)
      overflow = 1;
   Arg1->l.y = 0L;
}

void (*StkMod)(void) = dStkMod;

void StkClr(void) {
   s[0] = *Arg1;
   Arg1 = &s[0];
   Arg2 = Arg1;
   Arg2--;
}

void dStkSin(void) {
   double sinx, cosx, sinhy, coshy;

   FPUsincos(&Arg1->d.x, &sinx, &cosx);
   FPUsinhcosh(&Arg1->d.y, &sinhy, &coshy);
   Arg1->d.x = sinx*coshy;
   Arg1->d.y = cosx*sinhy;
}

void mStkSin(void) {
   Arg1->d = MPC2cmplx(Arg1->m);
   dStkSin();
   Arg1->m = cmplx2MPC(Arg1->d);
}

void lStkSin(void) {
   int f;
   long x, y, sinx, cosx, sinhy, coshy;

   x = Arg1->l.x >> Delta16;
   y = Arg1->l.y >> Delta16;
   SinCos086(x, &sinx, &cosx);
   SinhCosh086(y, &sinhy, &coshy);
   Arg1->l.x = multiply(sinx, coshy, ShiftBack); /* TIW 06-18-90 */
   Arg1->l.y = multiply(cosx, sinhy, ShiftBack); /* TIW 06-18-90 */
}

void (*StkSin)(void) = dStkSin;

void dStkSinh(void) {
   double siny, cosy, sinhx, coshx;

   FPUsincos(&Arg1->d.y, &siny, &cosy);
   FPUsinhcosh(&Arg1->d.x, &sinhx, &coshx);
   Arg1->d.x = sinhx*cosy;
   Arg1->d.y = coshx*siny;
}

void mStkSinh(void) {
   Arg1->d = MPC2cmplx(Arg1->m);
   dStkSinh();
   Arg1->m = cmplx2MPC(Arg1->d);
}

void lStkSinh(void) {
   int f;
   long x, y, sinhx, coshx, siny, cosy;

   x = Arg1->l.x >> Delta16;
   y = Arg1->l.y >> Delta16;
   SinCos086(y, &siny, &cosy);
   SinhCosh086(x, &sinhx, &coshx);
   Arg1->l.x = multiply(cosy, sinhx, ShiftBack); /* TIW 06-18-90 */
   Arg1->l.y = multiply(siny, coshx, ShiftBack); /* TIW 06-18-90 */
}

void (*StkSinh)(void) = dStkSinh;

void dStkCos(void) {
   double sinx, cosx, sinhy, coshy;

   FPUsincos(&Arg1->d.x, &sinx, &cosx);
   FPUsinhcosh(&Arg1->d.y, &sinhy, &coshy);
   Arg1->d.x = cosx*coshy;
   Arg1->d.y = sinx*sinhy;
}

void mStkCos(void) {
   Arg1->d = MPC2cmplx(Arg1->m);
   dStkCos();
   Arg1->m = cmplx2MPC(Arg1->d);
}

void lStkCos(void) {
   int f;
   long x, y, sinx, cosx, sinhy, coshy;

   x = Arg1->l.x >> Delta16;
   y = Arg1->l.y >> Delta16;
   SinCos086(x, &sinx, &cosx);
   SinhCosh086(y, &sinhy, &coshy);
   Arg1->l.x = multiply(cosx, coshy, ShiftBack); /* TIW 06-18-90 */
   Arg1->l.y = multiply(sinx, sinhy, ShiftBack); /* TIW 06-18-90 */
}

void (*StkCos)(void) = dStkCos;

void dStkCosh(void) {
   double siny, cosy, sinhx, coshx;

   FPUsincos(&Arg1->d.y, &siny, &cosy);
   FPUsinhcosh(&Arg1->d.x, &sinhx, &coshx);
   Arg1->d.x = coshx*cosy;
   Arg1->d.y = sinhx*siny;
}

void mStkCosh(void) {
   Arg1->d = MPC2cmplx(Arg1->m);
   dStkCosh();
   Arg1->m = cmplx2MPC(Arg1->d);
}

void lStkCosh(void) {
   int f;
   long x, y, sinhx, coshx, siny, cosy;

   x = Arg1->l.x >> Delta16;
   y = Arg1->l.y >> Delta16;
   SinCos086(y, &siny, &cosy);
   SinhCosh086(x, &sinhx, &coshx);
   Arg1->l.x = multiply(cosy, coshx, ShiftBack); /* TIW 06-18-90 */
   Arg1->l.y = multiply(siny, sinhx, ShiftBack); /* TIW 06-18-90 */
}

void (*StkCosh)(void) = dStkCosh;

void dStkLT(void) {
   Arg2->d.x = (double)(Arg2->d.x < Arg1->d.x);
   Arg2->d.y = 0.0;
   Arg1--;
   Arg2--;
}

void mStkLT(void) {
   Arg2->m.x = fg2MP((long)(MPcmp(Arg2->m.x, Arg1->m.x) == -1), 0);
   Arg2->m.y.Mant = (long)(Arg2->m.y.Exp = 0);
   Arg1--;
   Arg2--;
}

void lStkLT(void) {
   Arg2->l.x = Arg2->l.x < Arg1->l.x;
   Arg2->l.y = 0l;
   Arg1--;
   Arg2--;
}

void (*StkLT)(void) = dStkLT;

void dStkLTE(void) {
   Arg2->d.x = (double)(Arg2->d.x <= Arg1->d.x);
   Arg2->d.y = 0.0;
   Arg1--;
   Arg2--;
}

void mStkLTE(void) {
   int comp;

   comp = MPcmp(Arg2->m.x, Arg1->m.x);
   Arg2->m.x = fg2MP((long)(comp == -1 || comp == 0), 0);
   Arg2->m.y.Mant = (long)(Arg2->m.y.Exp = 0);
   Arg1--;
   Arg2--;
}

void lStkLTE(void) {
   Arg2->l.x = Arg2->l.x <= Arg1->l.x;
   Arg2->l.y = 0l;
   Arg1--;
   Arg2--;
}

void (*StkLTE)(void) = dStkLTE;

void dStkLog(void) {
   FPUcplxlog(&Arg1->d, &Arg1->d);
}

void mStkLog(void) {
   Arg1->d = MPC2cmplx(Arg1->m);
   dStkLog();
   Arg1->m = cmplx2MPC(Arg1->d);
}

void lStkLog(void) {
   struct complex x;

   x.x = (double)Arg1->l.x / fg;
   x.y = (double)Arg1->l.y / fg;
   FPUcplxlog(&x, &x);
   if(fabs(x.x) < fgLimit && fabs(x.y) < fgLimit) {
      Arg1->l.x = (long)(x.x * fg);
      Arg1->l.y = (long)(x.y * fg);
   }
   else
      overflow = 1;
}

void (*StkLog)(void) = dStkLog;

void FPUcplxexp(struct complex *x, struct complex *z) {
   double e2x, siny, cosy;

   if(fpu == 387)
      FPUcplxexp387(x, z);
   else {
      e2x = exp(x->x);
      FPUsincos(&x->y, &siny, &cosy);
      z->x = e2x * cosy;
      z->y = e2x * siny;
   }
}

void dStkExp(void) {
   FPUcplxexp(&Arg1->d, &Arg1->d);
}

void mStkExp(void) {
   Arg1->d = MPC2cmplx(Arg1->m);
   FPUcplxexp(&Arg1->d, &Arg1->d);
   Arg1->m = cmplx2MPC(Arg1->d);
}

void lStkExp(void) {
   struct complex x;

   x.x = (double)Arg1->l.x / fg;
   x.y = (double)Arg1->l.y / fg;
   FPUcplxexp(&x, &x);
   if(fabs(x.x) < fgLimit && fabs(x.y) < fgLimit) {
      Arg1->l.x = (long)(x.x * fg);
      Arg1->l.y = (long)(x.y * fg);
   }
   else
      overflow = 1;
}

void (*StkExp)(void) = dStkExp;

void dStkPwr(void) {
   Arg2->d = ComplexPower(Arg2->d, Arg1->d);
   Arg1--;
   Arg2--;
}

void mStkPwr(void) {
   struct complex x, y;

   x = MPC2cmplx(Arg2->m);
   y = MPC2cmplx(Arg1->m);
   x = ComplexPower(x, y);
   Arg2->m = cmplx2MPC(x);
   Arg1--;
   Arg2--;
}

void lStkPwr(void) {
   struct complex x, y;

   x.x = (double)Arg2->l.x / fg;
   x.y = (double)Arg2->l.y / fg;
   y.x = (double)Arg1->l.x / fg;
   y.y = (double)Arg1->l.y / fg;
   x = ComplexPower(x, y);
   if(fabs(x.x) < fgLimit && fabs(x.y) < fgLimit) {
      Arg2->l.x = (long)(x.x * fg);
      Arg2->l.y = (long)(x.y * fg);
   }
   else
      overflow = 1;
   Arg1--;
   Arg2--;
}

void (*StkPwr)(void) = dStkPwr;

void EndInit(void) {
   LastInitOp = OpPtr;
}

struct ConstArg far *isconst(char *Str, int Len) {
   struct complex z;
   unsigned n, j;

   for(n = 0; n < vsp; n++) {
      if(v[n].len == Len) {
         if(!strnicmp(v[n].s, Str, Len))
            return(&v[n]);
      }
   }
   v[vsp].s = Str;
   v[vsp].len = Len;
   v[vsp].a.d.x = v[vsp].a.d.y = 0.0;
   if(isdigit(Str[0]) || Str[0] == '.') {
      if(o[posp-1].f == StkNeg) {
         posp--;
         Str = Str - 1;
         InitN--;
      }
      for(n = 1; isdigit(Str[n]) || Str[n] == '.'; n++);
      if(Str[n] == ',') {
         j = n + strspn(&Str[n+1], " \t\r\n") + 1;
         if(isdigit(Str[j]) || (Str[j] == '-' && isdigit(Str[j+1]))) {
            z.y = atof(&Str[j]);
            for(; isdigit(Str[j]) || Str[j] == '.' || Str[j] == '-'; j++);
            v[vsp].len = j;
         }
      }
      else
         z.y = 0.0;
      z.x = atof(Str);
      switch(MathType) {
         case D_MATH:
            v[vsp].a.d = z;
            break;
         case M_MATH:
            v[vsp].a.m = cmplx2MPC(z);
            break;
         case L_MATH:
            v[vsp].a.l.x = (long)(z.x * fg);
            v[vsp].a.l.y = (long)(z.y * fg);
            break;
      }
      v[vsp].s = Str;
   }
   return(&v[vsp++]);
}

struct FNCT_LIST {
   char *s;
   void (* *ptr)(void);
};

struct FNCT_LIST FnctList[] = {
   "sin", &StkSin,
   "sinh", &StkSinh,
   "cos", &StkCos,
   "cosh", &StkCosh,
   "sqr", &StkSqr,
   "log", &StkLog,
   "exp", &StkExp,
   "abs", &StkAbs,
   "conj", &StkConj,
   "real", &StkReal,
   "imag", &StkImag,
};

void NotAFnct(void) { }
void FnctNotFound(void) { }

void (far *isfunct(char *Str, int Len))(void) {
   unsigned n;

   n = strspn(&Str[Len], " \t\r\n");
   if(Str[Len+n] == '(') {
      for(n = 0; n < sizeof(FnctList) / sizeof(struct FNCT_LIST); n++) {
         if(strlen(FnctList[n].s) == Len) {
            if(!strnicmp(FnctList[n].s, Str, Len))
               return(*FnctList[n].ptr);
         }
      }
      return(FnctNotFound);
   }
   return(NotAFnct);
}

void RecSortPrec(void) {
   int ThisOp = NextOp++;

   while(o[ThisOp].p > o[NextOp].p && NextOp < posp)
      RecSortPrec();
   f[OpPtr++] = o[ThisOp].f;
}

static char *Constants[] = {
   "pixel",
   "p1",
   "p2",
   "z",
   "LastSqr",
};

struct SYMETRY {
   char *s;
   int n;
} SymStr[] = {
   "NOSYM",         0,
   "XAXIS_NOPARM", -1,
   "XAXIS",         1,
   "YAXIS_NOPARM", -2,
   "YAXIS",         2,
   "XYAXIS_NOPARM",-3,
   "XYAXIS",        3,
   "ORIGIN_NOPARM",-4,
   "ORIGIN",        4,
   "PI_SYM_NOPARM",-5,
   "PI_SYM",        5,
   "NOPLOT",       99,
   "", 0
};

int ParseStr(char *Str) {
   char memstr[80];
   struct ConstArg far *c;
   int CondFlag = 0, ModFlag = 999, Len, Equals = 0, Mod[20], mdstk = 0;
   struct ERROR { int n, s; } far *e;
   strcpy(memstr,"Insufficient memory to run fractal type 'formula'");

   e = (struct ERROR far *)farmemalloc(sizeof(struct ERROR) * 100L);
   if(!e || !o || !a || !Store || !Load || !v) {
      stopmsg(0,memstr); /* PB printf replaced by stopmsg */
      return(1);
   }
   switch(MathType) {
      case D_MATH:
         StkAdd = dStkAdd;
         StkSub = dStkSub;
         StkNeg = dStkNeg;
         StkMul = dStkMul;
         StkSin = dStkSin;
         StkSinh = dStkSinh;
         StkLT = dStkLT;
         StkLTE = dStkLTE;
         StkMod = dStkMod;
         StkSqr = dStkSqr;
         StkCos = dStkCos;
         StkCosh = dStkCosh;
         StkLog = dStkLog;
         StkExp = dStkExp;
         StkPwr = dStkPwr;
         StkDiv = dStkDiv;
         StkAbs = dStkAbs;
         StkReal = dStkReal;
         StkImag = dStkImag;
         StkConj = dStkConj;
         break;
      case M_MATH:
         StkAdd = mStkAdd;
         StkSub = mStkSub;
         StkNeg = mStkNeg;
         StkMul = mStkMul;
         StkSin = mStkSin;
         StkSinh = mStkSinh;
         StkLT = mStkLT;
         StkLTE = mStkLTE;
         StkMod = mStkMod;
         StkSqr = mStkSqr;
         StkCos = mStkCos;
         StkCosh = mStkCosh;
         StkLog = mStkLog;
         StkExp = mStkExp;
         StkPwr = mStkPwr;
         StkDiv = mStkDiv;
         StkAbs = mStkAbs;
         StkReal = mStkReal;
         StkImag = mStkImag;
         StkConj = mStkConj;
         break;
      case L_MATH:
         Delta16 = bitshift - 16;
         ShiftBack = 32 - bitshift; /* TW 06-18-90 */
         StkAdd = lStkAdd;
         StkSub = lStkSub;
         StkNeg = lStkNeg;
         StkMul = lStkMul;
         StkSin = lStkSin;
         StkSinh = lStkSinh;
         StkLT = lStkLT;
         StkLTE = lStkLTE;
         StkMod = lStkMod;
         StkSqr = lStkSqr;
         StkCos = lStkCos;
         StkCosh = lStkCosh;
         StkLog = lStkLog;
         StkExp = lStkExp;
         StkPwr = lStkPwr;
         StkDiv = lStkDiv;
         StkAbs = lStkAbs;
         StkReal = lStkReal;
         StkImag = lStkImag;
         StkConj = lStkConj;
         break;
   }
   for(vsp = 0; vsp < sizeof(Constants) / sizeof(char*); vsp++) {
      v[vsp].s = Constants[vsp];
      v[vsp].len = strlen(Constants[vsp]);
   }
   switch(MathType) {
      case D_MATH:
         v[1].a.d.x = param[0];
         v[1].a.d.y = param[1];
         v[2].a.d.x = param[2];
         v[2].a.d.y = param[3];
         break;
      case M_MATH:
         v[1].a.m.x = d2MP(param[0]);
         v[1].a.m.y = d2MP(param[1]);
         v[2].a.m.x = d2MP(param[2]);
         v[2].a.m.y = d2MP(param[3]);
         break;
      case L_MATH:
         v[1].a.l.x = (long)(param[0] * fg);
         v[1].a.l.y = (long)(param[1] * fg);
         v[2].a.l.x = (long)(param[2] * fg);
         v[2].a.l.y = (long)(param[3] * fg);
         break;
   }

   LastInitOp = ErrPtr = paren = OpPtr = LodPtr = StoPtr = posp = 0;
   SyntaxErr = -1;
   ExpectingArg = 1;
   for(n = 0; Str[n]; n++) {
      n += strspn(&Str[n], " \t\r\n");
      if(!Str[n])
         break;
      InitN = n;
      switch(Str[n]) {
         case '(':
            paren++;
            if(!ExpectingArg)
               SyntaxErr = 1;
            break;
         case ')':
            if(paren)
               paren--;
            else
               SyntaxErr = 2;
            if(ExpectingArg) {
               e[ErrPtr].n = InitN;
               e[ErrPtr++].s = 0;
            }
            break;
         case '|':
            if(ModFlag == paren-1) {
               if(ExpectingArg)
                  SyntaxErr = 0;
               paren--;
               ModFlag = Mod[--mdstk];
            }
            else {
               if(!ExpectingArg)
                  SyntaxErr = 1;
               Mod[mdstk++] = ModFlag;
               o[posp].f = StkMod;
               o[posp++].p = 2 - (paren + Equals)*15;
               ModFlag = paren++;
            }
            break;
         case ',':
         case ';':
            if(paren) {
               e[ErrPtr].n = InitN;
               e[ErrPtr++].s = 3;
            }
            if(ExpectingArg)
               SyntaxErr = 0;
            else
               ExpectingArg = 1;
            o[posp].f = (void(far*)(void))0;
            o[posp++].p = 15;
            o[posp].f = StkClr;
            o[posp++].p = -30000;
            Equals = paren = 0;
            break;
         case ':':
            if(paren) {
               e[ErrPtr].n = InitN;
               e[ErrPtr++].s = 3;
            }
            if(ExpectingArg)
               SyntaxErr = 0;
            else
               ExpectingArg = 1;
            o[posp].f = (void(far*)(void))0;
            o[posp++].p = 15;
            o[posp].f = EndInit;
            o[posp++].p = -30000;
            Equals = paren = 0;
            LastInitOp = 10000;
            break;
         case '+':
            if(ExpectingArg)
               SyntaxErr = 0;
            ExpectingArg = 1;
            o[posp].f = StkAdd;
            o[posp++].p = 4 - (paren + Equals)*15;
            break;
         case '-':
            if(ExpectingArg) {
               o[posp].f = StkNeg;
               o[posp++].p = 2 - (paren + Equals)*15;
            }
            else {
               o[posp].f = StkSub;
               o[posp++].p = 4 - (paren + Equals)*15;
               ExpectingArg = 1;
            }
            break;
         case '<':
            if(ExpectingArg)
               SyntaxErr = 0;
            ExpectingArg = 1;
            if(Str[n+1] == '=') {
               n++;
               o[posp].f = StkLTE;
            }
            else
               o[posp].f = StkLT;
            o[posp++].p = 6 - (paren + Equals)*15;
            break;
         case '*':
            if(ExpectingArg)
               SyntaxErr = 0;
            ExpectingArg = 1;
            o[posp].f = StkMul;
            o[posp++].p = 3 - (paren + Equals)*15;
            break;
         case '/':
            if(ExpectingArg)
               SyntaxErr = 0;
            ExpectingArg = 1;
            o[posp].f = StkDiv;
            o[posp++].p = 3 - (paren + Equals)*15;
            break;
         case '^':
            if(ExpectingArg)
               SyntaxErr = 0;
            ExpectingArg = 1;
            o[posp].f = StkPwr;
            o[posp++].p = 2 - (paren + Equals)*15;
            break;
         case '=':
            if(ExpectingArg)
               SyntaxErr = 0;
            ExpectingArg = 1;
            o[posp-1].f = StkSto;
            o[posp-1].p = 5 - (paren + Equals)*15;
            Store[StoPtr++] = Load[--LodPtr];
            Equals++;
            break;
         default:
            if(isalnum(Str[n]) || Str[n] == '.') {
               while(isalnum(Str[n+1]) || Str[n+1] == '.')
                  n++;
               if(!ExpectingArg) {
                  SyntaxErr = 1;
               }
               ExpectingArg = 0;
               Len = (n+1)-InitN;
               o[posp].f = isfunct(&Str[InitN], Len);
               if(o[posp].f != NotAFnct) {
                  if(o[posp].f == FnctNotFound) {
                     e[ErrPtr].n = InitN;
                     e[ErrPtr++].s = 5;
                  }
                  else
                     o[posp++].p = 1 - (paren + Equals)*15;
                  ExpectingArg = 1;
               }
               else {
                  c = isconst(&Str[InitN], Len);
                  Load[LodPtr++] = &(c->a);
                  o[posp].f = StkLod;
                  o[posp++].p = 1 - (paren + Equals)*15;
                  n = InitN + c->len - 1;
               }
            }
            else {
               if(ExpectingArg)
                  SyntaxErr = 0;
               ExpectingArg = 1;
               e[ErrPtr].n = InitN;
               e[ErrPtr++].s = 4;
            }
            break;
      }
      if(SyntaxErr >= 0) {
         e[ErrPtr].n = InitN;
         e[ErrPtr++].s = SyntaxErr;
         SyntaxErr = -1;
      }
      if(posp>=100-1) { /* PB 901103 added safety test here */
         e[ErrPtr].n = InitN;
         e[ErrPtr++].s = 7;
         break;
      }
   }

   o[posp].f = (void(far*)(void))0;
   o[posp++].p = 16;
   if(paren > 0) {
      e[ErrPtr].n = n;
      e[ErrPtr++].s = 3;
   }
   if (ErrPtr) {
      int i, j, k, m;
      char msgbuf[700];  /* PB replaced printf loop by build msgbuf & stopmsg */
      /* stopmsg defined to have max 9 lines, show at most first 3 errors */
      msgbuf[0] = 0;
      for(n = 0; n < ErrPtr && n < 3; n++) {
         if (n)
            strcat(msgbuf,"\n");
         sprintf(&msgbuf[strlen(msgbuf)], "Error(%d):  %s\n  ", e[n].s,
               ErrStrings[e[n].s]);
         j = 24;
         if ((i = e[n].n - j) < 0) {
            j = e[n].n;
            i = 0;
         }
         else {
            strcat(msgbuf,"...");
            j += 3;
         }
         k = strlen(msgbuf);
         m = i + 66;
         while (i < m && Str[i]) {
            if ((msgbuf[k] = Str[i]) == '\n' || msgbuf[k] == '\t')
               msgbuf[k] = ' ';
            ++i;
            ++k;
         }
         if (Str[i]) {
            msgbuf[k++] = '.';
            msgbuf[k++] = '.';
            msgbuf[k++] = '.';
         }
         msgbuf[k++] = '\n';
         while (--j >= -2)
            msgbuf[k++] = ' ';
         msgbuf[k++] = '^';
         msgbuf[k] = 0;
      }
      stopmsg(8,msgbuf);
   }
   if(!ErrPtr) {
      NextOp = 0;
      LastOp = posp;
      NumVar = vsp;
      if(f)
         farmemfree(f);
      f = (void(far * far *)(void))farmemalloc(sizeof(void(far * far *)(void))
            * (long)posp);
      if(!f) {
         stopmsg(0,memstr); /* PB printf replaced by stopmsg */
         return(1);
      }
      while(NextOp < posp) {
         if(o[NextOp].f)
            RecSortPrec();
         else {
            NextOp++;
            LastOp--;
         }
      }
   }
   else
      posp = 0;
   farmemfree(e);
/*
   farmemfree(o);
   farmemfree(a);
   farmemfree(Store);
   farmemfree(Load);
   farmemfree(v);
*/
   return(ErrPtr);
}

int Formula(void) {
   if(FormName[0] == 0 || overflow) return(1);

   LodPtr = InitLodPtr;
   StoPtr = InitStoPtr;
   OpPtr = InitOpPtr;

   Arg1 = &s[0];
   Arg2 = Arg1;
   Arg2--;
   while(OpPtr < LastOp)
      f[OpPtr++]();

   switch(MathType) {
      case D_MATH:
         old = new = v[3].a.d;
         return(Arg1->d.x == 0.0);
      case M_MATH:
         old = new = MPC2cmplx(v[3].a.m);
         return(Arg1->m.x.Exp == 0 && Arg1->m.x.Mant == 0);
      case L_MATH:
         lold = lnew = v[3].a.l;
         if(overflow)
            return(1);
         return(Arg1->l.x == 0L);
   }
   return(1);
}

int form_per_pixel(void) {
   double x, y;

   if (FormName[0] == 0) return(1);
   overflow = LodPtr = StoPtr = OpPtr = 0;
   Arg1 = &s[0];
   Arg2 = Arg1;
   Arg2--;
   switch(MathType) {
      case D_MATH:
         old.x = new.x = v[0].a.d.x = dx0[col]+dShiftx;
         old.y = new.y = v[0].a.d.y = dy0[row]+dShifty;
         break;
      case M_MATH:
         v[0].a.m.x = d2MP(old.x = new.x = dx0[col]+dShiftx);
         v[0].a.m.y = d2MP(old.y = new.y = dy0[row]+dShifty);
         break;
      case L_MATH:
         lold.x = lnew.x = v[0].a.l.x = lx0[col]+lShiftx;
         lold.y = lnew.y = v[0].a.l.y = ly0[row]+lShifty;
         break;
   }

   if(LastInitOp)
      LastInitOp = LastOp;
   while(OpPtr < LastInitOp)
      f[OpPtr++]();

   InitLodPtr = LodPtr;
   InitStoPtr = StoPtr;
   InitOpPtr = OpPtr;

   if(overflow)
      return(0);
   else
      return(1);
}

char *FormStr;

extern char FormFileName[];   /* BDT file to find the formulas in */
extern char FormName[];    /* BDT Name of the Formula (if not null) */

char *FindFormula(char *Str) {
   static char StrBuff[201];  /* PB, to match a safety fix in parser */
   char fullfilename[100]; /* BDT Full file name */
   int c, j, k, l;
   FILE *File;

   findpath(FormFileName, fullfilename);  /* BDT get full path name */

   symmetry = 0;
   if(File = fopen(fullfilename, "rt")) { /* BDT use variable files */
      while(fscanf(File, "%200[^ \n\t({]", StrBuff) != EOF) {
         if(!stricmp(StrBuff, Str) || !Str[0]) {
            while((c = getc(File)) != EOF) {
               if(c == '(') {
                  fscanf(File, "%200[^)]", StrBuff);
                  for(n = 0; SymStr[n].s[0]; n++) {
                     if(!stricmp(SymStr[n].s, StrBuff)) {
                        symmetry = SymStr[n].n;
                        break;
                     }
                  }
                  if(!SymStr[n].s[0]) {
                     sprintf(fullfilename,"Undefined symmetry:\n  %.76s",
                           StrBuff);
                     stopmsg(0,fullfilename); /* PB printf -> stopmsg */
                     fclose(File);
                     return((char*)0);
                  }
               }
               else if(c == '{')
                  break;
            }
            if(fscanf(File, "%200[^}]", StrBuff) != EOF) {
               Str = &StrBuff[strspn(StrBuff, " \n\t\r")];
               l = strlen(Str);
               for(j = 1; isspace(Str[l-j]); j++);
               Str[l-j+1] = 0;
               for(n = 0; n < l-j; n++)
                  if(isspace(Str[n]))
                     Str[n] = ' ';
               fclose(File);        /* BDT close the file */
               return(StrBuff);
            }
            else
               break;               /* PB moved close & msg to end of while */
         }
         fscanf(File, "%200[ \n\t({]", StrBuff);
         if(StrBuff[strcspn(StrBuff, "({")]) {
skipcomments:
            fscanf(File, "%200[^}]", StrBuff);
            if (getc(File)!= '}') goto skipcomments;
         }
      }
      fclose(File);                 /* BDT close the file */
      sprintf(fullfilename, "Formula \"%s\" not found", Str);
      stopmsg(0,fullfilename);      /* PB printf -> stopmsg */
   }
   else {
      sprintf(fullfilename, "Unable to open %s", FormFileName);
      stopmsg(0,fullfilename);      /* PB printf -> stopmsg */
   }
   return((char*)0);
}

int RunForm(char *Name) {
   if (FormName[0] == 0) return(1);
   parser_allocate();
   if(FormStr = FindFormula(Name))
      return(ParseStr(FormStr));
   else
      return(1);                    /* PB, msg moved to FindFormula */
}

int fpFormulaSetup(void) {
   if (fpu > 0) {
      MathType = D_MATH;
      return(!RunForm(FormName));
    }
    else {
       MathType = M_MATH;
       return(!RunForm(FormName));
    }
 }

int intFormulaSetup(void) {
   MathType = L_MATH;
   fg = (double)(1L << bitshift);
   fgLimit = (double)0x7fffffffL / fg;
   ShiftBack = 32 - bitshift;
   return(!RunForm(FormName));
}


/* TIW added 06-20-90 so functions can be called from fractals.c */
void init_misc()
{
   static struct ConstArg far vv[5];
   static union Arg argfirst,argsecond;
   if(!v) /* PB 901103 added this test to avoid clobbering the real thing */
      v = vv;  /* this is needed by lStkSqr and dStkSqr */
   Arg1 = &argfirst; Arg2 = &argsecond; /* needed by all the ?Stk* functions */
   fg = (double)(1L << bitshift);
   fgLimit = (double)0x7fffffffL / fg;
   ShiftBack = 32 - bitshift;
   Delta16 = bitshift - 16;
   bitshiftless1 = bitshift-1;
}

static int parser_allocated = 0;

parser_allocate()
{
   if (parser_allocated) return(0);
   parser_allocated = 1;

   o = (struct PEND_OP far *)farmemalloc(sizeof(struct PEND_OP) * 100L);
   a = (union Arg far *)farmemalloc(sizeof(union Arg) * 100L);
   Store = (union Arg far * far *)farmemalloc(sizeof(union Arg far *) * 100L);
   Load = (union Arg far * far *)farmemalloc(sizeof(union Arg far *) * 100L);
   v = (struct ConstArg far *)farmemalloc(sizeof(struct ConstArg) * 100L);

}
