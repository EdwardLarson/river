#include "VM.h"

#define INTEGRATE_ASSEMBLER

Register_File registerFile;

inline Data_Object* fetch_data(Byte* rawBytes);
Data_Object read_bytes(Byte* rawBytes);
Byte read_bool_direct(Byte* rawBegin);
IntegerType read_integer_direct(Byte* rawBegin);
RationalType read_double_direct(Byte* rawBegin);
PCType read_address_literal(Byte* rawBytes);

void initialize_register_file(Register_File* rFile);

Data_Object create_object_INTEGER(IntegerType n);
Data_Object create_object_RATIONAL(RationalType d);
Data_Object create_object_BOOL(Byte b);
inline void clear_data(Data_Object* object);

Data_Object* access_register(Byte reg){
	unsigned char depth = 0;
	// get depth if this is a non-persistent register
	if (reg < (NUM_REGISTERS - NUM_PERSISTENT_REGISTERS)) depth = registerFile.depth;
	
	return &(registerFile.registers[reg][depth] );
}

void write_default_output(Data_Object* object){
	*access_register(registerFile.defaultOutput) = *object;
}

void push_pc(PCType* pc_entry){
	++registerFile.pcTop; //TO-DO: bounds checking
	registerFile.pcStack[registerFile.pcTop] = *pc_entry;
}

PCType pop_pc(){
	--registerFile.pcTop; //TO-DO: bounds checking
	return registerFile.pcStack[registerFile.pcTop + 1];
}

void execute(Byte* byteStream, PCType* length);

#ifndef INTEGRATE_VM

int main(int argc, char** argv){
	
	if (argc < 2){
		printf("size of Byte:%ld\n", sizeof(Byte));
		printf("size of Data_Type:%ld\n", sizeof(Data_Type));
		printf("size of Data_Object:%ld\n", sizeof(Data_Object));
		printf("size of RationalType:%ld\n", sizeof(RationalType));
		printf("size of IntegerType:%ld\n", sizeof(IntegerType));
		printf("size of Data:%ld\n", sizeof(Data));
		printf("size of char*:%ld\n", sizeof(char*));
		printf("size of unsigned int:%ld\n", sizeof(unsigned int));
		
		PCType len = 19;
		
		Byte buffer[19] = {
			0x69, // print: opcode = 11010, funct = 01, r = 0: 0110-1001 = 0x69
			// begin object
			0x64,
			0x00,
			0x00,
			0x00,
			0x00,
			0x00,
			0x00,
			0x00,
			0x00,
			0x00,
			0x00,
			0x00,
			0x00,
			0x00,
			0x00,
			0x00,
			//end object
			0x6A, // print: opcode = 11010, funct = 10, r = 0: 0110-1010 = 0x6A
			0x20 // halt: opcode = 01000, funct = 00, r = 0: 0010-0000 = 0x10
		};
		
		PCType len2 = 42;
		
		Byte buffer2[42] = {
		//	LOAD	Data_Object.data = 5							Data_Object.type = 0	Data_Object spare		registerR = 1
			0xAC,	0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	0x00, 0x00, 0x00, 0x00,	0x00, 0x00, 0x00, 0x00,	0x01, //18 bytes
		//	ADD		registerA = 1	Data_Object.data = 3							Data_Object.type = 0	Data_Object spare		registerR = 2
			0x85,	0x01,			0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	0x00, 0x00, 0x00, 0x00,	0x00, 0x00, 0x00, 0x00,	0x02, //19 bytes
		//	PRINT	registerA = 1
			0x68,	0x01, // 2 bytes
		//	PRINT	registerA = 2
			0x68,	0x02, // 2 bytes
		//	HALT
			0x20
		};
		
		execute(buffer2, &len2);
		return 0;
	}
	
	char* filename = argv[1];
	
	
	FILE *fileptr;
	Byte *buffer;
	PCType filelen;

	fileptr = fopen(filename, "rb");  // Open the file in binary mode
	fseek(fileptr, 0, SEEK_END);          // Jump to the end of the file
	filelen = (PCType) ftell(fileptr);             // Get the current byte offset in the file
	rewind(fileptr);                      // Jump back to the beginning of the file

	buffer = (Byte *)malloc((filelen+1)*sizeof(Byte)); // Enough memory for file + \0
	fread(buffer, filelen, 1, fileptr); // Read in the entire file
	fclose(fileptr); // Close the file
	
	printf("Loaded a bytecode file of size %ld\n", filelen);
	
	execute(buffer, &filelen);
	
	return 0;
}

#endif

