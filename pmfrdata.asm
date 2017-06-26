;
;  FRACTINT for PM
;
;  12/22/90   Code by Donald P. Egen
;
;  PMFRDATA - External initialized data segments
;
;
; COLORSEG - Color tables
;

COLORSEG SEGMENT PARA PUBLIC 'FAR_DATA'

	 PUBLIC  _bmiColorTableVGA16
	 PUBLIC  _bmiColorTableVGA256
	 PUBLIC  _bmiColorTableBW
	 PUBLIC  _bmiColorTableWB
	 PUBLIC  _bmiColorTablePhys
	 PUBLIC  _bmiColorTableUser

;
;   16-Color table.
;     This is a 16-times repeat of the physical OS/2 VGA palette.
;
_bmiColorTableVGA16  LABEL DWORD
;  The start is a BITMAPINFOHEADER structure.
	 DD	  12   ; Length
	 DW	  0    ; cx
	 DW	  0    ; cy
	 DW	  1    ; cPlanes
	 DW	  8    ; cBitCount
;
;  Color RGB values (triplets).   Note: the values are really BBGGRR
;
;
	 IRP	  Count,<0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15>
	 DB	  000H,000H,000H     ;	Black, BB GG RR
	 DB	  080H,000H,000H     ;	Blue
	 DB	  000H,080H,000H     ;	Green
	 DB	  080H,080H,000H     ;	Cyan
	 DB	  000H,000H,080H     ;	Red
	 DB	  080H,000H,080H     ;	Magenta
	 DB	  000H,080H,080H     ;	Brown
	 DB	  080H,080H,080H     ;	White
	 DB	  0CCH,0CCH,0CCH     ;	Bright Black?
	 DB	  0FFH,000H,000H     ;	Bright Blue
	 DB	  000H,0FFH,000H     ;	Bright Green
	 DB	  0FFH,0FFH,000H     ;	Bright Cyan
	 DB	  000H,000H,0FFH     ;	Bright Red
	 DB	  0FFH,000H,0FFH     ;	Bright Pink
	 DB	  000H,0FFH,0FFH     ;	Bright Yellow
	 DB	  0FFH,0FFH,0FFH     ;	Bright White
	 ENDM

;
;   256-Color table.
;     This is DOS 256-color VGA physical palette.
;

_bmiColorTableVGA256 LABEL DWORD
;  The start is a BITMAPINFOHEADER structure.
	 DD	  12   ; Length
	 DW	  0    ; cx
	 DW	  0    ; cy
	 DW	  1    ; cPlanes
	 DW	  8    ; cBitCount
