#include "VM.h"
#include "Assembler.h"

Assembler::Assembler(std::istream& instrm): inStream(instrm), byteVec() {
	nextFree = 0;
	nextFreePersistent = NUM_REGISTERS - NUM_PERSISTENT_REGISTERS;
	
}

// Assemble the given input into bytecode. Outputs to the given output stream.
// Returns true on assembly success, false on failure/error
bool Assembler::assemble(){
	
			if (log) std::cout << "beginning assembly" << std::endl; ///DEBUG
	
	error = AERROR_NONE;
	unsigned int lineCount = 1;
	
	// write metadata
	write_byte(META_BEGIN);
	write_byte(META_VERSION);
	write_byte(ASSEMBLER_VERSION_MAIN);
	write_byte(ASSEMBLER_VERSION_SUB);
	write_byte(META_END);
	
	
	std::string line;
	
	while (std::getline(inStream, line)){
		// remove '\r' from line
		if (*line.rbegin() == '\r'){
			line.erase(line.size() - 1, 1);
		}
		
		if (log) std::cout << "Line " << line << std::endl;
		assemble_line(line);
		
		switch(error){
		case AERROR_NONE:
			if (log) std::cout << "line assemby success" << std::endl;
			break;
		case AERROR_WRONGARGS:
			std::cout << "error [" << lineCount << "]: instruction has wrong args" << std::endl;
			return false;
		case AERROR_NOPCODE:
			std::cout << "error [" << lineCount << "]: opcode not found" << std::endl;
			return false;
		case AERROR_REGLIMIT:
			std::cout << "error [" << lineCount << "]: not enough registers to allocate" << std::endl;
			return false;
		case AERROR_BADRET:
			std::cout << "error [" << lineCount << "]: bad return type" << std::endl;
			return false;
		case AERROR_ARGTYPES:
			std::cout << "error [" << lineCount << "]: incorrect argument types" << std::endl;
			return false;
		case AERROR_REGBOUNDS:
			std::cout << "error [" << lineCount << "]: register number out of bounds" << std::endl;
		}
		
		error = AERROR_NONE;
		
		++lineCount;
	}
	
	// go back and fill in labels
	assemble_labels();
	
	if (log) std::cout << "file assembly successful" << std::endl;
	
	return true;
}

//==========================================================
// ASSEMBLER STATE
//==========================================================

// get_register: Looks up the register mapped to a given 
// identifier. If none exists, creates a new mapping. Returns
// the register number.
Byte Assembler::get_register(const std::string& identifier, bool isPersistent){
	
	if (isPersistent){
		if (nextFreePersistent == NUM_REGISTERS - 1){
			 if (log) std::cerr << "error: no more registers to allocate" << std::endl;
			error = AERROR_REGLIMIT;
			return 0;
		}
	}else{
		// return 0 if no more registers to allocate
		if (nextFree == NUM_REGISTERS - NUM_PERSISTENT_REGISTERS){
			 if (log) std::cerr << "error: no more registers to allocate" << std::endl;
			error = AERROR_REGLIMIT;
			return 0;
		}
	}
	
	std::pair<RegisterMap::iterator, bool> search;
	
	if (isPersistent){
		std::pair<std::string, Byte> entry(identifier, nextFreePersistent);
		search = persistent.insert(entry);
	}else{
		std::pair<std::string, Byte> entry(identifier, nextFree);
		search = mapping.insert(entry);
	}
	
	if (search.second){
		// entry inserted
		
		// increment nextFree and return the allocated register
		
		if (log) std::cout << "\tallocated new register " << identifier << std::endl; /// DEBUG
		
		if (isPersistent){
			++nextFreePersistent;
			return nextFreePersistent - 1;
		}else{
			++nextFree;
			return nextFree - 1;
		}
	}else{
		// entry not inserted
		
		// return the previously allocated register
		if (log) std::cout << "\tidentified old register " << identifier << std::endl; /// DEBUG
		return search.first->second;
	}
}