void execute(Byte* byteStream, PCType* length){
	PCType pc = 0;
	PCType pcNext = 0;
	
	Byte running = 1;
	
	Byte instruction;
	OPCODE opcode;
	Byte funct;
	Byte returnBit;
	
	IntegerType offset;
	void* finalAddress;
	
	// temporary argument and return pointers
	Data_Object* a;
	Data_Object* b;
	Data_Object* r;
	Data_Object tmpReturn; // r will point to here if an instruction returns a totally new object
	
	initialize_register_file(&registerFile);
	
	while(running && (pc < *length)){
		
		printf("\tpc = %ld\n", pc); ///DEBUG
		
		pcNext = pc;
		instruction = byteStream[pc];
		// instruction is structured as <rooo-ooff>
		//  r: return bit; returns to defaultOutput register if 0, to a return argument if 1
		//	o: bits of opcode; 
		//  f: bits of funct; select sub-functions of the operation, such as different argument types
		opcode = (instruction & 0x7C) >> 2;
		funct = (instruction & 0x03);
		returnBit = (instruction & 0x80);
		
		/// DEBUG
		printf("\topcode: %d\n", opcode);
		printf("\tfunct: %d\n", funct);
		printf("\treturnBit: %d\n", returnBit);
		
		switch (opcode){
//ABS
		case ABS:
			a = access_register(byteStream[pc + 1]);
			switch(a->type){
			case INTEGER:
				if (a->data.n < 0){
					tmpReturn = create_object_INTEGER(a->data.n * -1);
					r = &tmpReturn;
				}else{
					r = a;
				}
				break;
			case RATIONAL:
				if (a->data.d < 0){
					tmpReturn = create_object_RATIONAL(a->data.d * -1);
					r = &tmpReturn;
				}else{
					r = a;
				}
				break;
			default:
				r = a;
			}
			pcNext = pc + 2;
			break;
//ADD
		case ADD:
			switch(funct){
			case 0: // two registers
				a = access_register(byteStream[pc + 1]);
				b = access_register(byteStream[pc + 2]);
				pcNext = pc + 3;
				break;
			case 1: // one register and a constant
				a = access_register(byteStream[pc + 1]);
				b = fetch_data(&byteStream[pc + 2]);
				pcNext = pc + 2 + DATA_OBJECT_SIZE;
				break;
			}
			
			switch(a->type){
			case INTEGER:
				tmpReturn = create_object_INTEGER(a->data.n + b->data.n);
				r = &tmpReturn;
				break;
			case RATIONAL:
				tmpReturn = create_object_RATIONAL(a->data.d + b->data.d);
				r = &tmpReturn;
				break;
			case STRING: //TO-DO
				r = a;
				break;
			default:
				r = a;
			}
			break;
//AND
		case AND:
			break;
//BRANCH
		case BRANCH:
			// check given register
			if (read_bool_direct(&byteStream[pc + 1])){
				
				switch(funct){
				case 1: // linked branch
					{
					// get to next instruction
					PCType tmp = pc + 2 + DATA_OBJECT_SIZE;
					push_pc(&tmp);
					}
					// fall into default
				default:
					{
					//temporary cast union
					// union {
						// signed long asSigned; 
						// unsigned long asUnsigned;
					// } castUnion;
					// castUnion.asSigned = read_integer_direct(&byteStream[pc + 2]);
					pcNext = read_address_literal(&byteStream[pc + 2]);
					break;
					}
				}
			}else{
				pcNext = pc + 1 + sizeof(PCType);
			}
			
			pc = pcNext;
			continue;
//CALL
		case CALL:
			break;
//DIV
		case DIV:
			break;
//EQ
		case EQ:
			break;
//GT
		case GT:
			
			break;
//HALT
		case HALT:
			running = 0;
			printf("HALTING");
			continue;
//INPUT
		case INPUT:
			break;
//JUMP
		case JUMP:
			switch(funct){
			case 1: // linked jump
				{
				// get to next instruction
				PCType tmp = pc + 1 + sizeof(PCType);
				push_pc(&tmp);
				}
				// fall into default
			default:
				{
				//temporary cast union
				// union {
					// signed long asSigned; 
					// unsigned long asUnsigned;
				// } castUnion;
				// castUnion.asSigned = read_integer_direct(&byteStream[pc + 2]);
				// pc = castUnion.asUnsigned;
				pcNext = read_address_literal(&byteStream[pc + 1]);
				break;
				}
			}
			
			pc = pcNext;
			continue;
//LOAD
		case LOAD:
			
			///r = read_bytes(&byteStream[pc + 1]);
			r = fetch_data(&byteStream[pc + 1]);
			
			pcNext = pc + 1 + DATA_OBJECT_SIZE;
			
			printf("\tpcNext: %ld\n", pcNext); ///DEBUG
			break;
//LSH
		case LSH:
			
			break;
//LT
		case LT:
			break;
//MALLOC
		case MALLOC:
			break;
//MFREE			
		case MFREE:
			break;
//MLOAD
		case MLOAD:
			a = access_register(byteStream[pc + 1]);
			
			
			offset = 0;
			finalAddress = 0x00;
			
			switch(funct){
			case 1: // load offset Data_Object to r
				b = access_register(byteStream[pc + 2]);
				offset = b->data.n;
				
				pcNext = pc + 1;
				// no break, fall into next case
				case 0:  // load Data_Object to r
			
				// way to make compiler know real offset by pointer type?
				finalAddress = ((Data_Object*) a->data.p) + offset; // "a->data.p + b->data.n" if fallen into
				
				r = (Data_Object*) finalAddress;
				pcNext += 2; // "pcNext = pc + 3" if fallen into, "pcNext = pc + 2" otherwise
				break;
				
			case 3: // pure data load with offset
				b = access_register(byteStream[pc + 2]);
				offset = b->data.n;
				
				pcNext = pc + 1;
				//no break, fall into next case
				case 2: // pure data load
				clear_data(&tmpReturn);
				tmpReturn.type = access_register(byteStream[pc + 2])->type;
				
				finalAddress = a->data.p + offset; // literal offset values are useful for, say, object member loading
				
				switch(tmpReturn.type){
				case INTEGER:
					///finalAddress = ((IntegerType*) a->data.p) + offset;
					tmpReturn.data.n = *( (IntegerType*) finalAddress );
					break;
				case RATIONAL:
					///finalAddress = ((RationalType*) a->data.p) + offset;
					tmpReturn.data.d = *( (RationalType*) finalAddress );
					break;
				case NIL:
					break;
				case BOOL:
					///finalAddress = ((Byte*) a->data.p) + offset;
					tmpReturn.data.b = *( (Byte*) finalAddress );
					break;
				case STRING:
					///finalAddress = ((char**) a->data.p) + offset;
					tmpReturn.data.s = *( (char**) finalAddress );
					break;
				case POINTER:
					///finalAddress = ((void**) a->data.p) + offset;
					tmpReturn.data.p = *( (void**) finalAddress );
					break;
				}
				
				r = &tmpReturn;
				pcNext += 3; // "pcNext = pc + 4" if fallen into, "pcNext = pc + 3" otherwise
				
				break;
			}
			break;
//MOD
		case MOD:
			break;
//MOVE
		case MOVE:
			break;
//MUL
		case MUL:
			break;
//MSTORE		
		case MSTORE:
		
			//void* 
			finalAddress = 0x00;
			//IntegerType 
			offset = 0;
			
			a = access_register(pc + 1); // registerP
			b = access_register(pc + 2); // registerV
		
			switch(funct){
			case 1:
				offset = fetch_data(&byteStream[pc + 3])->data.n;
				pcNext = pc + 1;
				
				// no break, fall into next case
				case 0:
				
				// write
				*((Data_Object*) finalAddress) = *b;
				pcNext += 3;
				
				break;
			case 3:
				offset = fetch_data(&byteStream[pc + 3])->data.n;
				pcNext = pc + 1;
				
				// no break, fall into next case
				case 2:
				
				finalAddress = a->data.p + offset;
				
				// write
				switch(b->type){
				case INTEGER:
					*((IntegerType*) finalAddress) = b->data.n;
					break;
				case RATIONAL:
					*((RationalType*) finalAddress) = b->data.d;
					break;
				case NIL:
					///*((IntegerType*) finalAddress) = 0x0; UNDEFINED BEHAVIOR
					break;
				case BOOL:
					*((Byte*) finalAddress) = b->data.b;
					break;
				case STRING:
					*((char**) finalAddress) = b->data.s;
					break;
				case POINTER:
					*((void**) finalAddress) = b->data.p;
					break;
				}
				pcNext += 3;
				
				break;
			}
			
			break;
//NOT
		case NOT:
			break;
//OR
		case OR:
			break;
//POPFRAME
		case POPFRAME:
			--registerFile.depth;
			
			++pc;
			continue;
//PUSHFRAME
		case PUSHFRAME:
			++registerFile.depth;
			
			++pc;
			continue;
//POW
		case POW:
			break;
//PRINT
		case PRINT:
			printf("PRINTING\n");
			switch(funct){
			case 0: // print register
				a = access_register(byteStream[pc + 1]);
				pcNext = pc + 2;
				break;
			case 1: // print constant object
				a = fetch_data(&byteStream[pc + 1]);
				pcNext = pc + 1 + DATA_OBJECT_SIZE;
				break;
			case 2: // only printing a newline character, no arguments
				printf("\n");
				
				++pc;
				continue;
			}
			
			printf("\tactual type: %d\n", a->type);
			switch(a->type){
			case INTEGER:
				printf("INTEGER: %ld", a->data.n);
				break;
			case RATIONAL:
				printf("RATIONAL: %f", a->data.d);
				break;
			case BOOL:
				if (a->data.b) printf("BOOL: TRUE");
				else printf("BOOL: FALSE");
				break;
			case NIL:
				printf("NIL");
				break;
			case STRING:
				printf("STRING: %s", a->data.s);
				break;
			case POINTER:
				printf("POINTER: %p", a->data.p);
			}
			
			pc = pcNext;
			continue;
//RETURN
		case RETURN:
			pc = pop_pc();
			continue;
//RSH
		case RSH:
			break;
//SETDO
		case SETDO:
			registerFile.defaultOutput = byteStream[pc + 1];
			pcNext = pc + 2;
			
			pc = pcNext;
			continue;
//SUB
		case SUB:
			break;
//XOR
		case XOR:
			break;
		}
		
		// only leave switch if case block did not have 'continue', i.e. only instructions that have write values
		// after switch: pc should point to either last (return value) argument, or the next instruction
		// this is inferred from the return bit in the instruction
		
		if (returnBit){ // to returned register
			*access_register(byteStream[pcNext]) = *r;
			// increment pc past return register
			pc = pcNext + 1;
		}else{ // to defaultOutput
			write_default_output(r);
			pc = pcNext;
		}
	}
}