;
;  Color RGB values (triplets).   Note: the values are really BBGGRR
;
;
;	    BBB,RRR,GGG     R	G   B
	 DB   0,  0,  0;    0	0   0
	 DB 168,  0,  0;    0	0 168
	 DB   0,168,  0;    0 168   0
	 DB 168,168,  0;    0 168 168
	 DB   0,  0,168;  168	0   0
	 DB 168,  0,168;  168	0 168
	 DB   0, 84,168;  168  84   0
	 DB 168,168,168;  168 168 168
	 DB  84, 84, 84;   84  84  84
	 DB 252, 84, 84;   84  84 252
	 DB  84,252, 84;   84 252  84
	 DB 252,252, 84;   84 252 252
	 DB  84, 84,252;  252  84  84
	 DB 252, 84,252;  252  84 252
	 DB  84,252,252;  252 252  84
	 DB 252,252,252;  252 252 252
	 DB   0,  0,  0;    0	0   0
	 DB  20, 20, 30;   20  20  20
	 DB  32, 32, 32;   32  32  32
	 DB  44, 44, 44;   44  44  44
	 DB  56, 56, 56;   56  56  56
	 DB  68, 68, 68;   68  68  68
	 DB  80, 80, 80;   80  80  80
	 DB  96, 96, 96;   96  96  96
	 DB 112,112,112;  112 112 112
	 DB 128,128,128;  128 128 128
	 DB 144,144,144;  144 144 144
	 DB 160,160,160;  160 160 160
	 DB 180,180,180;  180 180 180
	 DB 200,200,200;  200 200 200
	 DB 224,224,224;  224 224 224
	 DB 252,252,252;  252 252 252
	 DB 252,  0,  0;    0	0 252
	 DB 252,  0, 64;   64	0 252
	 DB 252,  0,124;  124	0 252
	 DB 252,  0,188;  188	0 252
	 DB 252,  0,252;  252	0 252
	 DB 188,  0,252;  252	0 188
	 DB 124,  0,252;  252	0 124
	 DB  64,  0,252;  252	0  64
	 DB   0,  0,252;  252	0   0
	 DB   0, 64,252;  252  64   0
	 DB   0,124,252;  252 124   0
	 DB   0,188,252;  252 188   0
	 DB   0,252,252;  252 252   0
	 DB   0,252,188;  188 252   0
	 DB   0,252,124;  124 252   0
	 DB   0,252, 64;   64 252   0
	 DB   0,252,  0;    0 252   0
	 DB  64,252,  0;    0 252  64
	 DB 124,252,  0;    0 252 124
	 DB 188,252,  0;    0 252 188
	 DB 252,252,  0;    0 252 252
	 DB 252,188,  0;    0 188 252
	 DB 252,124,  0;    0 124 252
	 DB 252, 64,  0;    0  64 252
	 DB 252,124,124;  124 124 252
	 DB 252,124,156;  156 124 252
	 DB 252,124,188;  188 124 252
	 DB 252,124,220;  220 124 252
	 DB 252,124,252;  252 124 252
	 DB 220,124,252;  252 124 220
	 DB 188,124,252;  252 124 188
	 DB 156,124,252;  252 124 156
	 DB 124,124,252;  252 124 124
	 DB 124,156,252;  252 156 124
	 DB 124,188,252;  252 188 124
	 DB 124,220,252;  252 220 124
	 DB 124,252,252;  252 252 124
	 DB 124,252,220;  220 252 124
	 DB 124,252,188;  188 252 124
	 DB 124,252,156;  156 252 124
	 DB 124,252,124;  124 252 124
	 DB 156,252,124;  124 252 156
	 DB 188,252,124;  124 252 188
	 DB 220,252,124;  124 252 220
	 DB 252,252,124;  124 252 252
	 DB 252,220,124;  124 220 252
	 DB 252,188,124;  124 188 252
	 DB 252,156,124;  124 156 252
	 DB 252,180,180;  180 180 252
	 DB 252,180,196;  196 180 252
	 DB 252,180,216;  216 180 252
	 DB 252,180,232;  232 180 252
	 DB 252,180,252;  252 180 252
	 DB 232,180,252;  252 180 232
	 DB 216,180,252;  252 180 216
	 DB 196,180,252;  252 180 196
	 DB 180,180,252;  252 180 180
	 DB 180,196,252;  252 196 180
	 DB 180,216,252;  252 216 180
	 DB 180,232,252;  252 232 180
	 DB 180,252,252;  252 252 180
	 DB 180,252,232;  232 252 180
	 DB 180,252,216;  216 252 180
	 DB 180,252,196;  196 252 180
	 DB 180,252,180;  180 252 180
	 DB 196,252,180;  180 252 196
	 DB 216,252,180;  180 252 216
	 DB 232,252,180;  180 252 232
	 DB 252,252,180;  180 252 252
	 DB 252,232,180;  180 232 252
	 DB 252,216,180;  180 216 252
	 DB 252,196,180;  180 196 252
	 DB 112,  0,  0;    0	0 112
	 DB 112,  0, 28;   28	0 112
	 DB 112,  0, 56;   56	0 112
	 DB 112,  0, 84;   84	0 112
	 DB 112,  0,112;  112	0 112
	 DB  84,  0,112;  112	0  84
	 DB  56,  0,112;  112	0  56
	 DB  28,  0,112;  112	0  28
	 DB   0,  0,112;  112	0   0
	 DB   0, 28,112;  112  28   0
	 DB   0, 56,112;  112  56   0
	 DB   0, 84,112;  112  84   0
	 DB   0,112,112;  112 112   0
	 DB   0,112, 84;   84 112   0
	 DB   0,112, 56;   56 112   0
	 DB   0,112, 28;   28 112   0
	 DB   0,112,  0;    0 112   0
	 DB  28,112,  0;    0 112  28
	 DB  56,112,  0;    0 112  56
	 DB  84,112,  0;    0 112  84
	 DB 112,112,  0;    0 112 112
	 DB 112, 84,  0;    0  84 112
	 DB 112, 56,  0;    0  56 112
	 DB 112, 28,  0;    0  28 112
	 DB 112, 56, 56;   56  56 112
	 DB 112, 56, 68;   68  56 112
	 DB 112, 56, 84;   84  56 112
	 DB 112, 56, 96;   96  56 112
	 DB 112, 56,112;  112  56 112
	 DB  96, 56,112;  112  56  96
	 DB  84, 56,112;  112  56  84
	 DB  68, 56,112;  112  56  68
	 DB  56, 56,112;  112  56  56
	 DB  56, 68,112;  112  68  56
	 DB  56, 84,112;  112  84  56
	 DB  56, 96,112;  112  96  56
	 DB  56,112,112;  112 112  56
	 DB  56,112, 96;   96 112  56
	 DB  56,112, 84;   84 112  56
	 DB  56,112, 68;   68 112  56
	 DB  56,112, 56;   56 112  56
	 DB  68,112, 56;   56 112  68
	 DB  84,112, 56;   56 112  84
	 DB  96,112, 56;   56 112  96
	 DB 112,112, 56;   56 112 112
	 DB 112, 96, 56;   56  96 112
	 DB 112, 84, 56;   56  84 112
	 DB 112, 68, 56;   56  68 112
	 DB 112, 80, 80;   80  80 112
	 DB 112, 80, 88;   88  80 112
	 DB 112, 80, 96;   96  80 112
	 DB 112, 80,104;  104  80 112
	 DB 112, 80,112;  112  80 112
	 DB 104, 80,112;  112  80 104
	 DB  96, 80,112;  112  80  96
	 DB  88, 80,112;  112  80  88
	 DB  80, 80,112;  112  80  80
	 DB  80, 88,112;  112  88  80
	 DB  80, 96,112;  112  96  80
	 DB  80,104,112;  112 104  80
	 DB  80,112,112;  112 112  80
	 DB  80,112,104;  104 112  80
	 DB  80,112, 96;   96 112  80
	 DB  80,112, 88;   88 112  80
	 DB  80,112, 80;   80 112  80
	 DB  80,112, 88;   80 112  88
	 DB  80,112, 96;   80 112  96
	 DB 104,112, 80;   80 112 104
	 DB 112,112, 80;   80 112 112
	 DB 112,104, 80;   80 104 112
	 DB 112, 96, 80;   80  96 112
	 DB 112, 88, 80;   80  88 112
	 DB  64,  0,  0;    0	0  64
	 DB  64,  0, 16;   16	0  64
	 DB  64,  0, 32;   32	0  64
	 DB  64,  0, 48;   48	0  64
	 DB  64,  0, 64;   64	0  64
	 DB  48,  0, 64;   64	0  48
	 DB  32,  0, 64;   64	0  32
	 DB  16,  0, 64;   64	0  16
	 DB   0,  0, 64;   64	0   0
	 DB   0, 16, 64;   64  16   0
	 DB   0, 32, 64;   64  32   0
	 DB   0, 48, 64;   64  48   0
	 DB   0, 64, 64;   64  64   0
	 DB   0, 64, 48;   48  64   0
	 DB   0, 64, 32;   32  64   0
	 DB   0, 64, 16;   16  64   0
	 DB   0, 64,  0;    0  64   0
	 DB  16, 64,  0;    0  64  16
	 DB  32, 64,  0;    0  64  32
	 DB  48, 64,  0;    0  64  48
	 DB  64, 64,  0;    0  64  64
	 DB  64, 48,  0;    0  48  64
	 DB  64, 32,  0;    0  32  64
	 DB  64, 16,  0;    0  16  64
	 DB  64, 32, 32;   32  32  64
	 DB  64, 32, 40;   40  32  64
	 DB  64, 32, 48;   48  32  64
	 DB  64, 32, 56;   56  32  64
	 DB  64, 32, 64;   64  32  64
	 DB  56, 32, 64;   64  32  56
	 DB  48, 32, 64;   64  32  48
	 DB  40, 32, 64;   64  32  40
	 DB  32, 32, 64;   64  32  32
	 DB  32, 40, 64;   64  40  32
	 DB  32, 48, 64;   64  48  32
	 DB  32, 56, 64;   64  56  32
	 DB  32, 64, 64;   64  64  32
	 DB  32, 64, 56;   56  64  32
	 DB  32, 64, 48;   48  64  32
	 DB  32, 64, 40;   40  64  32
	 DB  32, 64, 32;   32  64  32
	 DB  40, 64, 32;   32  64  40
	 DB  48, 64, 32;   32  64  48
	 DB  56, 64, 32;   32  64  56
	 DB  64, 64, 32;   32  64  64
	 DB  64, 56, 32;   32  56  64
	 DB  64, 48, 32;   32  48  64
	 DB  64, 40, 32;   32  40  64
	 DB  64, 44, 44;   44  44  64
	 DB  64, 44, 48;   48  44  64
	 DB  64, 44, 52;   52  44  64
	 DB  64, 44, 60;   60  44  64
	 DB  64, 44, 64;   64  44  64
	 DB  60, 44, 64;   64  44  60
	 DB  52, 44, 64;   64  44  52
	 DB  48, 44, 64;   64  44  48
	 DB  44, 44, 64;   64  44  44
	 DB  44, 48, 64;   64  48  44
	 DB  44, 52, 64;   64  52  44
	 DB  44, 60, 64;   64  60  44
	 DB  44, 64, 64;   64  64  44
	 DB  44, 64, 60;   60  64  44
	 DB  44, 64, 52;   52  64  44
	 DB  44, 64, 48;   48  64  44
	 DB  44, 64, 44;   44  64  44
	 DB  48, 64, 44;   44  64  48
	 DB  52, 64, 44;   44  64  52
	 DB  60, 64, 44;   44  64  60
	 DB  64, 64, 44;   44  64  64
	 DB  64, 60, 44;   44  60  64
	 DB  64, 52, 44;   44  52  64
	 DB  64, 48, 44;   44  48  64
	 DB   0,  0,  0;    0	0   0
	 DB   0,  0,  0;    0	0   0
	 DB   0,  0,  0;    0	0   0
	 DB   0,  0,  0;    0	0   0
	 DB   0,  0,  0;    0	0   0
	 DB   0,  0,  0;    0	0   0
	 DB   0,  0,  0;    0	0   0
	 DB   0,  0,  0;    0	0   0

