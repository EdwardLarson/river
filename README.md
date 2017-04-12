# river
Experimental language implementation

To compile, open a terminal in the src folder and run:

    g++ -o river river.cpp lexer.cpp vm/assembler.cpp vm/vm.c

Also tested with clang++. Other compilers probably work but those two are the only ones tested.

To compile an assembly file into bytecode:

    ./river assemble -i <input file> -o <output file> [-log]

To run a bytecode file:

    ./river run -i <input file> [-log]

To compile an assembly file and immediately run its bytecode:

    ./river runassembly -i <input file> [-o <intermediate assembly file>] [-log]