void initialize_register_file(Register_File* rFile){
	// initialize $0 to always 0
	Data_Object zeroObject = create_object_INTEGER(0);
	for (unsigned int i = 0; i < FRAME_STACK_SIZE; i++){
		rFile->registers[0][i] = zeroObject;
	}
}

Data_Object read_bytes(Byte* rawBytes){
	union{Data_Object asObject; Byte asBytes[DATA_OBJECT_SIZE];} castUnion;
	for (unsigned int i = 0; i < DATA_OBJECT_SIZE; i++){
		castUnion.asBytes[i] = rawBytes[i];
	}
	
	return castUnion.asObject;
}

Data_Object* fetch_data(Byte* rawBytes){
	return (Data_Object*) rawBytes;
}

PCType read_address_literal(Byte* rawBytes){
	union{PCType asLong; Byte asBytes[sizeof(PCType)];} castUnion;
	for (unsigned char i = 0; i < sizeof(PCType); i++){
		castUnion.asBytes[i] = rawBytes[i];
	}
	
	return castUnion.asLong;
}

Byte read_bool_direct(Byte* rawBytes){
	// DATA_OBJECT: <<Bddddddd><tttt><____>>
	return rawBytes[0];
}

IntegerType read_integer_direct(Byte* rawBytes){
	union {signed long asLong; Byte asBytes[8];} castUnion;
	
	for (unsigned int i = 0; i < 8; ++i){
		castUnion.asBytes[i] = rawBytes[i];
	}
	
	return castUnion.asLong;
}