// define_label: Maps a label to a location in the program
void Assembler::define_label(const std::string& label){
	LabelMap::iterator search = labelMap.find(label);
	
	if (search == labelMap.end()){
		// create a new label pairing between label location and references
		std::pair<PCType, std::list<PCType> > locAndRefs;
		locAndRefs.first = byteVec.size();
		
		labelMap[label] = locAndRefs;
		
		if (log) std::cout << "\tDEFINED label " << label << " to " << byteVec.size() << std::endl; ///DEBUG
	}else{
		// set this label's location to the next byte (next instruction) which will be written
		search->second // std::pair<PCType, std::list<PCType>>
			.first // PCType
				= byteVec.size();
				
		if (log) std::cout << "\tREDEFINED label " << label << " to " << byteVec.size() << std::endl; /// DEBUG
	}
}

// add_label_ref: Stores an entry for a location that a
// label is used at.
void Assembler::add_label_ref(const std::string& label){
	LabelMap::iterator search = labelMap.find(label);
	
	if (search == labelMap.end()){
		// create a new label pairing between label location and references
		std::pair<PCType, std::list<PCType> > locAndRefs;
		locAndRefs.second.push_back(byteVec.size());
		
		labelMap[label] = locAndRefs;
	}else{
		// add a reference to this label at the next-written byte
		search->second // std::pair<PCType, std::list<PCType>>
			.second // std::list<PCType>
				.push_back(byteVec.size());
				
		
	}
	
	if (log) std::cout << "\tREFERENCED " << label << " at " << byteVec.size() << std::endl; /// DEBUG
}

// push_frame: Creates a new set of dynamic mappings which
// the VM will use in a new frame context
void Assembler::push_frame(){
	mappingStack.push(mapping);
	mapping.clear();
	freeStack.push(nextFree);
	nextFree = 0;
}

// pop_frame: Pops the last set of dynamic mappings which
// the VM will use when returning to a previous frame context
void Assembler::pop_frame(){
	mapping = mappingStack.top();
	mappingStack.pop();
	
	nextFree = freeStack.top();
	freeStack.pop();
}

//==========================================================
// OUTPUT/WRITING
//==========================================================

// write_byte: Writes a byte to output
void Assembler::write_byte(Byte b){
	if (log) std::cout << " {writing byte<" << std::hex << (unsigned int) b << std::dec << ">} ";
	byteVec.push_back(b);
}

// assemble_instruction: Processes a single assembly 
// instruction (one line of file) into bytecode and writes
// to the output
void Assembler::assemble_instruction(const std::string& instruction){
	unsigned int index = 0;
	
	std::cout << std::hex;
	
			if (log) std::cout << "instruction: " << instruction << std::endl; ///DEBUG 
	
	// check if line is a label
	if (instruction[0] == ':'){
		
				if (log) std::cout << "line is label" << std::endl; ///DEBUG
		
		std::string label = "";
		for (unsigned int i = 1; i < instruction.length(); i++){
			if (instruction[i] == ';'){
				return;
			}else if(instruction[i] == ':' && label.length() > 0){ // allows for multiple labels per line, e.g. "LOOP1:LOOP2:RESTART"
				define_label(label);
				label = "";
			}else{
				label += instruction[i];
			}
		}
		if (label.length() > 0){
			define_label(label);
		}
		
		return;
	}
	
	char subfunction = ' ';
	Byte opcode = read_opcode(instruction, index, subfunction);
	if (opcode == 0x16 << 2){
			if (log) std::cout << "frame popped" << std::endl; /// DEBUG
		pop_frame();
	}else if (opcode == 0x17 << 2){
			if (log) std::cout << "frame pushed" << std::endl; /// DEBUG
		push_frame();
	}
	
			if (log) std::cout << "opcode: " << (int) opcode << std::endl; ///DEBUG 
	
	std::vector<Argument> args = read_args(instruction, index);
	
			if (log) std::cout << "checkpoint: args read" << std::endl; ///DEBUG 
	
	Byte funct = get_funct(opcode, args, subfunction);
	
			if (log) std::cout << "funct: " << (int) funct << std::endl; ///DEBUG 
	
	Byte returnReg = 0;
	Byte returnBit = read_returns(instruction, index, returnReg);
	
			if (log) std::cout << "return bit: " << (int) returnBit << std::endl; ///DEBUG 
	
	// compile full opcode and output
	// opcode format: <rooo-ooff>
	Byte fullOpcode = returnBit | opcode | funct;
	
			if (log) std::cout << "full opcode: " << (int) fullOpcode << std::endl; ///DEBUG 
	
			if (log) std::cout << "\n\tInstruction {" << instruction << "} written at " << byteVec.size() << std::endl << std::endl;
	write_byte(fullOpcode);
	
	// compile arguments into bytes and output
	for (unsigned int i = 0; i < args.size(); i++){
		assemble_argument(args[i]);
	}
	
	// add return argument to tail of instruction
	if (returnBit){
		write_byte(returnReg);
	}
}