;
;   2-Color table. (Black and White)
;     This is not very interesting.
;

_bmiColorTableBW      LABEL DWORD
;  The start is a BITMAPINFOHEADER structure.
	 DD	  12   ; Length
	 DW	  0    ; cx
	 DW	  0    ; cy
	 DW	  1    ; cPlanes
	 DW	  8    ; cBitCount
;
;  Color RGB values (triplets).   Note: the values are really BBGGRR
;
;
;	 2 * 8 * 16 = 256
	 IRP	  Count1,<0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15>
	 IRP	  Count2,<0,1,2,3,4,5,6,7>
	 DB	 000H,000H,000H      ;	Black
	 DB	 0FFH,0FFH,0FFH      ;	White
	 ENDM
	 ENDM

;
;   2-Color table. (White and Black)
;     This is to preview 2-color printer hardcopy.
;

_bmiColorTableWB     LABEL DWORD
;  The start is a BITMAPINFOHEADER structure.
	 DD	  12   ; Length
	 DW	  0    ; cx
	 DW	  0    ; cy
	 DW	  1    ; cPlanes
	 DW	  8    ; cBitCount
;
;  Color RGB values (triplets).   Note: the values are really BBGGRR
;
;
;	 2 * 8 * 16 = 256
	 IRP	  Count1,<0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15>
	 IRP	  Count2,<0,1,2,3,4,5,6,7>
	 DB	 0FFH,0FFH,0FFH      ;	White
	 DB	 000H,000H,000H      ;	Black
	 ENDM
	 ENDM

