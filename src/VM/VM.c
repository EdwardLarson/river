#include "VM.h"

//TO-DO: void pointers should be cast to something that can hold them, not ints

#ifdef __cplusplus
#define void_ptr_add(ptr, offset) \
	(void*) (((unsigned long) ptr) + offset)
#else
#define void_ptr_add(ptr, offset) \
	ptr + offset
#endif

void execute(const Byte* byteStream, const PCType* length, Byte log){
	if (log){
		printf("Executing %ld bytes...\n", *length);
	} 
	
	Meta_Data metaData;
	PCType pc = read_metadata(byteStream, length, &metaData);
	
	if (metaData.versionMain > WWHEEL_VERSION_MAIN){
		printf("Error: VM is version %i, but bytecode is for future version %i", WWHEEL_VERSION_MAIN, metaData.versionMain);
		return;
	}else if (metaData.versionMain < WWHEEL_VERSION_MAIN){
		printf("Warning: VM is version %i, but bytecode is for previous version %i", WWHEEL_VERSION_MAIN, metaData.versionMain);
	}
	
	printf("Running bytecode for version %i:%i\n", metaData.versionMain, metaData.versionSub);
	if (log){
		printf("Execution beginning at pc=%ld\n", pc);
	}
	
	PCType pcNext = pc + 1;
	
	Byte running = 1;
	
	int instructionNo = 1;
	Byte limitInstructions = 1;
	
	Byte instruction;
	OPCODE opcode;
	Byte funct;
	
	unsigned long offset;
	void* finalAddress;
	
	// temporary argument and return pointers
	const Data_Object* a;
	const Data_Object* b;
	const Data_Object* r;
	Data_Object tmpReturn; // r will point to here if an instruction returns a totally new object
	
	Register_File registerFile;
	initialize_register_file(&registerFile);
	
	while(running && (pc < *length) && (!limitInstructions || instructionNo < INSTRUCTION_LIMIT)){
		
		++instructionNo;
		
		pcNext = pc + 1; // automatically incremented 1 byte for instruction, so operations should only increment it for argument bytes
		
		instruction = byteStream[pc];
		// instruction is structured as <rooo-ooff>
		//  r: return bit; returns to defaultOutput register if 0, to a return argument if 1
		//	o: bits of opcode; 
		//  f: bits of funct; select sub-functions of the operation, such as different argument types
		opcode = (OPCODE) ((instruction & 0xFC) >> 2);
		funct = (instruction & 0x03);
		
		/// DEBUG
		if (log){
			printf("\tpc = %ld\n", pc); ///DEBUG
			printf("\topcode: %d\n", opcode);
			printf("\tfunct: %d\n", funct);
		}
		
		switch (opcode){
//ABS
		case ABS:
			a = read_register(byteStream[pc + 1], &registerFile);
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
			pcNext += 1;
			break;
//ADD
		case ADD:
			switch(funct){
			case 0: // two registers
				a = read_register(byteStream[pc + 1], &registerFile);
				b = read_register(byteStream[pc + 2], &registerFile);
				pcNext += 2;
				break;
			case 1: // one register and a constant
				a = read_register(byteStream[pc + 1], &registerFile);
				b = fetch_data(&byteStream[pc + 2]);
				pcNext += 1 + DATA_OBJECT_SIZE;
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
			pcNext += 1 + sizeof(PCType);
			// check given register
			if (read_register(byteStream[pc + 1], &registerFile)->data.b){
				
				switch(funct){
				case 1: // linked branch
					{
					// get to next instruction
					///PCType tmp = pcNext + 1 + sizeof(PCType);
					push_pc(&pcNext, &registerFile);
					}
					// fall into default
				default:
					{
					pcNext = read_address_literal(&byteStream[pc + 2]);
					break;
					}
				}
			}
			///else{
			///	pcNext += 1 + sizeof(PCType);
			///}
			
			pc = pcNext;
			continue;
//CALL
		case CALL:
			
			a = read_register(byteStream[pc + 1], &registerFile);
			
			++(registerFile.depth);
			
			pcNext += 1;
			if (log)
				printf("pushed pcNext %ld, reading in a new pc: %ld\n", pcNext, a->data.f);
			push_pc(&pcNext, &registerFile);
			
			pcNext = a->data.f;
			pc = pcNext;
			continue;
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
			if (log) printf("HALTING");
			continue;
//INPUT
		case INPUT:
			break;
//JUMP
		case JUMP:
			pcNext += sizeof(PCType);
			switch(funct){
			case 1: // linked jump
				{
				// get to next instruction
				///PCType tmp = pcNext + sizeof(PCType);
				push_pc(&pcNext, &registerFile);
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
//LSH
		case LSH:
			
			break;
//LT
		case LT:
			
			tmpReturn.type = BOOL;
			
			switch(funct){
			case 0: //GG
				a = read_register(byteStream[pc + 1], &registerFile);
				b = read_register(byteStream[pc + 2], &registerFile);
				
				pcNext += 2;
				break;
			case 1: //GV
				a = read_register(byteStream[pc + 1], &registerFile);
				b = fetch_data(&byteStream[pc + 2]);
				
				pcNext += 1 + DATA_OBJECT_SIZE;
				break;
			case 2: //VG
				a = fetch_data(&byteStream[pc + 1]);
				b = read_register(byteStream[pc + DATA_OBJECT_SIZE + 1], &registerFile);
				
				pcNext += 1 + DATA_OBJECT_SIZE;
				break;
			}

			
			switch(a->type){
			case INTEGER:
				tmpReturn.data.b = (a->data.n) < (b->data.n);
				break;
			case RATIONAL:
				tmpReturn.data.b = (a->data.d) < (b->data.d);
				break;
			case BOOL:
				tmpReturn.data.b = (a->data.b) < (b->data.b);
				break;
			default:
				tmpReturn.data.b = 0;
				break;
			}
			
			r = &tmpReturn;
			
			break;
//MALLOC
		case MALLOC:
			a = read_register(byteStream[pc + 1], &registerFile);
			
			clear_data(&tmpReturn);
			tmpReturn.type = POINTER;
			
			switch(funct){
			case 0:
				tmpReturn.data.p = malloc(DATA_OBJECT_SIZE * a->data.n);
				pcNext += 1;
				break;
			case 1:
				//b = access_register(byteStream[pc + 2], &registerFile);
				tmpReturn.data.p = malloc(a->data.n);
				/*
				switch(b->type){
				case INTEGER:
					tmpReturn.data.p = malloc(sizeof(IntegerType) * a->data.n);
					break;
				case RATIONAL:
					tmpReturn.data.p = malloc(sizeof(RationalType) * a->data.n);
					break;
				case NIL:
					tmpReturn.data.p = 0x00;
					break;
				case BOOL:
					tmpReturn.data.p = malloc(sizeof(Byte) * a->data.n);
					break;
				case STRING:
					tmpReturn.data.p = malloc(sizeof(char*) * a->data.n);
					break;
				case POINTER:
					tmpReturn.data.p = malloc(sizeof(void*) * a->data.n);
					break;
				}
				*/
				pcNext += 1;
				break;
			}
			
			if (log) printf("Allocated memory at %p\n", tmpReturn.data.p);
			
			r = &tmpReturn;
			
			break;
//MFREE			
		case MFREE:
			a = read_register(byteStream[pc + 1], &registerFile);
			
			switch(funct){
			case 0: 
				free(a->data.p);
				break;
			case 1: // subfunction to be used for reference-counting garbage collection
				// check aux bytes
				if (1){
					free (a->data.p);
				}
				break;
			}
			
			pcNext += 1;
			
			pc = pcNext;
			continue;
//MLOAD
		case MLOAD:
			a = read_register(byteStream[pc + 1], &registerFile); // registerP
			
			pcNext += 1;
			
			
			offset = 0;
			finalAddress = 0x00;
			
			switch(funct){
			case 1: // load offset Data_Object to r			
				offset = read_register(byteStream[pc + 2], &registerFile)->data.n;
				
				pcNext += 1;
				
				// no break, fall into next case
				case 0:  // load Data_Object to r
			
				finalAddress = ((Data_Object*) a->data.p) + offset; // "a->data.p + b->data.n" if fallen into
				
				r = (Data_Object*) finalAddress;
				///pcNext += 2; // "pcNext = pc + 3" if fallen into, "pcNext = pc + 2" otherwise
				break;
				
			case 3: // pure data load with offset
				
				offset = read_register(byteStream[pc + 3], &registerFile)->data.n;
				
				pcNext += 1;
				//no break, fall into next case
				case 2: // pure data load
				clear_data(&tmpReturn);
				
				b = read_register(byteStream[pc + 2], &registerFile); //registerRef
				tmpReturn.type = b->type;
				
				pcNext += 1;
				
				if (log){
					printf("original pointer = %p\n", a->data.p);
					printf("offset = %ld\n", offset);
				}
				
				finalAddress = void_ptr_add(a->data.p, offset); // literal offset values are useful for, say, object member loading
				
				if (log) printf("finalAddress = %p\n", finalAddress);
				
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
				case FUNCTION:
					tmpReturn.data.f = *( (PCType*) finalAddress );
				}
				
				r = &tmpReturn;
				///pcNext += 3; // "pcNext = pc + 4" if fallen into, "pcNext = pc + 3" otherwise
				
				break;
			}
			break;
//MOD
		case MOD:
			break;
//MOVE
		case MOVE:
			
			switch(funct){
			case 0: // move one register's value to another
				r = read_register(byteStream[pc + 1], &registerFile); // registerO
				pcNext += 1;
				break;
			case 1: // load a literal value to a register
				tmpReturn = read_bytes(&byteStream[pc + 1]);
				r = &tmpReturn;
				pcNext += DATA_OBJECT_SIZE;
				break;
			case 2: // load a string literal to a register
				// read next two bytes as high and low of the constant's id
				{
				unsigned int stringID = byteStream[pc + 1];
				stringID = stringID << 8;
				stringID += byteStream[pc + 2];
				
				tmpReturn.type = STRING;
				tmpReturn.data.s = metaData.stringHeads[stringID];
				tmpReturn.aux[0] = metaData.stringLens[stringID] >> 8;
				tmpReturn.aux[1] = metaData.stringLens[stringID] & 0xFF;
				}
				r = &tmpReturn;
				
				pcNext += 2;
				break;
			case 3: //load an address literal into a register as a function
				tmpReturn.type = FUNCTION;
				tmpReturn.data.f = read_address_literal(&byteStream[pc + 1]);
				
				r = &tmpReturn;
				
				pcNext += sizeof(PCType);
				
			}
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
			
			a = read_register(byteStream[pc + 1], &registerFile); // registerP
			b = read_register(byteStream[pc + 2], &registerFile); // registerV
			
			pcNext += 2;
		
			switch(funct){
			// DATA_OBJECT allocation
			case 1:
				offset = read_register(byteStream[pc + 3], &registerFile)->data.n;
				
				pcNext += 1;
				
				
				
				// no break, fall into next case
				case 0:
				
				if (log){
					printf("original pointer = %p\n", a->data.p);
					printf("offset = %ld\n", offset);
				}
				
				offset *= DATA_OBJECT_SIZE;
				
				finalAddress = void_ptr_add(a->data.p, offset);
				
				if (log) printf("finalAddress = %p\n", finalAddress);
				
				// write
				*((Data_Object*) finalAddress) = *b;
				
				break;
			// Direct allocation
			case 3:
				offset = read_register(byteStream[pc + 3], &registerFile)->data.n;
				
				pcNext += 1;
				
				// no break, fall into next case
				case 2:
				
				if (log){
					printf("original pointer = %p\n", a->data.p);
					printf("offset = %ld\n", offset);
				}
				
				finalAddress = void_ptr_add(a->data.p, offset); /// offset should be block (size of data type) offset or direct offset?
				
				if (log) printf("finalAddress = %p\n", finalAddress);
				
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
				case FUNCTION:
					*((PCType*) finalAddress) = b->data.f;
				}
				
				break;
			}
			
			pc = pcNext;
			
			continue;
//NOT
		case NOT:
			break;
//OR
		case OR:
			break;
//POPFRAME
		case POPFRAME:
			--(registerFile.depth);
			
			if (log) printf("popped to register file depth %d\n", registerFile.depth);
			
			pc = pcNext;
			continue;
//PUSHFRAME
		case PUSHFRAME:
			++(registerFile.depth);
			
			if (log) printf("pushed to register file depth %d\n", registerFile.depth);
			
			pc = pcNext;
			continue;
//POW
		case POW:
			break;
//PRINT
		case PRINT:
			if (log) printf("PRINTING\n");
			switch(funct){
			case 0: // print register
				a = read_register(byteStream[pc + 1], &registerFile);
				pcNext += 1;
				break;
			case 1: // print constant object
				a = fetch_data(&byteStream[pc + 1]);
				pcNext += DATA_OBJECT_SIZE;
				break;
			case 2: // only printing a newline character, no arguments
				printf("\n");
				
				pc = pcNext;
				continue;
			}
			
			if (log) printf("\tactual type: %d\n", a->type);
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
				break;
			case FUNCTION:
				printf("FUNCTION: %ld", a->data.f);
				break;
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
		
		//return result to output register
		write_register(byteStream[pcNext], &registerFile, r);
		// increment pc past return register
		pc = pcNext + 1;
	}
	
	if (instructionNo >= INSTRUCTION_LIMIT){
		printf("Stopped at instruction limit\n");
	}
}

//==========================================================
// BYTESTREAM MANIPULATION
//==========================================================


const Data_Object* fetch_data(const Byte* rawBytes){
	return (Data_Object*) rawBytes;
}


Data_Object read_bytes(const Byte* rawBytes){
	union{Data_Object asObject; Byte asBytes[sizeof(Data_Object)];} castUnion;
	for (unsigned int i = 0; i < sizeof(Data_Object); i++){
		castUnion.asBytes[i] = rawBytes[i];
	}
	
	return castUnion.asObject;
}

Byte read_bool_direct(const Byte* rawBytes){
	// DATA_OBJECT: <<Bddddddd><tttt><____>>
	return rawBytes[0];
}

IntegerType read_integer_direct(const Byte* rawBytes){
	union {signed long asLong; Byte asBytes[8];} castUnion;
	
	for (unsigned int i = 0; i < 8; ++i){
		castUnion.asBytes[i] = rawBytes[i];
	}
	
	return castUnion.asLong;
}

RationalType read_double_direct(const Byte* rawBytes){
	union {RationalType asDouble; Byte asBytes[8];} castUnion;
	
	for (unsigned char i = 0; i < 8; ++i){
		castUnion.asBytes[i] = rawBytes[i];
	}
	
	return castUnion.asDouble;
}

PCType read_address_literal(const Byte* rawBytes){
	union{PCType asLong; Byte asBytes[sizeof(PCType)];} castUnion;
	for (unsigned char i = 0; i < sizeof(PCType); i++){
		castUnion.asBytes[i] = rawBytes[i];
	}
	
	return castUnion.asLong;
}

//==========================================================
// DATA_OBJECT MANIPULATION
//==========================================================

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
	
	rFile->globalRegisters[0] = zeroObject;
	rFile->pcTop = 0;
}
/*
Data_Object* access_register(Byte reg, Register_File* rf){
	switch(reg & 0xE0){ // get first 3 bits
	case 0x80: // args pass
		return &rf->localRegisters[reg & 0x1F][rf->depth + 1];
		break;
	case 0xA0: // returns pass
		return &rf->localRegisters[reg & 0x1F][rf->depth - 1];
		break;
		
	case 0xC0: // globals
	case 0xE0: // special purpose
		return &rf->globalRegisters[reg & 0x3F];
		break;
		
	default: // local registers
		return &rf->localRegisters[reg][rf->depth];
		break;
		
	}
}
*/

const Data_Object* read_register(Byte reg, Register_File* rf){
	switch(reg & 0xE0){ // get first 3 bits
	case 0x40: // args read
		return &rf->localRegisters[reg & 0x7F][rf->depth];
		break;
	case 0x60: // returns read
		return &rf->localRegisters[reg & 0x7F][rf->depth];
		break;
		
	case 0x80: // globals
	case 0xA0: // special purpose
		return &rf->globalRegisters[reg & 0x1F];
		break;
		
	case 0x00: // local registers
	case 0x20:
		return &rf->localRegisters[reg & 0x7F][rf->depth];
		break;
		
	default:
		printf("attempted read from nonexistant register\n");
		return &rf->globalRegisters[32]; // $zero
		break;
		
	}
}

void write_register(Byte reg, Register_File* rf, const Data_Object* data){
	switch(reg & 0xE0){ // get first 3 bits
	case 0x40: // args pass
		rf->localRegisters[reg & 0x7F][rf->depth + 1] = *data;
		break;
	case 0x60: // returns pass
		rf->localRegisters[reg & 0x7F][rf->depth - 1] = *data;
		break;
		
	case 0x80: // globals
	case 0xA0: // special purpose
		rf->globalRegisters[reg & 0x1F] = *data;
		break;
		
	case 0x00: // local registers
	case 0x20:
		rf->localRegisters[reg & 0x7F][rf->depth] = *data;
		break;
		
	default: // 0xC0, 0xE0 unused
		printf("attempted write to nonexistant register\n");
		break;
		
	}
}

void write_default_output(const Data_Object* object, Register_File* rf){
	//*access_register(rf->defaultOutput, rf) = *object;
}

void push_pc(PCType* pc_entry, Register_File* rf){
	if (rf->pcTop < PC_STACK_SIZE - 1){
		///printf("enough space\n"); ///DEBUG
		++(rf->pcTop);
		rf->pcStack[rf->pcTop] = *pc_entry;
	}else{
		printf("ERROR: pc stack overflow!\n");
	}
}

PCType pop_pc(Register_File* rf){
	if (rf->pcTop > 0){
		--(rf->pcTop);
		return rf->pcStack[rf->pcTop + 1];
	}else{
		printf("ERROR: pc stack underflow!\n");
		return -1;
	}
}

//==========================================================
// METADATA FUNCTIONS
//==========================================================

PCType read_metadata(const Byte* byteStream, const PCType* length, Meta_Data* metaData){
	PCType pc = 0;
	
	unsigned int stringLength;
	char* stringHead;
	unsigned int nstrings;
	
	while (pc < *length && byteStream[pc] != META_END){
		printf("meta byte <%x>\n", byteStream[pc]); /// DEBUG
		switch(byteStream[pc]){
		case META_BEGIN:
			printf("\tMETA_BEGIN found\n"); /// DEBUG
			pc += 1;
			break;
		case META_VERSION:
			// read version from next 2 bytes
			// version = <versionMain>:<versionSub> e.g. 1:24
			metaData->versionMain = byteStream[pc + 1];
			metaData->versionSub = byteStream[pc + 2];
			
			// move pc to next metadata
			pc += 3;
			break;
		case META_STRING:
			/// printf("\tMETA_STRING (%x) at %ld\n", byteStream[pc], pc); /// DEBUG
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
			break;
		case META_NSTRING:
			// read the number of string constants
			nstrings = byteStream[pc + 1];
			nstrings = nstrings << 8;
			nstrings += byteStream[pc + 2];
			
			// allocate memory for strings
			metaData->nstrings = nstrings;
			metaData->stringHeads = (char**) malloc(sizeof(char*) * nstrings);
			metaData->stringLens = (unsigned int*) malloc(sizeof(unsigned int) * nstrings);
			
			// move pc to next metadata
			pc += 3;
			break;
		case META_END:
			return pc + 1;
		default:
			pc += 1;
		}
	}
	
	return pc + 1;
}