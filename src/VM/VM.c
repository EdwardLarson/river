#include "VM.h"

//TO-DO: void pointers should be cast to something that can hold them, not ints

#ifdef __cplusplus
#define void_ptr_add(ptr, offset) \
	(void*) (((unsigned long) ptr) + offset)
#else
#define void_ptr_add(ptr, offset) \
	ptr + offset
#endif

#define RIVER_VERSION_MAIN	0
#define RIVER_VERSION_SUB	1

void execute(Byte* byteStream, PCType* length){
	Meta_Data metaData;
	PCType pc = read_metadata(byteStream, length, &metaData);
	
	if (metaData.versionMain > RIVER_VERSION_MAIN){
		printf("Error: VM is version %i, but bytecode is for future version %i", RIVER_VERSION_MAIN, metaData.versionMain);
		return;
	}else if (metaData.versionMain < RIVER_VERSION_MAIN){
		printf("Warning: VM is version %i, but bytecode is for previous version %i", RIVER_VERSION_MAIN, metaData.versionMain);
	}
	
	printf("Running bytecode for version %i:%i\n", metaData.versionMain, metaData.versionSub);
	
	PCType pcNext = pc;
	
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
	
	Register_File registerFile;
	initialize_register_file(&registerFile);
	
	while(running && (pc < *length)){
		
		printf("\tpc = %ld\n", pc); ///DEBUG
		
		pcNext = pc;
		instruction = byteStream[pc];
		// instruction is structured as <rooo-ooff>
		//  r: return bit; returns to defaultOutput register if 0, to a return argument if 1
		//	o: bits of opcode; 
		//  f: bits of funct; select sub-functions of the operation, such as different argument types
		opcode = (OPCODE) ((instruction & 0x7C) >> 2);
		funct = (instruction & 0x03);
		returnBit = (instruction & 0x80);
		
		/// DEBUG
		printf("\topcode: %d\n", opcode);
		printf("\tfunct: %d\n", funct);
		printf("\treturnBit: %d\n", returnBit);
		
		switch (opcode){
//ABS
		case ABS:
			a = access_register(byteStream[pc + 1], &registerFile);
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
			pcNext += 2;
			break;
//ADD
		case ADD:
			switch(funct){
			case 0: // two registers
				a = access_register(byteStream[pc + 1], &registerFile);
				b = access_register(byteStream[pc + 2], &registerFile);
				pcNext += 3;
				break;
			case 1: // one register and a constant
				a = access_register(byteStream[pc + 1], &registerFile);
				b = fetch_data(&byteStream[pc + 2]);
				pcNext += 2 + DATA_OBJECT_SIZE;
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
					push_pc(&tmp, &registerFile);
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
				push_pc(&tmp, &registerFile);
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
			switch(funct){
			case 0: // load a normal constant
				r = fetch_data(&byteStream[pc + 1]);
				pcNext += 1 + DATA_OBJECT_SIZE;
				break;
			case 1: // load a string constant
				// read next two bytes as high and low of the constant's id
				{
				unsigned int stringID = byteStream[pc + 1];
				stringID = stringID << 8;
				stringID += byteStream[pc + 2];
				
				tmpReturn.type = STRING;
				tmpReturn.data.s = metaData.stringHeads[stringID];
				tmpReturn.aux[0] == metaData.stringLengs[stringID] >> 8;
				tmpReturn.aux[1] == metaData.stringLengs[stringID] & 0xFF;
				r = &tmpReturn;
				
				pcNext += 3;
				}
				break;
			}
			break;
//LSH
		case LSH:
			
			break;
//LT
		case LT:
			break;
//MALLOC
		case MALLOC:
			a = access_register(byteStream[pc + 1]);
			b = access_register(byteStream[pc + 2]);
			
			tmpReturn.type = POINTER;
			
			switch(funct){
			case 0:
				tmpReturn.data.p = malloc(DATA_OBJECT_SIZE * b->data.n);
				break;
			case 1:
				switch(a->data.type){
				case INTEGER:
					tmpReturn.data.p = malloc(sizeof(IntegerType) * b->data.n);
					break;
				case RATIONAL:
					tmpReturn.data.p = malloc(sizeof(RationalType) * b->data.n);
					break;
				case NIL:
					tmpReturn.data.p = 0x00;
					break;
				case BOOL:
					tmpReturn.data.p = malloc(sizeof(Byte) * b->data.n);
					break;
				case STRING:
					tmpReturn.data.p = malloc(sizeof(char*) * b->data.n);
					break;
				case POINTER:
					tmpReturn.data.p = malloc(sizeof(void*) * b->data.n);
					break;
				}
				break;
			}
			
			pcNext += 3;
			
			break;
//MFREE			
		case MFREE:
			a = access_register(byteStream[pc + 1]);
			
			switch(funct){
			case 0: 
				free(a->data.p);
				break;
			case 1:
				// check aux bytes
				if (0){
					free (a->data.p);
				}
				break;
			}
			
			pcNext += 2;
			
			break;
//MLOAD
		case MLOAD:
			a = access_register(byteStream[pc + 1], &registerFile);
			
			
			offset = 0;
			finalAddress = 0x00;
			
			switch(funct){
			case 1: // load offset Data_Object to r
				b = access_register(byteStream[pc + 2], &registerFile);
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
				b = access_register(byteStream[pc + 2], &registerFile);
				offset = b->data.n;
				
				pcNext = pc + 1;
				//no break, fall into next case
				case 2: // pure data load
				clear_data(&tmpReturn);
				tmpReturn.type = access_register(byteStream[pc + 2], &registerFile)->type;
				
				finalAddress = void_ptr_add(a->data.p, offset); // literal offset values are useful for, say, object member loading
				
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
			
			a = access_register(pc + 1, &registerFile); // registerP
			b = access_register(pc + 2, &registerFile); // registerV
		
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
				
				finalAddress = void_ptr_add(a->data.p, offset);
				
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
				a = access_register(byteStream[pc + 1], &registerFile);
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
			pc = pop_pc(&registerFile);
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
			*access_register(byteStream[pcNext], &registerFile) = *r;
			// increment pc past return register
			pc = pcNext + 1;
		}else{ // to defaultOutput
			write_default_output(r, &registerFile);
			pc = pcNext;
		}
	}
}

//==========================================================
// BYTESTREAM MANIPULATION
//==========================================================

Data_Object* fetch_data(Byte* rawBytes){
	return (Data_Object*) rawBytes;
}

Data_Object read_bytes(Byte* rawBytes){
	union{Data_Object asObject; Byte asBytes[DATA_OBJECT_SIZE];} castUnion;
	for (unsigned int i = 0; i < DATA_OBJECT_SIZE; i++){
		castUnion.asBytes[i] = rawBytes[i];
	}
	
	return castUnion.asObject;
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

//==========================================================
// DATA_OBJECT MANIPULATION
//==========================================================

PCType read_address_literal(Byte* rawBytes){
	union{PCType asLong; Byte asBytes[sizeof(PCType)];} castUnion;
	for (unsigned char i = 0; i < sizeof(PCType); i++){
		castUnion.asBytes[i] = rawBytes[i];
	}
	
	return castUnion.asLong;
}

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

// clears only an object's data, type is untouched
void clear_data(Data_Object* object){
	object->data.n = 0;
}

char* get_object_cstring(Data_Object* stringObject){
	return (stringObject->data.s) + STRING_LENGTH_BYTES;
}

//==========================================================
// REGISTER FILE FUNCTIONS
//==========================================================

void initialize_register_file(Register_File* rFile){
	// initialize #zero to always 0
	Data_Object zeroObject = create_object_INTEGER(0);
	
	rFile->registers[NUM_REGISTERS - NUM_PERSISTENT_REGISTERS][0] = zeroObject;
}

Data_Object* access_register(Byte reg, Register_File* rf){
	unsigned char depth = 0;
	// get depth if this is a non-persistent register
	if (reg < (NUM_REGISTERS - NUM_PERSISTENT_REGISTERS)) depth = rf->depth;
	
	return &(rf->registers[reg][depth]);
}

void write_default_output(Data_Object* object, Register_File* rf){
	*access_register(rf->defaultOutput, rf) = *object;
}

void push_pc(PCType* pc_entry, Register_File* rf){
	++rf->pcTop; //TO-DO: bounds checking
	rf->pcStack[rf->pcTop] = *pc_entry;
}

PCType pop_pc(Register_File* rf){
	--rf->pcTop; //TO-DO: bounds checking
	return rf->pcStack[rf->pcTop + 1];
}

//==========================================================
// METADATA FUNCTIONS
//==========================================================

PCType read_metadata(const Byte* byteStream, PCType* length, Meta_Data* metaData){
	PCType pc = 0;
	
	unsigned int stringLength;
	char* stringHead;
	unsigned int nstrings;
	
	while (pc < *length && byteStream[pc] != META_END){
		switch(byteStream[pc]){
		case META_BEGIN:
			pc += 1;
			break;
		case META_VERSION:
			// read version from next 2 bytes
			// version = <versionMain>:<versionSub> e.g. 1:24
			metaData->versionMain = byteStream[pc + 1];
			metaData->versionSub = byteStream[pc + 2];
			
			// move pc to next metadata
			pc += 3;
			
		case META_STRING:
			// read string length from next 2 bytes
			stringLength = byteStream[pc + 1];
			stringLength = stringLength << 8;
			stringLength += byteStream[pc + 2];
			// allocate memory for string
			stringHead = (char*) malloc(sizeof(char) * (stringLength));
			// read string characters
			for (unsigned int i = 0; i < stringLength; i++){
				stringHead[i] = (char) byteStream[pc + 3 + i];
			}
			
			// store string
			if (metaData->lastStored == metaData->nstrings){
				// more strings than expected, do nothing with this string
				printf("Error: more strings than expected in metadata\n");
				free(stringHead);
			}else{
				metaData->lastStored++;
				metaData->stringHeads[metaData->lastStored] = stringHead;
				metaData->stringLens[metaData->lastStored] = stringLength;
			}
			
			// move pc to next metadata
			pc += 3 + stringLength;
			
		case META_NSTRING:
			// read the number of string constants
			nstrings = byteStream[pc + 1];
			nstrings = nstrings << 8;
			nstrings += byteStream[pc + 2];
			
			// allocate memory for strings
			metaData->nstrings = nstrings;
			metaData->stringHeads = (char**) malloc(sizeof(char*) * nstrings);
			metaData->stringLens = (char**) malloc(sizeof(unsigned int) * nstrings);
			
			// move pc to next metadata
			pc += 3;
			break;
		case META_END:
			return pc + 1;
		default:
			pc += 1;
		}
	}
	
	// should not reach here
	return 0;
}