;
;   Physical Palette Color table.
;     This is initialized as the physical OS/2 VGA palette.
;     It is filled in during initialization.
;
_bmiColorTablePhys   LABEL DWORD
;  The start is a BITMAPINFOHEADER structure.
	 DD	  12   ; Length
	 DW	  0    ; cx
	 DW	  0    ; cy
	 DW	  1    ; cPlanes
	 DW	  8    ; cBitCount
;
;  Color RGB values (triplets).   Note: the values are really BBGGRR
;
;
	 IRP	  Count,<0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15>
	 DB	  000H,000H,000H     ;	Black, BB GG RR
	 DB	  080H,000H,000H     ;	Blue
	 DB	  000H,080H,000H     ;	Green
	 DB	  080H,080H,000H     ;	Cyan
	 DB	  000H,000H,080H     ;	Red
	 DB	  080H,000H,080H     ;	Magenta
	 DB	  000H,080H,080H     ;	Brown
	 DB	  080H,080H,080H     ;	White
	 DB	  0CCH,0CCH,0CCH     ;	Bright Black?
	 DB	  0FFH,000H,000H     ;	Bright Blue
	 DB	  000H,0FFH,000H     ;	Bright Green
	 DB	  0FFH,0FFH,000H     ;	Bright Cyan
	 DB	  000H,000H,0FFH     ;	Bright Red
	 DB	  0FFH,000H,0FFH     ;	Bright Pink
	 DB	  000H,0FFH,0FFH     ;	Bright Yellow
	 DB	  0FFH,0FFH,0FFH     ;	Bright White
	 ENDM

