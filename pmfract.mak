#------------------
# PMFRACT make file
#------------------

# debug compile macros
cd=cl -c -W4 -Aln -G2s -FPi -Zpei -Od -FR
sd=cl -c -W1 -Aln -G2s -FPi -Zpei -Od -FR

# final compile macros
co=cl -c -W4 -Aln -G2s -FPi -Zpei -Oxaz -B2 C2L
so=cl -c -W1 -Aln -G2s -FPi -Zpei -Oat

# link options - nd= {null} to drop Codeview symbols
nd=/CO

# default compile is debugging mode
cf = $(sd)
cl = $(cd)


.SUFFIXES: .sbr .obj .c


# inference rules
.c.obj:
      $(cl) $*.c

.asm.obj:
#      masm /ML $*;
       ml /c /Cp /W3 /Zi /FR $*.asm

.c.sbr :
	cl /Zs -W1 -Alnw -G2sw -Zpei -Od /FR$@ $*.c

.asm.sbr:
       ml /c /Zs /Cp /W3 /Zi /FR $*.asm


SBRS = pmfract.sbr pmfrinit.sbr pmfrdlg1.sbr pmfrdlg2.sbr pmfruser.sbr \
       pmfrthrd.sbr pmfrcalc.sbr pmfrsave.sbr pmfrload.sbr pmfrprnt.sbr \
       fractals.sbr calcfrac.sbr testpt.sbr mpmath_c.sbr jb.sbr \
       3d.sbr cmdfiles.sbr decoder.sbr encoder.sbr f16.sbr \
       fracsubr.sbr gifview.sbr line3d.sbr loadfile.sbr lorenz.sbr \
       miscres.sbr parser.sbr plot3d.sbr tgaview.sbr lsys.sbr \
       calcmand.sbr newton.sbr mpmath_a.sbr fpu387.sbr fpu087.sbr \
       fracsuba.sbr pmgenerl.sbr pmfrdata.sbr

OBJS = pmfract.obj pmfrinit.obj pmfrdlg1.obj pmfrdlg2.obj pmfruser.obj \
       pmfrthrd.obj pmfrcalc.obj pmfrsave.obj pmfrload.obj pmfrprnt.obj \
       fractals.obj calcfrac.obj testpt.obj mpmath_c.obj jb.obj \
       3d.obj cmdfiles.obj decoder.obj encoder.obj f16.obj \
       fracsubr.obj gifview.obj line3d.obj loadfile.obj lorenz.obj \
       miscres.obj parser.obj plot3d.obj tgaview.obj lsys.obj \
       calcmand.obj newton.obj mpmath_a.obj fpu387.obj fpu087.obj \
       fracsuba.obj pmgenerl.obj pmfrdata.obj

#  dummy targets:
#	ALL is the main results (executable file)
#	bsc is the source browser data base
#
ALL: pmfract.exe

bsc: pmfract.bsc


# main result - exec file
pmfract.exe : $(OBJS) pmfract.def pmfract.lnk pmfract.res
     link $(nd) @pmfract.lnk
     rc pmfract.res
     mapsym pmfract

pmfract.res : pmfract.rc pmfract.dlg pmfract.h pmfract.ico intro.doc \
	     pmfract2.txt pmfract3.txt pmfract4.txt pmfract5.txt
     rc -r pmfract

# source browser data base
pmfract.bsc : $(SBRS)
	pwbrmake @<<
/o pmfract.bsc /Es $(SBRS)
<<

# main module and primary subroutines
pmfract.obj : pmfract.c pmfract.h fractint.h fractype.h

# initialization routines
pmfrinit.obj: pmfrinit.c pmfract.h fractint.h fractype.h

# fractal attributes dialog boxes routines
pmfrdlg1.obj: pmfrdlg1.c pmfract.h fractint.h fractype.h

# presentation attributes dialog boxes routines
pmfrdlg2.obj: pmfrdlg2.c pmfract.h fractint.h fractype.h

# user interface actions routines (main menu mostly)
pmfruser.obj: pmfruser.c pmfract.h fractint.h fractype.h

# background thread control module
pmfrthrd.obj: pmfrthrd.c pmfract.h fractint.h fractype.h

# file save background module
pmfrsave.obj: pmfrsave.c pmfract.h fractint.h

# file load background module
pmfrload.obj: pmfrload.c pmfract.h fractint.h

# print background module
pmfrprnt.obj: pmfrprnt.c pmfract.h fractint.h

# color tables
pmfrdata.obj: pmfrdata.asm

# fractal calculation driver (FRACTINT interface)
pmfrcalc.obj: pmfrcalc.c pmfract.h fractint.h fractype.h

# fractal calculation routines, from FRACTINT - C routines
calcfrac.obj: calcfrac.c fractint.h fractype.h mpmath.h targa_lc.h
       $(cf) calcfrac.c

fractals.obj: fractals.c fractint.h fractype.h
       $(cf) fractals.c

fracsubr.obj: fracsubr.c fractint.h fractype.h
       $(cf) fracsubr.c

lorenz.obj: lorenz.c fractint.h fractype.h
       $(cf) lorenz.c

testpt.obj: testpt.c fractint.h
       $(cf) testpt.c

parser.obj: parser.c
       $(cf) parser.c

jb.obj: jb.c fractint.h
       $(cf) jb.c

cmdfiles.obj: cmdfiles.c fractint.h fractype.h
       $(cf) cmdfiles.c

plot3d.obj: plot3d.c fractint.h 
       $(cf) plot3d.c

3d.obj: 3d.c fractint.h
       $(cf) 3d.c

loadfile.obj: loadfile.c fractint.h 
       $(cf) loadfile.c

decoder.obj: decoder.c fractint.h 
       $(cf) decoder.c

encoder.obj: encoder.c fractint.h 
       $(cf) encoder.c

gifview.obj: gifview.c fractint.h 
       $(cf) gifview.c

tgaview.obj: tgaview.c fractint.h 
       $(cf) tgaview.c

f16.obj: f16.c fractint.h 
       $(cf) f16.c

line3d.obj: line3d.c fractint.h 
       $(cf) line3d.c

miscres.obj: miscres.c fractint.h fractype.h
       $(cf) miscres.c

mpmath_c.obj: mpmath_c.c mpmath.h
       $(cf) mpmath_c.c

lsys.obj: lsys.c fractint.h
       $(cf) lsys.c

#   assembler routines
calcmand.obj: calcmand.asm

newton.obj: newton.asm
#      masm /e /ML newton;
       ml /c /Cp /FPi /W3 /Zi /FR newton.asm

newton.sbr: newton.asm
       ml /c /Zs /Cp /FPi /W3 /Zi /FR newton.asm

fracsuba.obj: fracsuba.asm

fpu387.obj : fpu387.asm

fpu087.obj : fpu087.asm
#      masm /e /ML fpu087;
       ml /c /Cp /FPi /W3 /Zi /FR fpu087.asm

fpu087.sbr:  fpu087.asm
       ml /c /Zs /Cp /FPi /W3 /Zi /FR fpu087.asm

mpmath_a.obj : mpmath_a.asm

#  cut down version of general.asm for pmfract
pmgenerl.obj: pmgenerl.asm
