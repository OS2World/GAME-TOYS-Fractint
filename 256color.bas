' 256COLOR.BAS -- M. Covington 1990

DEFINT A-Z
DIM P&(0:256) ' AS LONG  ' the palette

' Set video mode

'SCREEN 13
PRINT "      Remapping colors..."

' Redefine colors 1-253

DIM L(3)
L(1) = 0        ' initial red, green, and blue levels
L(2) = 0
L(3) = 42

C = 1           ' which color is being defined (1-252)
W = 2           ' which element of V is changing
I = 1           ' what's being added to L(I)

FOR S = 1 TO 6          ' 6 sectors of color wheel
  FOR N = 1 TO 42       ' 42 colors in each
    P&(C) = L(1) + 256 * L(2) + 65536 * L(3)
    C = C + 1
    L(W) = L(W) + I
  NEXT
  W = (W MOD 3) + 1
  I = -I
NEXT

' Define the remaining colors

P&(0) = 0
P&(254) = P&(253) + 5
P&(255) = P&(254) + 5

' Set the palette

'PALETTE USING P(0)

' Demonstrate the results

'FOR C = 0 TO 255
'  LINE (C + 32, 0)-(C + 32, 180), C
'NEXT

OPEN "I:COV1.MAP" FOR OUTPUT AS #1
FOR C = 0 TO 255
B = (P&(C) MOD 256&) * 4
G = ((P&(C) \ 256&) MOD 256&) * 4
R = ((P&(C) \ 65536&) MOD 256&) * 4
PRINT #1,R,G,B
NEXT
END