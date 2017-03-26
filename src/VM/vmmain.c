#include "VM.h"

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