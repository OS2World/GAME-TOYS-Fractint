        TITLE   'SmplHelp DLL Initialization Routine'
        PAGE    60,132

        .286p

DGROUP  GROUP   DATA

DATA    SEGMENT WORD PUBLIC 'DATA'
        EXTRN   _hmodHelp: WORD
DATA    ENDS

INIT    SEGMENT WORD PUBLIC 'CODE'

START   PROC    FAR
        ASSUME  CS:INIT,DS:DGROUP

        MOV     _hmodHelp, AX                   ; Capture our module handle

        MOV     AX, 1                           ; Indicate successful initialization
        RET                                     ; Go home

START   ENDP

INIT    ENDS
        END     START