// assemble_argument: Writes to output the bytecode for a
// single assembly argument (e.g. as a literal Data_Object)
// or a register number
void Assembler::assemble_argument(const Argument& arg){
	if (arg.type == A_VALUE){
		Data_Object_Cast_Union castUnion;
		castUnion.data = read_literal(arg.arg);
		
		if (log) std::cout << "created Data_Object {type=" << castUnion.data.type << ", data=" << castUnion.data.data.n << "} for literal " << arg.arg << std::endl; ///DEBUG
		
		for (unsigned int i = 0; i < DATA_OBJECT_SIZE; i++){
			write_byte(castUnion.bytes[i]);
		}
		
	}else if (arg.type == A_REGISTER){
		Byte reg = get_register(arg.arg);
		
		if (error != AERROR_REGLIMIT){
			if (log) std::cout << "\tsuccessfully mapped " << arg.arg << " to register " << (int) reg << std::endl; /// DEBUG
			write_byte(reg);
		}
	}else if(arg.type == A_REGISTER_P){
		Byte reg = get_register(arg.arg, true);
		if (error != AERROR_REGLIMIT){
			if (log) std::cout << "\tsuccessfully mapped " << arg.arg << " to persistent register " << (int) reg << std::endl; /// DEBUG
			write_byte(reg);
		}
	}else if (arg.type == A_LABEL){
		// store the location that this reference is used
		// that is, the location of where the first byte will be written to in loop
		add_label_ref(arg.arg);
		// write blank bytes where address literal will go
		for (unsigned int i = 0; i < sizeof(PCType); i++){
			write_byte(0x00);
		}
	}
}

// assemble_labels: Goes to the location where each label is 
// referenced and fills in the actual bytecode address for that
// label
void Assembler::assemble_labels(){
	// will cast label's address to bytes using a cast union
	union {PCType asAddress; Byte asBytes[sizeof(PCType)];} castUnion;
	for (LabelMap::const_iterator currentLabel = labelMap.begin(); currentLabel != labelMap.end(); ++currentLabel){
		for (std::list<PCType>::const_iterator currentReference = currentLabel->second.second.begin(); currentReference != currentLabel->second.second.end(); ++currentReference){
			castUnion.asAddress = currentLabel->second.first;
			
			// write each byte of address to this reference location
			for (unsigned int i = 0; i < sizeof(PCType); i++){
				byteVec[*currentReference + i] = castUnion.asBytes[i];
			}
			
					if (log) std::cout << "wrote a reference to " << currentLabel->first << '(' << currentLabel->second.first << ") at " << *currentReference << std::endl; ///DEBUG
		}
	}
}

void Assembler::recordOpcodePosition(){
	lastOpcodePosition = (PCType) byteVec.size();
}
	
void Assembler::overwriteReturnBit(bool returnBit){
	if (lastOpcodePosition >= (PCType) byteVec.size()) return;
	
	if (returnBit){
		byteVec[lastOpcodePosition] |= 0x80; 
	}else{
		byteVec[lastOpcodePosition] &= 0x7F; 
	}
}

//==========================================================
// INPUT/READING
//==========================================================

