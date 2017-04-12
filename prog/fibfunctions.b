MOVE #zero > #arg0
MOVE #zero > #return0
MOVE 5 > $a
MOVE 6 > $b
MOVE $a > #arg0
PUSHFRAME
JUMP_L :FIB
POPFRAME
MOVE #return0 > $a_fib
MOVE $b > #arg0
PUSHFRAME
JUMP_L :FIB
POPFRAME
MOVE #return0 > $b_fib
PRINT $a_fib
PRINT_B
PRINT $b_fib
PRINT_B
HALT
:FIB
MOVE #arg0 > $n
MOVE #zero > $compare
MOVE 1 > $fib_current
LT $n 2 > $compare
BRANCH $compare :FIBRETURN
MOVE 1 > $fib_prev
MOVE #zero > $fib_prev_prev
MOVE 2 > $i
:FIBLOOP
MOVE $fib_prev > $fib_prev_prev
MOVE $fib_current > $fib_prev
ADD $fib_prev $fib_prev_prev > $fib_current
ADD $i 1 > $i
LT $i $n > $compare
BRANCH $compare :FIBLOOP
:FIBRETURN
MOVE $fib_current > #return0
RETURN