RationalType read_double_direct(Byte* rawBytes){
	union {RationalType asDouble; Byte asBytes[8];} castUnion;
	
	for (unsigned char i = 0; i < 8; ++i){
		castUnion.asBytes[i] = rawBytes[i];
	}
	
	return castUnion.asDouble;
}

// clears only an object's data, type is untouched
void clear_data(Data_Object* object){
	object->data.n = 0;
}

// DATA_OBJECT FACTORIES
Data_Object create_object_INTEGER(IntegerType n){
	Data_Object object;
	object.type = INTEGER;
	object.data.n = n;
	
	return object;
}

Data_Object create_object_RATIONAL(RationalType d){
	Data_Object object;
	object.type = RATIONAL;
	object.data.d = d;
	
	return object;
}

Data_Object create_object_BOOL(Byte b){
	Data_Object object;
	object.type = BOOL;
	object.data.b = b;
	
	return object;
}

Data_Object create_object_STRING(const char* cstr){
	// count chars in cstr
	unsigned int cstrLength;
	for (cstrLength = 0; cstr[cstrLength] != '\0'; ++cstrLength);
	++cstrLength; // increment one more to count the entire array
	
	// allocate new char array
	char* newString = (char*) malloc(cstrLength * sizeof(char));
	
	// copy given array
	for (unsigned int i = 0; i < cstrLength - 1; i++){
		newString[i] = cstr[i];
	}
	
	Data_Object object;
	object.type = STRING;
	object.data.s = newString;
	
	return object;
}