bool Assembler::assemble_line(const std::string& line){
	int lineEnd = line.find(';');
	if (lineEnd == std::string::npos) lineEnd = line.size();

	// move tokenBegin to beginning of opcode
	int tokenBegin = 0;
	int tokenLength = 0;
	
	while (tokenBegin < lineEnd && line[tokenBegin] == ' '){
		tokenBegin++;
	}
	
	// check if a label is being defined
	if (line[tokenBegin] == ':'){
		// add label
		while (tokenBegin + tokenLength < lineEnd && line[tokenBegin + tokenLength] != ' ') tokenLength++;
		define_label(line.substr(tokenBegin + 1, tokenLength - 1));
		
		if (log) std::cout << "Label read: " << line.substr(tokenBegin + 1, tokenLength - 1) << std::endl;
		
		return true;
	}
	
	// get opcode
	while (tokenBegin + tokenLength < lineEnd 
		&& !(line[tokenBegin + tokenLength] == '(' || line[tokenBegin + tokenLength] == ' ')){
			tokenLength++;
	}
	
	Byte opcode = get_opcode_hex(line.substr(tokenBegin, tokenLength));
	if (log) std::cout << "Instruction read: " << line.substr(tokenBegin, tokenLength) << " as opcode " << std::hex << ((unsigned int) opcode >> 2) << std::dec << std::endl;
	
	// get funct
	Byte funct;
	if (line[tokenBegin + tokenLength] == '('){
		++tokenLength;
		while (tokenBegin + tokenLength < lineEnd && line[tokenBegin + tokenLength] == ' ') tokenLength++;
		funct = line[tokenBegin + tokenLength] - 48;
		while (tokenBegin + tokenLength < lineEnd && line[tokenBegin + tokenLength] != ')') tokenLength++;
		tokenLength++;
	}else{
		funct = 0x00;
	}
	
	if (log) std::cout << "Funct read: " << (unsigned int) funct << std::endl;
	
	// get full opcode
	Byte fullOpcode = (opcode) | funct;
	
	if (log) std::cout << "Full opcode: " << std::hex << (unsigned int) fullOpcode << std::dec << std::endl;
	
	// store opcode location so the returnbit can be overwritten
	recordOpcodePosition();
	// write opcode
	write_byte(fullOpcode);
	
	// get to beginning of arguments
	
	while (tokenBegin + tokenLength < lineEnd && line[tokenBegin + tokenLength] == ' ') tokenLength++;
	
	tokenBegin = tokenLength;
	tokenLength = 0;
	
	// for each argument:
	while (tokenBegin < lineEnd && line[tokenBegin] != '>'){
		
		Byte convertedRegister;
		
		// get type of argument
		switch(line[tokenBegin]){
		case '$': // local register
			while (tokenBegin + tokenLength < lineEnd && line[tokenBegin + tokenLength] != ' ') tokenLength++;
			
			convertedRegister = convert_register(line.substr(tokenBegin + 1, tokenLength - 1));
			if (log) std::cout << "Register read: " << line.substr(tokenBegin + 1, tokenLength - 1) << " and converted to " << (unsigned int) convertedRegister << std::endl;
			
			write_byte(convertedRegister);
			break;
		case ':': // label
			while (tokenBegin + tokenLength < lineEnd && line[tokenBegin + tokenLength] != ' ') tokenLength++;
			
			// mark a label reference here and fill in empty bits
			add_label_ref(line.substr(tokenBegin + 1, tokenLength - 1));
			for (unsigned int i = 0; i < sizeof(PCType); i++){
				write_byte(0x00);
			}
			
			if (log) std::cout << "Label argument read: " << line.substr(tokenBegin + 1, tokenLength - 1) << std::endl;
			
			break;
		case '"': // string literal
		
			// scan to next double quotes
			while (tokenBegin + tokenLength < lineEnd && line[tokenBegin + tokenLength] != '"') tokenLength++;
			
			break;
		default: // assume it is a literal
			while (tokenBegin + tokenLength < lineEnd && line[tokenBegin + tokenLength] != ' ') tokenLength++;
			
			if (log) std::cout << "Read literal: " << line.substr(tokenBegin, tokenLength) << std::endl;
			
			// convert literal to a Data_Object stored in a cast union object
			union {Data_Object asObject; Byte asBytes[sizeof(Data_Object)];} castUnion;
			castUnion.asObject = read_literal(line.substr(tokenBegin, tokenLength));
			
			if (log) std::cout << "Data_Object in cast union as: " << castUnion.asObject.data.n << std::endl;
			
			// write Data_Object's bytes 
			for (unsigned int i = 0; i < sizeof(Data_Object); i++){
				write_byte(castUnion.asBytes[i]);
				if (log) std::cout << "\twriting Data_Object byte " << std::hex << (unsigned int) castUnion.asBytes[i] << std::dec << std::endl;
			}
			
			break;
		}
		
		// go to next argument
		while (tokenBegin + tokenLength < lineEnd && line[tokenBegin + tokenLength] == ' ') tokenBegin++;
		
		tokenBegin = tokenBegin + tokenLength;
		tokenLength = 0;
	}
	
	if (log) std::cout << "tokenBegin < lineEnd: " << (tokenBegin < lineEnd) << std::endl;
	
	// read return register
	tokenBegin++;
	while (tokenBegin + tokenLength < lineEnd && line[tokenBegin + tokenLength] == ' ') tokenBegin++;
	
	tokenBegin = tokenBegin + tokenLength;
	tokenLength = 0;
	
	if (tokenBegin + tokenLength < lineEnd && line[tokenBegin] != '$'){ // incorrect returns
		std::cout << "HERE" << std::endl;
		error = AERROR_BADRET;
		return false;
	}
	
	while (tokenBegin + tokenLength < lineEnd && line[tokenBegin + tokenLength] != ' ') tokenLength++;
	
	if (tokenBegin < lineEnd && log) std::cout << "Return register read: " << line.substr(tokenBegin + 1, tokenLength - 1) << std::endl;
	
	if (tokenBegin < lineEnd){
		write_byte(convert_register(line.substr(tokenBegin + 1, tokenLength - 1)));
		overwriteReturnBit(true);
	}else{
		overwriteReturnBit(false);
	}
	
	return true;
}

