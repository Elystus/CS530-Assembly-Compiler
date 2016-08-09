.sample program for SIC/XE architecture
.test program for prog4
.This file should assemble without errors

prog    start   0    .start of sample program
bsrch   EQU     $6000
OFFB    EQU     8 
        lds     #3
        +ldx     @foo
        +lda     beta,x
        add     gamma
        ldx     @zeta   
first   
        ldt     #alpha
        +ldt     #foo
        lda     @alpha
        lda     foo
        addr    S,T
        shiftr  T,2
        shiftl  S,1
        tixr    T
        j       first
        +j      first
        Clear   T
        +ldt    #50000	
        ldx     #0
        +jsub    bsrch
addl    lda     alpha,x   .loop control     
        add     beta,x
        sta     gamma,x     . extra space for testing                                
        addr    s,t
        compr   x,t
        +jlt     addl
        jlt     addl
        rsub  
        
. storage allocation section        
alpha   resw    10
beta    RESW    10
gamma   resw    10 
delta   WORD    20 
zeta    BYTE    C'EOF' 
eta     byte    X'FE'
theta	byte	c'eof'
buffer  RESB    30
foo     word    1
        end     prog    .end of sample program    
