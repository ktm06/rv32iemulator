# we do this so we end on ecall for when we compile

.global _start
_start:
    call main
    ecall 