Byte Assembler::convert_register(const std::string& registerName){
	Byte registerID;
	
	// find beginning of parentheses
	int parenthesesBegin = registerName.find('(');
	// if no parentheses found, register is a local register
	if (parenthesesBegin == std::string::npos){
		registerID = (Byte) string_to_int(registerName);
		if (registerID >= 64){
			error = AERROR_REGBOUNDS;
			return 0xA0;
		}else{
			return registerID;
		}
	}
	
	int parenthesesEnd = registerName.find(')');
	if (parenthesesEnd == std::string::npos) return 0xE0; // $zero by default in case of error
	
	// the raw register ID found in the parentheses
	std::string registerID_raw = registerName.substr(parenthesesBegin + 1, parenthesesEnd - parenthesesBegin - 1);
	registerID = (Byte) string_to_int(registerID_raw);
	if (registerID >= 32){
		error = AERROR_REGBOUNDS;
		return 0xA0;
	}
	
	std::string registerType = registerName.substr(0, parenthesesBegin);
	
	if (log) std::cout << "Register type read as: " << registerType << " for raw register " << (registerID_raw) << std::endl;
	
	if (registerType == "ARG")	{
		return 0x40 | registerID;
		
	}else if (registerType == "RETURN") {
		return 0x60 | registerID;
		
	}else if (registerType == "GLOBAL") {
		return 0x80 | registerID;
		
	}else if (registerType == "SFR") {
		return 0xA0 | registerID;
		
	}else{
		return 0xA0; // $zero by default
	}
}

// read_opcode: Read an assembly opcode from an instruction.
// Returns the byte opcode without correct returnBit or funct bits
Byte Assembler::read_opcode(const std::string& instruction, unsigned int& index, char& subfunction){
	std::string opcodeRaw = "";
	
	for ( ; index < instruction.length() && instruction[index] != ' '; ++index){
		if (instruction[index] == '_'){
			++index;
			subfunction = instruction[index];
			++index;
			break;
		}
		opcodeRaw += instruction[index];
	}
	
	Byte opcode = get_opcode_hex(opcodeRaw);
	
	if (opcode == 0xFF){
		error = AERROR_NOPCODE;
	}
	return opcode;
}

