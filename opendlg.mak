# Makefile for OPENDLG.DLL
# Created by Microsoft Corporation, 1989
#

# Abbreviations
#
nd      = /codeview
ASM     = masm
CO	= cl -Zpei -W4 -c -u -Asnw -G2sc -Oxaz -B2 C2L -B3 C3L -NT
CD	= cl -Zpei -W4 -c -u -Asnw -G2sc -Od  -NT
LF      = /far /align:16 /map /nod $(nd) /li
OBJS    = tool.obj tool1.obj toola.obj data.obj\
          file.obj file1.obj init.obj gpi.obj
#
#  default to debugging compile
CC	= $(CD)

# Inference Rules
#
.asm.obj:
    $(ASM) $*.asm;

.c.obj:
    $(CC) _TEXT $*.c

#
# main target
#

ALL : opendlg.dll opendlg.lib


# Main files:  OPENDLG.DLL
#
opendlg.dll:	opendlg.mak opendlg.def opendlg.res opendlg.lib $(OBJS)
    echo $(OBJS)        >  opendlg.lnk
    echo opendlg.dll    >> opendlg.lnk
    echo opendlg.map    >> opendlg.lnk
    echo opendlg os2    >> opendlg.lnk
    echo opendlg.def    >> opendlg.lnk

    link $(LF) @opendlg.lnk
    mapsym opendlg
    rc opendlg.res opendlg.dll
    del opendlg.lnk

# Import Libraries
#
opendlg.lib:	opendlg.mak opendlg.def
    implib $*.lib $*.def

# Resources
#
opendlg.res:	opendlg.mak opendlg.dlg opendlg.h opendlg.rc tool.h
    rc -r opendlg.rc

# MASM files
#
toola.obj:	opendlg.mak toola.asm

# C files
#
tool.obj:	opendlg.mak tool.c

data.obj:	opendlg.mak data.c
    $(CC) _INIT data.c

gpi.obj:	opendlg.mak gpi.c
    $(CC) _INIT gpi.c

tool1.obj:	opendlg.mak tool1.c
    $(CC) _INIT tool1.c

init.obj:	opendlg.mak init.c
    $(CC) _INIT init.c

file.obj:	opendlg.mak file.c
    $(CC) _FILE file.c

file1.obj:	opendlg.mak file1.c
    $(CC) _FILE file1.c