;
;   Space for the User color table.
;     This is a initialized as a repeat of the 16-color table.
;     FILE/Read Map reads into this area.
;
_bmiColorTableUser   LABEL DWORD
;  The start is a BITMAPINFOHEADER structure.
	 DD	  12   ; Length
	 DW	  0    ; cx
	 DW	  0    ; cy
	 DW	  1    ; cPlanes
	 DW	  8    ; cBitCount
;
;  Color RGB values (triplets).   Note: the values are really BBGGRR
;
;
	 IRP	  Count,<0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15>
	 DB	  000H,000H,000H     ;	Black, BB GG RR
	 DB	  080H,000H,000H     ;	Blue
	 DB	  000H,080H,000H     ;	Green
	 DB	  080H,080H,000H     ;	Cyan
	 DB	  000H,000H,080H     ;	Red
	 DB	  080H,000H,080H     ;	Magenta
	 DB	  000H,080H,080H     ;	Brown
	 DB	  080H,080H,080H     ;	White
	 DB	  0CCH,0CCH,0CCH     ;	Bright Black?
	 DB	  0FFH,000H,000H     ;	Bright Blue
	 DB	  000H,0FFH,000H     ;	Bright Green
	 DB	  0FFH,0FFH,000H     ;	Bright Cyan
	 DB	  000H,000H,0FFH     ;	Bright Red
	 DB	  0FFH,000H,0FFH     ;	Bright Pink
	 DB	  000H,0FFH,0FFH     ;	Bright Yellow
	 DB	  0FFH,0FFH,0FFH     ;	Bright White
	 ENDM

COLORSEG ENDS
	 END