// read_args: Read an instruction's assembly arguments, tagging
// each one with a type. Returns a vector of tagged arguments
std::vector<Assembler::Argument> Assembler::read_args(const std::string& instruction, unsigned int& index){
	std::vector<Argument> args;
	
	while (index < instruction.length() && instruction[index] != '>'){
		
		if (instruction[index] != ' '){
			args.push_back(read_argument(instruction, index));
		}
		
		++index;
	}
	
	return args;
}

// read_argument: Read an assembly argument and give it a type.
// Returns the argument tagge with a type
Assembler::Argument Assembler::read_argument(const std::string& instruction, unsigned int& index){
	Argument argument;
	
	// increment index until reaching beginning of argument
	for (; index < instruction.length() && instruction[index] == ' '; ++index);
	
	switch(instruction[index]){
	case '$':
		argument.type = A_REGISTER;
		++index;
		break;
	case '#':
		argument.type = A_REGISTER_P;
		++index;
		break;
	case '@':
		argument.type = A_ADDRESS;
		++index;
		break;
	case ':':
		argument.type = A_LABEL;
		++index;
		break;
	case '"':
		argument.type = A_STRING;
		++index;
		break;
	default:
		argument.type = A_VALUE;
	}
	
	while(index < instruction.length() 
			&& ((argument.type != A_STRING && instruction[index] != ' ') || (argument.type == A_STRING && instruction[index] != '"'))) // loops until reaching a space or, if this is a string, until reaching a "
	{
		argument.arg += instruction[index];
		++index;
	}
	//--index; // pull index away from the space after the identifier
	
	if (log) std::cout << "argument " << argument.arg << " of type " << argument.type << std::endl; /// DEBUG
	
	return argument;
}

// read_returns: Reads the last part of an assembly instruction
// to determine what the return type is. Returns a byte 
// containing the correct returnBit
Byte Assembler::read_returns(const std::string& instruction, unsigned int& index, Byte& returnReg){
	// iterate index to the beginning of the argument
	for (; index < instruction.length() && (instruction[index] == ' ' || instruction[index] == '>'); ++index){
		// return on finding a comment
		
		if (instruction[index] == ';'){
			return 0x00;
		}
	}
	
	// check for a no-argument case
	if (index >= instruction.length()){
		return 0x00;
	}else{
	}
	
	Argument returnArg = read_argument(instruction, index);
	if (!(returnArg.type == A_REGISTER || returnArg.type == A_REGISTER_P)){
		// bad return type error
		error = AERROR_BADRET;
	}else{
		returnReg = get_register(returnArg.arg, returnArg.type == A_REGISTER_P);
	}
	
	return 0x80;
}

// read_literal: Reads a literal numerical value from an 
// instruction. Returns a Data_Object wrapping the value.
Data_Object Assembler::read_literal(const std::string& literal){
	
	
	Data_Object object;
	object.type = INTEGER;
	
	if (literal == "NIL"){
		object.type = NIL;
		return object;
	}else if (literal == "TRUE"){
		object.type = BOOL;
		object.data.b = 1;
		return object;
	}
	else if (literal == "FALSE"){
		object.type = BOOL;
		object.data.b = 0;
		return object;
	}
	
	IntegerType tmpInt = 0;
	RationalType tmpRat = 0.0f;
	unsigned int decimal = 0;
	
	for (unsigned int i = 0; i < literal.length(); i++){
		if (literal[i] == '.'){
			object.type = RATIONAL;
			decimal = i;
			continue;
		}
		unsigned int x;
		RationalType y;
		switch(object.type){
		case INTEGER:
			tmpInt *= 10;
			tmpInt += (IntegerType) char_to_int(literal[i]);
			break;
		case RATIONAL:
			tmpRat *= 10;
			tmpRat += (RationalType) char_to_int(literal[i]);
			break;
		default:
			break;
		}
	}
	
	if (object.type == RATIONAL){
		RationalType denom = 1;
		for (unsigned int i = 0; i < (literal.length() - decimal); i++, denom *= 10);
		
		tmpRat /= denom;
		tmpRat += (RationalType) tmpInt;
		
		if (log) std::cout << "Rational " << tmpRat << " built from tmpInt " << tmpInt << std::endl; ///DEBUG
		
		object.data.d = tmpRat;
	}else{
		object.data.n = tmpInt;
		
		if (log) std::cout << "converted literal: " << tmpInt << std::endl;
	}
	
	return object;
}

