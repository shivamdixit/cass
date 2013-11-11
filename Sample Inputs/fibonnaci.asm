;Program to generate fibonnaci series
START
 MOI A,0
 MOI B,1
 LDR C,2048H
 MAI ME,5000H
 STI A,ME
 INC ME
 DEC C
 STI B,ME
 DEC C
 LOP C
 MOI D,0
 INC M
 ADD D,A
 ADD D,B
 STI D,ME
 MVR A,B
 MVR B,D
 ELP
 HLT