//==========================================================
// UTILITY
//==========================================================

Byte Assembler::get_opcode_hex(const std::string& opcode) const{
		
	if(opcode == "ABS"){
		return 0x00 << 2;
	}
	else if(opcode == "ADD"){
		return 0x01 << 2;
	}
	else if(opcode == "AND"){
		return 0x02 << 2;
	}
	else if(opcode == "BRANCH"){
		return 0x03 << 2;
	}
	else if(opcode == "CALL"){
		return 0x04 << 2;
	}
	else if(opcode == "DIV"){
		return 0x05 << 2;
	}
	else if(opcode == "EQ"){
		return 0x06 << 2;
	}
	else if(opcode == "GT"){
		return 0x07 << 2;
	}
	else if(opcode == "HALT"){
		return 0x08 << 2;
	}
	else if(opcode == "INPUT"){
		return 0x09 << 2;
	}
	else if(opcode == "JUMP"){
		return 0x0A << 2;
	}
	else if(opcode == "LSH"){
		return 0x0B << 2;
	}
	else if(opcode == "LT"){
		return 0x0C << 2;
	}
	else if(opcode == "MALLOC"){
		return 0x0D << 2;
	}
	else if(opcode == "MFREE"){
		return 0x0E << 2;
	}
	else if(opcode == "MLOAD"){
		return 0x0F << 2;
	}
	else if(opcode == "MOD"){
		return 0x10 << 2;
	}
	else if(opcode == "MOVE"){
		return 0x11 << 2;
	}
	else if(opcode == "MSTORE"){
		return 0x12 << 2;
	}
	else if(opcode == "MUL"){
		return 0x13 << 2;
	}
	else if(opcode == "NOT"){
		return 0x14 << 2;
	}
	else if(opcode == "OR"){
		return 0x15 << 2;
	}
	else if(opcode == "POPFRAME"){
		return 0x16 << 2;
	}
	else if(opcode == "PUSHFRAME"){
		return 0x17 << 2;
	}
	else if(opcode == "POW"){
		return 0x18 << 2;
	}
	else if(opcode == "PRINT"){
		return 0x19 << 2;
	}
	else if(opcode == "RETURN"){
		return 0x1A << 2;
	}
	else if(opcode == "RSH"){
		return 0x1B << 2;
	}
	else if(opcode == "SETDO"){
		return 0x1C << 2;
	}
	else if(opcode == "SUB"){
		return 0x1D << 2;
	}
	else if(opcode == "XOR"){
		return 0x1E << 2;
	}else{
		return 0xFF;
	}
}

Byte Assembler::get_funct(Byte opcodeShifted, std::vector<Argument> args, char subfunction){
	
	switch(opcodeShifted >> 2){
	case POPFRAME:
	case PUSHFRAME:
	case HALT:
	case RETURN:
	
		if (args.size() != 0){
			std::cout << "\tA" << std::endl;
			error = AERROR_WRONGARGS;
		}
		
		return 0x00;
		break;
		
	case SETDO:
	case ABS:
	case NOT:
	case INPUT:
		if (args.size() != 1){
			error = AERROR_WRONGARGS;
			if (log) std::cout << "\tB" << std::endl;
		}
		
		return 0x00;
		break;
		
	case ADD:
	case AND:
	case DIV:
	case EQ:
	case GT:
	case LSH:
	case LT:
	case MOD:
	case MUL:
	case OR:
	case POW:
	case RSH:
	case SUB:
	case XOR:
		if (args.size() != 2){
			error = AERROR_WRONGARGS;
			if (log) std::cout << "\tC1" << std::endl;
			return 0x00;
		}
		
		if ((args[0].type == A_REGISTER || args[0].type == A_REGISTER_P) 
				&& (args[1].type == A_REGISTER || args[1].type == A_REGISTER_P)){ // GG
			return 0x00;
		}else if ((args[0].type == A_REGISTER || args[0].type == A_REGISTER_P) 
				&& (args[1].type == A_VALUE)){ // GV
			return 0x01;
		}else if ((args[0].type == A_VALUE) 
				&& (args[1].type == A_REGISTER || args[1].type == A_REGISTER_P)){ // VG
			return 0x02;
		}else{
			if (log) std::cout << "\tC2" << std::endl;
			error = AERROR_WRONGARGS;
			return 0x00;
		}
		break;
	
	case MLOAD:
		if (subfunction == 'D'){
			if (args.size() == 2)
				return 0x02;
			else if (args.size() == 3)
				return 0x03;
			else{
				error = AERROR_WRONGARGS;
				if (log) std::cout << "\tD" << std::endl;
				return 0x00;
			}
		}else{
			if (args.size() == 1)
				return 0x00;
			else if (args.size() == 2)
				return 0x01;
			else{
				if (log) std::cout << "\tE" << std::endl;
				error = AERROR_WRONGARGS;
				return 0x00;
			}
		}
		break;
	case MSTORE:
		if (subfunction == 'D'){
			if (args.size() == 2)
				return 0x02;
			else if (args.size() == 3)
				return 0x03;
			else{
				error = AERROR_WRONGARGS;
				if (log) std::cout << "\tD" << std::endl;
				return 0x00;
			}
		}else{
			if (args.size() == 2)
				return 0x00;
			else if (args.size() == 3)
				return 0x01;
			else{
				if (log) std::cout << "\tE" << std::endl;
				error = AERROR_WRONGARGS;
				return 0x00;
			}
		}
		break;
	
	case BRANCH:
		if (args.size() != 2){
			if (log) std::cout << "\tF" << std::endl;
			error = AERROR_WRONGARGS;
		}
		
		if (subfunction == 'T'){
			return 0x00;
		}else if (subfunction == 'F'){
			return 0x01;
		}else{
			return 0x00;
		}
	case CALL:
		return 0x00;
		break;
	case JUMP:
		if (args.size() != 1){
			if (log) std::cout << "\tG" << std::endl;
			error = AERROR_WRONGARGS;
		}
		
		if (subfunction == 'L'){
			return 0x01;
		}else{
			return 0x00;
		}
	case MOVE:
		if (args.size() != 1){
			if (log) std::cout << "\tH" << std::endl;
			error = AERROR_WRONGARGS;
		}
		if (args[0].type == A_VALUE){
			return 0x01;
		}else if (args[0].type == A_STRING){
			return 0x02;
		}else{
			return 0x00;
		}
		break;
	case MALLOC:
		if (subfunction == 'D'){
			if (args.size() != 1){
				error = AERROR_WRONGARGS;
			}
			return 0x01;
		}else{
			if (args.size() != 1){
				error = AERROR_WRONGARGS;
			}
			return 0x00;
		}
		break;
	case MFREE:
		return 0x00;
		break;
	case PRINT:
		if (subfunction == 'B'){
			return 0x02;
		}else{
			if (args.size() != 1){
				if (log) std::cout << "\tI" << std::endl;
				error = AERROR_WRONGARGS;
			}
			
			if (args[0].type == A_REGISTER || args[0].type == A_REGISTER_P)
				return 0x00;
			else if (args[0].type == A_VALUE)
				return 0x01;
		}
		break;
	default:
		return 0x00;
	}
}

unsigned int Assembler::char_to_int(char c){
	return ((unsigned int) c) - 48;
}

unsigned int Assembler::string_to_int(const std::string& s){
	unsigned int result = 0;
	
	for (unsigned int i = 0;i < s.size(); i++){
		result *= 10;
		result += (unsigned int) s[i] - 48;
	}
	
	return result;
}
