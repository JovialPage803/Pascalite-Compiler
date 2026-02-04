//Ryan Dusek - CS 4301 - Stage 1


#include <stage1.h>
#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <ctime>
#include <iomanip>
#include <set>			//for creating sets of strings/keywords
#include <bits/stdc++.h>//for all_of
#include <cctype>		//isspace



//pascallite overall compiler structure stage 0 --> page 6 begins pseudocode


//Constructor and Destructor (open/close files)
Compiler::Compiler(char **argv){
	
	//psuedocode
	//open sourceFile using argv[1]
	//open listingFile using argv[2]
	//open objectFile using argv[3]
	
	sourceFile.open(argv[1]);///.dat
	if(!sourceFile){
		cerr << "Error: Cannot open source file " << argv[1] << endl;
		exit(EXIT_FAILURE);
	}
	
	listingFile.open(argv[2]);//.lst
	if(!listingFile){
		cerr << "Error: Cannot open listing file " << argv[2] << endl;
		exit(EXIT_FAILURE);
	}
	
	objectFile.open(argv[3]);//.asm
	if(!objectFile){
		cerr << "Error: Cannot open object file " << argv[3] << endl;
		exit(EXIT_FAILURE);
	}
	
}//end Constructor

Compiler::~Compiler(){
	
	//close all opened files from the constructor
	sourceFile.close();//.dat
	listingFile.close();//.lst
	objectFile.close();//.asm
	
}//end Destructor





//.lst file
void Compiler::createListingHeader(){
	
	/*string timeOut;
	FILE* pipe = popen("./getTime", "r");
	
	char buffer[32];
	while(fgets(buffer, sizeof(buffer), pipe) != nullptr)
		timeOut += buffer;
	
	string skip = timeOut.substr(timeOut.find(':') + 2);//skip space after last modified:
	string currTime = skip.substr(0, skip.length() - 1);*/
	
	auto now = chrono::system_clock::now();
    time_t now_time = chrono::system_clock::to_time_t(now);

    // Format time as a string
    stringstream ss;
    ss << put_time(localtime(&now_time), "%b %d %Y  %H:%M:%S");
    string currTime = ss.str();
	
	//top line
	listingFile << "STAGE1:  Ryan Dusek & Ryan Duncan       " << currTime << endl << endl;
	
	//Line no. source...
	listingFile << "LINE NO." << setw(30) << "SOURCE STATEMENT" << endl << endl;
	
}//end createListingHeader

void Compiler::parser(){
	
	nextChar();
	
	//get the first token (consume it)
	token = nextToken();
	if(token != "program")
		processError("keyword \"program\" expected");
	
	prog();
	
}//end parser

void Compiler::createListingTrailer(){
	
	listingFile << endl <<  "COMPILATION TERMINATED" << "      ";
	if(errorCount > 1 || errorCount == 0)
		listingFile << errorCount << " ERRORS ENCOUNTERED" << endl;
	else
		listingFile << errorCount << " ERROR ENCOUNTERED" << endl;
	
}//end createListingTrailer

void Compiler::processError(string err){
	
	errorCount++;
	
	int displayLine = (lineNo < 1 ? 1 : lineNo);
	
	listingFile << endl << "Error: Line " << displayLine << ": " << err << endl;
	
	cerr << "Error: " << err << endl;
	
	createListingTrailer();
	listingFile.flush();
	
	exit(EXIT_FAILURE);
	
}//end processError





//Methods implementing the grammar productions
void Compiler::prog(){           //stage 0, production 1
	
	if(token != "program")
		processError("keyword \"program\" expected");

	progStmt();

	if(token == "const")
		consts();

	if(token == "var")
		vars();

	if(token != "begin")
		processError("keyword \"begin\" expected");

	beginEndStmt();

	if(token[0] != END_OF_FILE)
		processError("no text may follow \"end\"");
	
}//end prog

void Compiler::progStmt(){       //stage 0, production 2
	
	string name;

	if(token != "program")
		processError("keyword \"program\" expected");

	token = nextToken();  
	name = token;

	if(!isNonKeyId(name))
		processError("program name expected");

	token = nextToken();
	if(token != ";")
		processError("semicolon expected");

	token = nextToken();
	code("program", name);
	insert(name, PROG_NAME, CONSTANT, name, NO, 0);
	
}//end progStmt

void Compiler::consts(){         //stage 0, production 3
	
	if(token != "const")
		processError("keyword \"const\" expected");

	token = nextToken();
	if(!isNonKeyId(token))
		processError("non-keyword identifier must follow \"const\"");

	constStmts();
	
}//end consts

void Compiler::vars(){           //stage 0, production 4
	
	if(token != "var")
		processError("keyword \"var\" expected");

	token = nextToken();
	if(!isNonKeyId(token))
		processError("non-keyword identifier must follow \"var\"");

	varStmts();
	
}//end vars

void Compiler::beginEndStmt(){   //stage 0, production 5
	
	if(token != "begin")
		processError("keyword \"begin\" expected");

	token = nextToken();  //consume begin
	
	
	
	//EXEC_STMTS HERE
	execStmts();
	

	if(token != "end")
		processError("keyword \"end\" expected");

	token = nextToken();  //consume end

	if(token != ".")
		processError("period expected");

	token = nextToken();  //consume period

	code("end", ".");
	
}//end beginEndStmt

void Compiler::constStmts(){     //stage 0, production 6
	
	string x, y;
	
	if(!isNonKeyId(token))
		processError("non-keyword identifier expected");
	
	x = token;
	
	//expect =
	token = nextToken();
	if(token != "=")
		processError("\"=\" expected");
	
	//get right-hand side token
	token = nextToken();
	y = token;
	
	if(!(y == "+" || y == "-" || y == "not" || isNonKeyId(y) || y == "true" || y == "false" || isInteger(y)))
		processError("token to the right of '=' is illegal");
	
	if((y == "+") || (y == "-")){
		
		//expect integer after sign
		token = nextToken();
		if(!(isInteger(token)))
			processError("integer expected after sign");
		
		y = y + token;
		
	}//end + - 
	
	if(y == "not"){
		
		//expect boolean after not
		token = nextToken();
		if(!(isBoolean(token)))
			processError("boolean expected after \"not\"");
		
		if(token == "true")
			y = "false";
		else
			y = "true";
		
	}//end if = not
	
	//expect semicolon
	token = nextToken();
	if(token != ";")
		processError("semicolon expected");
	
	if(whichType(y) != INTEGER && whichType(y) != BOOLEAN)
		processError("data type of token on the right-hand side must be INTEGER or BOOLEAN");
	
	insert(x, whichType(y), CONSTANT, whichValue(y), YES, 1);
	
	//fetch next token to decide whether to continue
	token = nextToken();
	x = token;
	
	if(!((x == "begin") || (x == "var") || (isNonKeyId(x))))
		processError("non-keyword identifier, \"begin\", or \"var\" expected");
	
	if(isNonKeyId(x))
		constStmts();
	
}//end constStmts

void Compiler::varStmts(){       //stage 0, production 7
	string x, y;
	
	if(!(isNonKeyId(token)))
		processError("non-keyword identifier expected");
	
	x = ids();
	
	if(token != ":")
		processError("\":\" expected");
	
	//consume type token and check it
	token = nextToken();
	if(!((token == "integer") || (token == "boolean")))
		processError("illegal type follows \":\"");
	
	y = token;
	
	//expect semicolon
	token = nextToken();
	if(token != ";")
		processError("semicolon expected");
	
	insert(x, (y == "integer" ? INTEGER : BOOLEAN), VARIABLE, "", YES, 1);
	
	//peek at next token to decide whether to continue: get it once
	token = nextToken();
	if(!(token == "begin" || isNonKeyId(token)))
		processError("non-keyword identifier or \"begin\" expected");
	

	if(isNonKeyId(token))
		varStmts();
	
}//end varStmts

string Compiler::ids(){          //stage 0, production 8
	
	string temp, tempString;
	
	if(!(isNonKeyId(token)))
		processError("non-keyword identifier expected");
	
	tempString = token;
	temp = token;
	
	//look ahead once
	token = nextToken();
	if(token == ","){
		
		//expect another identifier after comma
		token = nextToken();
		if(!(isNonKeyId(token)))
			processError("non-keyword identifier expected");
		
		tempString = temp + "," + ids();
		
	}//end if
	
	return tempString;
	
}//end ids



//Helper functions for the Pascallite lexicon
bool Compiler::isKeyword(string s) const{
	
	//added 6 additional keywords - Stage 1 (2 int ops, 2 boolean, 2 input/output)
	static const set<string> keywords = {"program", "begin", "end", "var", "const", "integer", "boolean", "true", "false", "not", "mod", "div", "and", "or", "read", "write"};
	return keywords.find(s) != keywords.end();
	
}//end isKeyword

bool Compiler::isSpecialSymbol(char c) const { // determines if c is a special symbol
    //cout << "       We're in isSpecialSymbol" << endl;

    static const std::set<char> specialSymbols = {'+', '-', '*', '/', '=', '<', '>', ';', ',', '.', '(', ')', '{', '}', '[', ']', '!', ':', '?', '&', '|'};
    return specialSymbols.find(c) != specialSymbols.end();
}//end isSpecialSymbol

bool Compiler::isNonKeyId(string s) const{    //determines if s is a non-keyword id
	
	if(isKeyword(s))
		return false;
	
	for(uint i = 0; i < s.length(); i++){
		
		if(s[i] == '_' && s[i + 1] == '_')
			return false;
		
		if(i == 0 && !(islower(s[i]) || isdigit(s[i])))
			return false;
		
		if(!(isdigit(s[i]) || islower(s[i]) || s[i] == '_'))
			return false;
		
		if(i == (s.length() - 1) && (s[i] == '_'))
			return false;
		
	}//end for
	
	return true;
	
}//end isNonKeyId


bool Compiler::isInteger(string s) const{     //determines if s is an integer

	if(s.empty())
		return false;
	
	if(s[0] == '-')
		s = s.substr(1);
	
	for(uint i = 0; i < s.length(); i++){
		
		if(!(isdigit(s[i])))
			return false;
		
	}
	
	return true;
	
}//end isInteger


bool Compiler::isBoolean(string s) const{     //determines if s is a boolean

	if(s == "false" || s == "true")
		return true;
	else
		return false;
	
}//end isBoolean


bool Compiler::isLiteral(string s) const{     //determines if s is a literal
	
	if(isInteger(s))
		return true;
	if(isBoolean(s))
		return true;
	
	if(s[0] == '0' || s[0] == '1'){
		
		s = s.substr(1);
		
		if(isInteger(s))
			return true;
		else
			return false;
		
	}//end if
	
	if(s.substr(0, 3) == "not"){
		
		s = s.substr(4);
		
		if(isBoolean(s))
			return true;
		else
			return false;
		
	}//end if
	
	return false;
	
}//end isLiteral



//Action routines
void Compiler::insert(string externalName, storeTypes inType, modes inMode, string inValue, allocation inAlloc, int inUnits){
	
	string name;
	int index;
	istringstream nameStream(externalName);
	
	while(getline(nameStream, name, ',')){
		
		index = externalName.find(',');
		name = externalName.substr(0, index);
		externalName = externalName.substr(index+1);
		//cout << endl << endl << "INLOOP: " << name << endl << endl;
	
		//Check for multiple definition in symbol table
		if(symbolTable.find(name) != symbolTable.end()){
			
			processError("symbol " + name + " is multiply defined");
			continue;  //Skip this name and move to the next one
			
		}//end if
		
		//Check if the name is a reserved keyword
		else if(isKeyword(name)){
		
			processError("Illegal use of keyword: " + name);
			continue;  //Skip this name and move to the next one
			
		}//end isKeyword else if
		
		//Determine the internal name or use external name based on rules
		else{
			if(isupper(name[0])){
				
				if(symbolTable.size() == 256)
					processError("symbol table overflow");
				
				//External name is defined by the compiler
				symbolTable.insert(pair<string, SymbolTableEntry>(name, SymbolTableEntry(name,inType,inMode,inValue,inAlloc,inUnits)));
			}
			else{
				if(symbolTable.size() == 256)
					processError("symbol table overflow");
				
				//Generate a unique internal name based on the type
				string internalName = genInternalName(inType);
				
				//symbolTable.emplace(internalName, SymbolTableEntry(name, inType, inMode, inValue, inAlloc, inUnits));
				symbolTable.insert(pair<string, SymbolTableEntry>(name, SymbolTableEntry(internalName,inType,inMode,inValue,inAlloc,inUnits)));
			
			}//end inner else
		}//end outer else
		
	}//end while

}//end insert

storeTypes Compiler::whichType(string name) {
    //cout << "       We're in whichType" << endl;

    storeTypes datatype;
    name = name.substr(0, 15);

    // Check if the name is a literal
    if(isLiteral(name))
    {
        if (isBoolean(name)) {
            // If the name is a boolean literal, return BOOLEAN type
            datatype =  BOOLEAN;
        } else { //if (isInteger(name)) {
            // If the name is an integer literal, return INTEGER type
            datatype =  INTEGER;
        }
    }
    else if (name == "integer") {
        datatype = INTEGER;
    }
    else if (name == "boolean") {
        datatype = BOOLEAN;
    }
    else
    {
        if (symbolTable.find(name) != symbolTable.end()) {
            datatype = symbolTable.at(name).getDataType();
        }
        else {
            //listingFile << "UnVar 1";
            processError("reference to undefined symbol " + name);
        }
    }

    // This return is just a safeguard; the function will exit in case of an error
    return datatype; // Default return to satisfy all control paths
}//end whichType

string Compiler::whichValue(string name){
	string value;
	name = name.substr(0, 15);
	
	if(isLiteral(name))
		value = name;
	else{
		
		if(symbolTable.find(name) != symbolTable.end())
			value = symbolTable.at(name).getValue();
		else
			processError("reference to undefined symbol " + name);
		
	}//end else
	
	return value;
	
}//end whichValue
	
	

void Compiler::code(string op, string operand1, string operand2){
	
	if(op == "program")
		emitPrologue(operand1);
	
	else if(op == "end")
		emitEpilogue();
	
	else if(op == "read")
		emitReadCode(operand1);
	else if(op == "write")
		emitWriteCode(operand1);
	else if(op == "+")//THIS MUST BE BINARY +
		emitAdditionCode(operand1, operand2);
	else if(op == "-")//THIS MUST BE BINARY -
		emitSubtractionCode(operand1, operand2);
	else if(op == "neg")//THIS MUST BE UNARY -
		emitNegationCode(operand1);
	else if(op == "not")
		emitNotCode(operand1);
	else if(op == "*")
		emitMultiplicationCode(operand1, operand2);
	else if(op == "div")
		emitDivisionCode(operand1, operand2);
	else if(op == "mod")
		emitModuloCode(operand1, operand2);
	else if(op == "and")
		emitAndCode(operand1, operand2);
	else if(op == "or")
		emitOrCode(operand1, operand2);
	else if(op == "=")
		emitEqualityCode(operand1, operand2);
	else if(op == "<>")
		emitInequalityCode(operand1, operand2);
	else if(op == ":=")
		emitAssignCode(operand1, operand2);
	else if(op == "<")
		emitLessThanCode(operand1, operand2);
	else if(op == ">")
		emitGreaterThanCode(operand1, operand2);
	else if(op == "<=")
		emitLessThanOrEqualToCode(operand1, operand2);
	else if(op == ">=")
		emitGreaterThanOrEqualToCode(operand1, operand2);
	
	else
		processError("compiler error since function code should not be called with illegal arguments");
	
}//end code

void Compiler::emit(string label, string instruction, string operands, string comment){
	//cout << "       We're in emit";

	//Ensure left justification for all output fields
	objectFile << left;

	//Output the label in a field of width 8
	objectFile << setw(8) << label;

	//Output the instruction in a field of width 8
	objectFile << setw(8) << instruction;

	//Output the operands in a field of width 24
	objectFile << setw(24) << operands;

	//Output the comment (no width constraint, align to the right)
	objectFile << comment << endl;
	
}//emit

void Compiler::emitPrologue(string progName, string operand2){
	
	time_t when = time(0);
	char* currTime = ctime(&when);
	//Output identifying comments at the beginning of the object file
	objectFile << "; Ryan Dusek & Ryan Duncan" << "       " << currTime;
	//objectFile << endl;

	//Output the %INCLUDE directives
	objectFile << "%INCLUDE \"Along32.inc\"" << endl;
	objectFile << "%INCLUDE \"Macros_Along.inc\"" << endl;
	objectFile << endl;

	//Emit the initial code section directives
	emit("SECTION", ".text");
	emit("global", "_start", "", "; program " + progName);
	objectFile << endl;
	emit("_start:");
	
}//end emitPrologue

void Compiler::emitEpilogue(string operand1, string operand2){
		
	emit("", "Exit", "{0}");
	objectFile << endl;
	emitStorage();
		
}//end emitEpilogue

void Compiler::emitStorage(){
	
	emit("SECTION", ".data");
	
	for (auto it = symbolTable.begin(); it != symbolTable.end(); it++){//for every entry in the symbol table
		string symbolName = it->first;
		
		if(symbolTable.at(symbolName).getAlloc() == YES && symbolTable.at(symbolName).getMode() == CONSTANT){
			string value = symbolTable.at(symbolName).getValue();
			if(value == "true")
				value = "-1";
				
			if(value == "false")
				value = "0";
			
			emit(symbolTable.at(symbolName).getInternalName(), "dd", value, "; " + symbolName);
			
		}//end big if
	}//end for
	
	objectFile << endl;
	
	//Emit the bss section for variable allocations
	emit("SECTION", ".bss");
	for (auto it = symbolTable.begin(); it != symbolTable.end(); it++){
		
		string symbolName = it->first;
		if(symbolTable.at(symbolName).getAlloc() == YES && symbolTable.at(symbolName).getMode() == VARIABLE)
			emit(symbolTable.at(symbolName).getInternalName(), "resd", "1", "; " + symbolName);
		
	}//end for
		
}//end emitStorage

string Compiler::nextToken() {
	//cout << "       We're in nextToken" << endl;
	
	token = "";
	while (token == "") {
		if (ch == '{') {
			nextChar();
			while (ch != END_OF_FILE && ch != '}') {
				nextChar();
			}
			if (ch == END_OF_FILE) {
				processError("unexpected end of file");
			}
			else {
				nextChar();
			}
		}
		else if (ch == '}') {
			processError("'}' cannot begin token");
		}
		else if (ch == ' ' || ch == '\n' || ch == 9) {
			nextChar();
		}
		else if (isSpecialSymbol(ch)) {
			if (ch == ':') {
				token = ch;
				if (sourceFile.peek() == '=') {
					nextChar();
					token += ch;
				}
			}
			else if (ch == '<') {
				token = ch;
				if (sourceFile.peek() == '>' || sourceFile.peek() == '=') {
					nextChar();
					token += ch;
				}
			}
			else if (ch == '>') {
				token = ch;
				if (sourceFile.peek() == '=') {
					nextChar();
					token += ch;
				}
			}
			else {
				token = ch;
			}
			nextChar();
		}
		else if (islower(ch)) {
			token = ch;
			nextChar();
			bool test = (islower(ch) || isupper(ch) || isdigit(ch) || ch == '_');
			while (test && (ch != END_OF_FILE)) {
				token += ch;
				if (ch == '_') {
					nextChar();
					if (ch == '_') {
						processError("non-keyword identifier, \"begin\", or \"var\" expected");
					}
				}
				else {
					nextChar();
				}
				test = (islower(ch) || isupper(ch) || isdigit(ch) || ch == '_');
			}
			if (ch == END_OF_FILE) {
				processError("unexpected end of file");
			}
		}
		else if (isdigit(ch)) {
			token = ch;
			nextChar();
			while (isdigit(ch) && ch != END_OF_FILE) {
				token += ch;
				nextChar();
			}
			if (ch == END_OF_FILE) {
				processError("unexpected end of file");
			}
		}
		else if (ch == END_OF_FILE) {
			token = ch;
		}
		else { // default
			processError("illegal symbol");
		}
	}
	return token;
	}//end nextToken

char Compiler::nextChar(){
	sourceFile.get(ch);
	
	if(sourceFile.eof())
		ch = END_OF_FILE;
	else{
		
		if(lineNo == 0){
			
			lineNo = 1;
			listingFile << right << setw(5) << lineNo << "|";
			
		}
	
		if(ch == '\n'){
			listingFile.put(ch);
			
			if(sourceFile.peek() != EOF){
				
				lineNo++;
				listingFile << right << setw(5) << lineNo << "|";
				
			}//end if sourceFile.peek()
		}//end if = new line
		else
			listingFile.put(ch);
		
		
	}//end else
	
	return ch;
	
}//end nextChar

string Compiler::genInternalName(storeTypes stype) const {
	string prefix;
	static int boolCount = 0;
	static int intCount = 0;

	//Determine the prefix based on the type
	switch (stype){
		
		case INTEGER:
			prefix = "I" + to_string(intCount);
			intCount++;
			break;
		case BOOLEAN:
			prefix = "B" + to_string(boolCount);
			boolCount++;
			break;
		case PROG_NAME:
			prefix = "P";
			break;
		default:
		prefix = "U";  //U for Unknown, just in case
			break;
	}
	
	return prefix;
	
}//end genInternalName











//Stage 1

void Compiler::execStmts(){
	
	execStmt();
	
	if(token != "end")
		nextToken();
	if(token == "write" || token == "read" || isNonKeyId(token))
		execStmts();
	if(token != "end")
		processError("non-keyword id, \"read\", or \"write\" expected");
	
}//end execStmts

void Compiler::execStmt(){
	
	assignStmt();
	readStmt();
	writeStmt();
	
}//end execStmt

void Compiler::assignStmt(){
	
	string pop1, pop2, pop3;
	
	if(isNonKeyId(token)){
		
		pushOperand(token);
		token = nextToken();
		
		if(token == ":="){
			
			pushOperator(token);
			nextToken();
			
			express();
			
			if(token != ";")
				processError("one of \"*\", \"and\", \"div\", \"mod\", \")\", \"+\", \"-\", \";\", \"<\", \"<=\", \"<>\", \"=\", \">\", \">=\", or \"or\" expected");
			
			pop1 = popOperator();
			pop2 = popOperand();
			pop3 = popOperand();
			
			code(pop1, pop2, pop3);
			
		}//end if :=
		else
			processError("':=' expected in assignment statement");
		
	}//end if nonkeyid
	
}//end assignStmt

void Compiler::readStmt(){
	
	string t;
	
	if(token == "read"){
		
		nextToken();
		
		if(token == "("){
			
			nextToken();
			t = ids();
			
			if(token != ")")
				processError("')' expected are non-keyword id in \"read\"");
			
			code("read", t);
			
			if(nextToken() != ";")
				processError("';' expected");
			
		}//end if next token (
		else
			processError("'(' expected after \"read\"");
		
	}//end if token read
	
}//end readStmt

void Compiler::writeStmt(){
	
	string t;
	
	if(token == "write"){
		
		nextToken();
		
		if(token == "("){
			
			nextToken();
			t = ids();
			
			if(token != ")")
				processError("',' or ')' expected after non-keyword identifier");
			else
				code("write", t);
			
			if(nextToken() != ";")
				processError("';' expected");
			
		}//end if (
		else
			processError("'(' expected after \"write\"");
		
	}//end if "write"
	
}//end writeStmt

void Compiler::express(){
	
	term();
	expresses();
	
}//end express

void Compiler::expresses(){
	
	string pop1, pop2, pop3;
	
	if(token == "<" || token == ">" || token == "=" || token == "<>" || token == "<=" || token == ">="){
		
		pushOperator(token);
		nextToken();
		
		term();
		
		pop1 = popOperator();
		pop2 = popOperand();
		pop3 = popOperand();
		
		code(pop1, pop2, pop3);
		expresses();
		
	}
	
}//end expresses

void Compiler::term(){
	
	factor();
	terms();
	
}//end term

void Compiler::terms(){
	
	string pop1, pop2, pop3;
	
	if(token == "+" || token == "-" || token == "or"){
		
		pushOperator(token);
		nextToken();
		
		factor();
		
		pop1 = popOperator();
		pop2 = popOperand();
		pop3 = popOperand();
		
		code(pop1, pop2, pop3);
		terms();
		
	}
	
}//end terms

void Compiler::factor(){
	
	part();
	factors();
	
}//end factor

void Compiler::factors(){
	
	string pop1, pop2, pop3;
	
	if(token == "*" || token == "div" || token == "mod" || token == "and"){
		
		pushOperator(token);
		nextToken();
		
		part();
		
		pop1 = popOperator();;
		pop2 = popOperand();
		pop3 = popOperand();
		
		code(pop1, pop2, pop3);
		factors();
		
	}
	
}//end factors

void Compiler::part(){
	
	string  t, popped;
	
	if(token == "not"){
		
		nextToken();
		if(token == "("){
			
			nextToken();
			express();
			
			if(token != ")")
				processError("')' expected");
			else{
				
				popped = popOperand();
				code("not", popped);
				
			}//end else
		}//end '('
		else if(isBoolean(token)){
			
			if(token == "false")
				pushOperand("true");
			else
				pushOperand("false");
			
		}//end is boolean
		else if(isNonKeyId(token))
			code("not", token);
		else
			processError("expected '(', boolean, or non-keyword id");
		
		nextToken();
		
	}//end not
	else if(token == "+" || token == "-"){
		
		t = token;
		if(nextToken() == "("){
			
			nextToken();
			express();
			
			if(token != ")")
				processError("')' expected");
			
			if(t == "-"){
				
				popped = popOperand();
				code("neg", popped);
				
			}
			
			nextToken();
			
		}//end (
		
		else if(isInteger(token)){
			
			if(t == "-")
				pushOperand("-" + token);
			else
				pushOperand(token);
			
			nextToken();
			
		}//end integer
		
		else if(isNonKeyId(token)){
			
			if(t == "-")
				code("neg", token);
			else
				pushOperand(token);
			
			nextToken();
			
		}
		
		else
			processError("expected '(', integer, or non-keyword id; found +");
		
	}//end + or -
	else if(token == "("){
		
		nextToken();
		express();
		
		if(token != ")")
			processError("')' expected");
		
		nextToken();
		
	}
	else if((isInteger(token) || isNonKeyId(token) || isBoolean(token))){
		
		pushOperand(token);
		nextToken();
		
	}
	else
		processError("expected non-keyword id, integer, \"not\", \"true\", \"false\", '(', '+', or '='");
	
}//end part







void Compiler::pushOperator(string operand){
	
	operatorStk.push(operand);
	
	
}//end pushOperator

string Compiler::popOperator(){
	
	string t;
	if(operatorStk.empty() == false){
		
		t = operatorStk.top();
		operatorStk.pop();
		return t;
		
	}
	else
		processError("Compiler error; operator stack underflow");
	
	return "";
	
}//end popOperator

void Compiler::pushOperand(string operand){
	
	//if name > 15 characters
	if(operand.length() >= 15)
		operand = operand.substr(0, 15);
	if(isLiteral(operand) && symbolTable.count(operand) == 0){
		if(operand == "true")
			symbolTable.insert({"true", SymbolTableEntry("TRUE", BOOLEAN, CONSTANT, "-1" ,YES, 1)});
		else if(operand == "false")
			symbolTable.insert({"false", SymbolTableEntry("FALSE", BOOLEAN, CONSTANT, "0", YES, 1)});
		else//     name        data type       const/var  value   allocation size
			insert(operand, whichType(operand), CONSTANT, operand, YES, 1);
		
		
	}//end if operand is literal & in symbol table
	
	operandStk.push(operand);
	
}//end pushOperand

string Compiler::popOperand(){
	
	string t;
	if(operandStk.empty() == false){
		
		t = operandStk.top();
		operandStk.pop();
		return t;
		
	}
	else
		processError("Compiler error; operand stack underflow");
	
	return "";
	
}//end popOperand








void Compiler::emitReadCode(string operand, string){
	
	string name = "";
	uint i = 0;
	
	while(i < operand.length()){
		
		name = "";
		
		while(name == ""){
			
			while(i < operand.length() && operand[i] != ','){
				
				name = name + operand[i];
				i++;
				
			}
			
			i++;
			name = name.substr(0, 15);
			
			if(symbolTable.count(name) == 0)
				processError("reference to undefined variable '" + name + "'");
			
			if(whichType(name) == BOOLEAN){
				listingFile<<endl;
				processError("can't read variables of this type");
			}
			if(symbolTable.at(name).getMode() != VARIABLE)
				processError("reading in of read-only location '" + name + "'");
			
			emit("", "call", "ReadInt", "; read int; value placed in eax");
			emit("", "mov", "[" + symbolTable.at(name).getInternalName() + "],eax", "; store eax at " + name);
			
			contentsOfAReg = name;
			
		}//end inner while
	}//end shell
	
}//end emitReadCode

void Compiler::emitWriteCode(string operand, string){
		/*
		mov     eax,[I2]                ; load a in eax
        call    WriteInt                ; write int in eax to standard out
        call    Crlf                    ; write \r\n to standard out
		*/
	
	string name = "";
	uint i = 0;
	
	while(i < operand.length()){
		
		name = "";
		
		while(name == ""){
			
			while(i < operand.length() && operand[i] != ','){
				
				name = name + operand[i];
				i++;
				
			}
			
			i++;
			name = name.substr(0, 15);
			
			if(symbolTable.count(name) == 0)
				processError("reference to undefined variable '" + name + "'");
			
			if(contentsOfAReg != name){
				
				emit("", "mov", "eax,[" + symbolTable.at(name).getInternalName() + "]", "; load " + name + " in eax");
				contentsOfAReg = name;
				
			}
			
			if(whichType(name) == INTEGER || whichType(name) == BOOLEAN)
				emit("", "call", "WriteInt", "; write int in eax to standard out");
			
			emit("", "call", "Crlf", "; write \\r\\n to standard out");
			
		}//end inner while
	}//end shell
	
}//end emitWriteCode

void Compiler::emitAssignCode(string operand1, string operand2){
	//op2 = op1
	
	if(symbolTable.at(operand1).getDataType() != symbolTable.at(operand2).getDataType())
		processError("incompatible types for operator ':='");
	
	if(symbolTable.at(operand2).getMode() != VARIABLE)
		processError("symbol on left-hand side of assignment must have a storage mode of VARIABLE");
	
	if(operand1 == operand2)
		return;
	
	if(operand1 != contentsOfAReg)
		emit("", "mov", "eax,[" + symbolTable.at(operand1).getInternalName() + "]", "; AReg = " + operand1 + "");
	
	emit("", "mov", "[" + symbolTable.at(operand2).getInternalName() + "],eax", "; " + operand2 + " = AReg");
	
	contentsOfAReg = operand2;
	
	if (isTemporary(operand1))
		freeTemp();
	
}//end emitAssignCode

void Compiler::emitAdditionCode(string operand1, string operand2){
	//op2 + op1
	
	if(whichType(operand1) != INTEGER || whichType(operand2) != INTEGER)
		processError("binary '+' requires integer operands");
	
	if(isTemporary(contentsOfAReg) && contentsOfAReg != operand1 && contentsOfAReg != operand2){
		
		emit("", "mov", "[" + symbolTable.at(contentsOfAReg).getInternalName() + "],eax", "; deassign AReg");
		symbolTable.at(contentsOfAReg).setAlloc(YES);
		contentsOfAReg = "";
		
	}
	
	if(!isTemporary(contentsOfAReg) && contentsOfAReg != operand1 && contentsOfAReg != operand2)
		contentsOfAReg = "";
	
	if(contentsOfAReg != operand1 && contentsOfAReg != operand2){
		
		emit("", "mov", "eax,[" + symbolTable.at(operand2).getInternalName() + "]", "; AReg = " + operand2);
		contentsOfAReg = operand2;
		
	}
	
	if(contentsOfAReg == operand1)
		emit("", "add", "eax,[" + symbolTable.at(operand2).getInternalName() + "]", "; AReg = " + operand1 + " + " + operand2); 
	else if(contentsOfAReg == operand2)
		emit("", "add", "eax,[" + symbolTable.at(operand1).getInternalName() + "]", "; AReg = " + operand2 + " + " + operand1);
	
	if(isTemporary(operand1))
		freeTemp();
	
	if(isTemporary(operand2))
		freeTemp();
	
	contentsOfAReg = getTemp();
	symbolTable.at(contentsOfAReg).setDataType(INTEGER);
	
	pushOperand(contentsOfAReg);
	
}//end emitAdditionCode

void Compiler::emitSubtractionCode(string operand1, string operand2){
	//op2 - op1
	
	if(whichType(operand1) != INTEGER || whichType(operand2) != INTEGER)
		processError("binary '-' requires integer operands");
	
	if(isTemporary(contentsOfAReg) && contentsOfAReg != operand2){
		
		emit("", "mov", "[" + symbolTable.at(contentsOfAReg).getInternalName() + "],eax", "; deassign AReg");
		symbolTable.at(contentsOfAReg).setAlloc(YES);
		contentsOfAReg = "";
		
	}
	
	if(!isTemporary(contentsOfAReg) && contentsOfAReg != operand2)
		contentsOfAReg = "";
	
	if(contentsOfAReg != operand2){
		
		emit("", "mov", "eax,[" + symbolTable.at(operand2).getInternalName() + "]", "; AReg = " + operand2);
		contentsOfAReg = operand2;
		
	}
	
	emit("", "sub", "eax,[" + symbolTable.at(operand1).getInternalName() + "]", "; AReg = " + operand2 + " - " + operand1);
	
	if(isTemporary(operand1))
		freeTemp();
	
	if(isTemporary(operand2))
		freeTemp();
	
	contentsOfAReg = getTemp();
	symbolTable.at(contentsOfAReg).setDataType(INTEGER);
	
	pushOperand(contentsOfAReg);
	
}//end emitSubtractionCode

void Compiler::emitMultiplicationCode(string operand1, string operand2){
	//op2 * op1
	
	if(whichType(operand1) != INTEGER || whichType(operand2) != INTEGER)
		processError("binary '*' requires integer operands");
	
	if(isTemporary(contentsOfAReg) && contentsOfAReg != operand1 && contentsOfAReg != operand2){
		
		emit("", "mov", "[" + symbolTable.at(contentsOfAReg).getInternalName() + "],eax", "; deassign AReg");
		symbolTable.at(contentsOfAReg).setAlloc(YES);
		contentsOfAReg = "";
		
	}
	
	if(!isTemporary(contentsOfAReg) && contentsOfAReg != operand1 && contentsOfAReg != operand2)
		contentsOfAReg = "";
		
	if(contentsOfAReg != operand1 && contentsOfAReg != operand2){
		
		emit("", "mov", "eax,[" + symbolTable.at(operand2).getInternalName() + "]", "; AReg = " + operand2);
		contentsOfAReg = operand2;
		
	}
	
	if(contentsOfAReg == operand1)
		emit("", "imul", "dword [" + symbolTable.at(operand2).getInternalName() + "]", "; AReg = " + operand1 + " * " + operand2);
	else if(contentsOfAReg == operand2)
		emit("", "imul", "dword [" + symbolTable.at(operand1).getInternalName() + "]", "; AReg = " + operand2 + " * " + operand1);
		
	if(isTemporary(operand1))
		freeTemp();
		
	if(isTemporary(operand2))
		freeTemp();
	
	contentsOfAReg = getTemp();
	symbolTable.at(contentsOfAReg).setDataType(INTEGER);
	//listingFile << "07" << endl;
	
	pushOperand(contentsOfAReg);
	
}//end emitMultiplicationCode

void Compiler::emitDivisionCode(string operand1, string operand2){
	//op2 / op1
	
	if(whichType(operand1) != INTEGER || whichType(operand2) != INTEGER)
		processError("binary 'div' requires integer operands");
		
	if(isTemporary(contentsOfAReg) && contentsOfAReg != operand2){
		
		emit("", "mov", "[" + symbolTable.at(contentsOfAReg).getInternalName() + "],eax", "; deassign AReg");
		symbolTable.at(contentsOfAReg).setAlloc(YES);
		contentsOfAReg = "";
		
	}
	
	if(!isTemporary(contentsOfAReg) && contentsOfAReg != operand2)
		contentsOfAReg = "";
	
	if(contentsOfAReg != operand2) {
		
		emit("", "mov", "eax,[" + symbolTable.at(operand2).getInternalName() + "]", "; AReg = " + operand2);
		contentsOfAReg = operand2;
		
	}
	
	emit("","cdq","","; sign extend dividend from eax to edx:eax");
	emit("", "idiv", "dword [" + symbolTable.at(operand1).getInternalName() + "]", "; AReg = " + operand2 + " div " + operand1);
	
	if(isTemporary(operand1))
		freeTemp();
	
	if(isTemporary(operand2))
		freeTemp();
	
	contentsOfAReg = getTemp();
	symbolTable.at(contentsOfAReg).setDataType(INTEGER);
	
	pushOperand(contentsOfAReg);
	
}//end emitDivisionCode

void Compiler::emitModuloCode(string operand1, string operand2){
	//op2 % op1
	
	if(whichType(operand1) != INTEGER || whichType(operand2) != INTEGER)
		processError("binary 'mod' requires integer operands");
		
	if(isTemporary(contentsOfAReg) && contentsOfAReg != operand2){
		
		emit("", "mov", "[" + symbolTable.at(contentsOfAReg).getInternalName() + "],eax", "; deassign AReg");
		symbolTable.at(contentsOfAReg).setAlloc(YES);
		contentsOfAReg = "";
		
	}
	
	if(!isTemporary(contentsOfAReg) && contentsOfAReg != operand2)
		contentsOfAReg = "";
	
	if(contentsOfAReg != operand2){
		emit("", "mov", "eax,[" + symbolTable.at(operand2).getInternalName() + "]", "; AReg = " + operand2);
		contentsOfAReg = operand2;
	}
	
	emit("","cdq","","; sign extend dividend from eax to edx:eax");
	emit("", "idiv", "dword [" + symbolTable.at(operand1).getInternalName() + "]", "; AReg = " + operand2 + " div " + operand1);
	emit("", "xchg", "eax,edx", "; exchange quotient and remainder");
	
	if(isTemporary(operand1))
		freeTemp();
	
	if(isTemporary(operand2))
		freeTemp();
	
	contentsOfAReg = getTemp();
	symbolTable.at(contentsOfAReg).setDataType(INTEGER);
	
	pushOperand(contentsOfAReg);
	
}//end emitModuloCode

void Compiler::emitNegationCode(string operand, string){
	//-op1
	
	if(whichType(operand) != INTEGER)
		processError("unary '-' requires an integer operand");
		
	if(isTemporary(contentsOfAReg) && contentsOfAReg != operand){
		
		emit("", "mov", "[" + symbolTable.at(contentsOfAReg).getInternalName() + "],eax", "; deassign AReg");
		symbolTable.at(contentsOfAReg).setAlloc(YES);
		contentsOfAReg = "";
		
	}
	
	if(!isTemporary(contentsOfAReg) && contentsOfAReg != operand)
		contentsOfAReg = "";
		
	if(contentsOfAReg != operand){
		
		emit("", "mov", "eax,[" + symbolTable.at(operand).getInternalName() + "]", "; AReg = " + operand);
		contentsOfAReg = operand;
		
	}
	
	emit("", "neg ", "eax", "; AReg = -AReg");
	
	if(isTemporary(operand))
		freeTemp();
	
	contentsOfAReg = getTemp();
	symbolTable.at(contentsOfAReg).setDataType(INTEGER);
	
	pushOperand(contentsOfAReg);
	
}//end emitNegationCode

void Compiler::emitNotCode(string operand, string){
	//!op1
	
	if(whichType(operand) != BOOLEAN)
		processError("unary 'not' requires a boolean operand");
	
	if(isTemporary(contentsOfAReg) && contentsOfAReg != operand){
		
		emit("", "mov", "[" + symbolTable.at(contentsOfAReg).getInternalName() + "],eax", "; deassign AReg");
		symbolTable.at(contentsOfAReg).setAlloc(YES);
		contentsOfAReg = "";
		
	}
	
	if(!isTemporary(contentsOfAReg) && contentsOfAReg != operand)
		contentsOfAReg = "";
	
	if(contentsOfAReg != operand)
	{
		emit("", "mov", "eax,[" + symbolTable.at(operand).getInternalName() + "]", "; AReg = " + operand);
		contentsOfAReg = operand;
		
	}
	
	emit("", "not", "eax", "; AReg = !AReg");
	
	if(isTemporary(operand))
		freeTemp();
	
	contentsOfAReg = getTemp();
	symbolTable.at(contentsOfAReg).setDataType(BOOLEAN);
	
	pushOperand(contentsOfAReg);
	
}//end emitNotCode

void Compiler::emitAndCode(string operand1, string operand2){
	//op2 && op1
	
	if(whichType(operand1) != BOOLEAN || whichType(operand2) != BOOLEAN)
		processError("binary 'and' requires boolean operands");
		
	if(isTemporary(contentsOfAReg) && contentsOfAReg != operand1 && contentsOfAReg != operand2){
		
		emit("", "mov", "[" + symbolTable.at(contentsOfAReg).getInternalName() + "],eax", "; deassign AReg");
		symbolTable.at(contentsOfAReg).setAlloc(YES);
		contentsOfAReg = "";
		
	}
	
	if(!isTemporary(contentsOfAReg) && contentsOfAReg != operand1 && contentsOfAReg != operand2)
		contentsOfAReg = "";
	
	if(contentsOfAReg != operand1 && contentsOfAReg != operand2) 
		emit("", "mov", "eax,[" + symbolTable.at(operand2).getInternalName() + "]", "; AReg = " + operand2);
	
	if(contentsOfAReg == operand1)
		emit("", "and", "eax,[" + symbolTable.at(operand2).getInternalName() + "]", "; AReg = " + operand1 + " and " + operand2);
	else
		emit("", "and", "eax,[" + symbolTable.at(operand1).getInternalName() + "]", "; AReg = " + operand2 + " and " + operand1);
	
	if(isTemporary(operand1))
		freeTemp();
	
	if(isTemporary(operand2))
		freeTemp();
	
	contentsOfAReg = getTemp();
	symbolTable.at(contentsOfAReg).setDataType(BOOLEAN);
	
	pushOperand(contentsOfAReg);
	
}//end emitAndCode

void Compiler::emitOrCode(string operand1, string operand2){
	//op2 || op1
	
	if(whichType(operand1) != BOOLEAN || whichType(operand2) != BOOLEAN)
		processError("binary 'or' requires a boolean operands");
	
	if(isTemporary(contentsOfAReg) && contentsOfAReg != operand1 && contentsOfAReg != operand2){
		
		emit("", "mov", "[" + symbolTable.at(contentsOfAReg).getInternalName() + "],eax", "; deassign AReg");
		symbolTable.at(contentsOfAReg).setAlloc(YES);
		contentsOfAReg = "";
		
	}
	
	if(!isTemporary(contentsOfAReg) && contentsOfAReg != operand1 && contentsOfAReg != operand2)
		contentsOfAReg = "";
	
	if(contentsOfAReg != operand1 && contentsOfAReg != operand2)
		emit("", "mov", "eax,[" + symbolTable.at(operand2).getInternalName() + "]", "; AReg = " + operand2);
	
	if(contentsOfAReg == operand1)
		emit("", "or", "eax,[" + symbolTable.at(operand2).getInternalName() + "]", "; AReg = " + operand1 + " or " + operand2);
	else
		emit("", "or", "eax,[" + symbolTable.at(operand1).getInternalName() + "]", "; AReg = " + operand2 + " or " + operand1);
	
	if(isTemporary(operand1))
		freeTemp();
	
	if(isTemporary(operand2))
		freeTemp();
	
	contentsOfAReg = getTemp();
	symbolTable.at(contentsOfAReg).setDataType(BOOLEAN);
	
	pushOperand(contentsOfAReg);
	
}//end emitOrCode

void Compiler::emitEqualityCode(string operand1, string operand2){
	//op2 == op1
	
	string label1 = getLabel();
	string label2 = getLabel();
	
	if(symbolTable.at(operand1).getDataType() != symbolTable.at(operand2).getDataType())
		processError("binary '=' requires operands of the same type");
	
	if(isTemporary(contentsOfAReg) && contentsOfAReg != operand1 && contentsOfAReg != operand2){
		
		emit("", "mov", "[" + symbolTable.at(contentsOfAReg).getInternalName() + "],eax", "; deassign AReg");
		symbolTable.at(contentsOfAReg).setAlloc(YES);
		contentsOfAReg = "";
		
	}
	
	if(!isTemporary(contentsOfAReg) && contentsOfAReg != operand1 && contentsOfAReg != operand2)
		contentsOfAReg = "";
	
	if(contentsOfAReg != operand1 && contentsOfAReg != operand2)
		emit("", "mov", "eax,[" + symbolTable.at(operand2).getInternalName() + "]", "; AReg = " + operand2);
	
	if(contentsOfAReg != operand1)
		emit("", "cmp", "eax,[" + symbolTable.at(operand1).getInternalName() + "]","; compare " + operand2 + " and " + operand1);
	else
		emit("", "cmp", "eax,[" + symbolTable.at(operand2).getInternalName() + "]","; compare " + operand1 + " and " + operand2);
	
	
	emit("", "je", label1, "; if " + operand2 + " = " + operand1 + " then jump to set eax to TRUE" );
	emit("", "mov", "eax,[FALSE]", "; else set eax to FALSE");
	
	if (symbolTable.count("false") == 0)
		symbolTable.insert({ "false", SymbolTableEntry("FALSE", BOOLEAN, CONSTANT, "0", YES, 1)});
	
	emit("", "jmp", label2, "; unconditionally jump");
	emit(label1 + ":", "", "", ""); 
	emit("", "mov", "eax,[TRUE]", "; set eax to TRUE");
	
	if(symbolTable.count("true") == 0)
		symbolTable.insert({ "true", SymbolTableEntry("TRUE", BOOLEAN, CONSTANT, "-1", YES, 1)});
	
	emit(label2 + ":", "", "", "");
	
	if(isTemporary(operand1))
		freeTemp();
	
	if(isTemporary(operand2))
		freeTemp();
	
	contentsOfAReg = getTemp();
	symbolTable.at(contentsOfAReg).setDataType(BOOLEAN);
	
	pushOperand(contentsOfAReg);
	
}//end emitEqualityCode

void Compiler::emitInequalityCode(string operand1, string operand2){
	//op2 != op1
	
	string label1 = getLabel();
	string label2 = getLabel();
	
	if(symbolTable.at(operand1).getDataType() != symbolTable.at(operand2).getDataType())
		processError("binary '<>' requires operands of the same type");
	
	if(isTemporary(contentsOfAReg) && contentsOfAReg != operand1 && contentsOfAReg != operand2){
		
		emit("", "mov", "[" + symbolTable.at(contentsOfAReg).getInternalName() + "],eax", "; deassign AReg");
		symbolTable.at(contentsOfAReg).setAlloc(YES);
		contentsOfAReg = "";
		
	}
	
	if(!isTemporary(contentsOfAReg) && contentsOfAReg != operand1 && contentsOfAReg != operand2)
		contentsOfAReg = "";
	
	if (contentsOfAReg != operand2 && contentsOfAReg != operand1)
		emit("", "mov", "eax,[" + symbolTable.at(operand2).getInternalName() + "]", "; AReg = " + operand2);
	
	if (contentsOfAReg != operand1)
		emit("", "cmp", "eax,[" + symbolTable.at(operand1).getInternalName() + "]","; compare " + operand2 + " and " + operand1);
	else
		emit("", "cmp", "eax,[" + symbolTable.at(operand2).getInternalName() + "]","; compare " + operand1 + " and " + operand2);
	
	emit("", "jne", label1, "; if " + operand2 + " <> " + operand1 + " then jump to set eax to TRUE" );
	emit("", "mov", "eax,[FALSE]", "; else set eax to FALSE");
	
	if (symbolTable.count("false") == 0)
		symbolTable.insert({ "false", SymbolTableEntry("FALSE", BOOLEAN, CONSTANT, "0", YES, 1)});
	
	emit("", "jmp", label2, "; unconditionally jump");
	emit(label1 + ":", "", "", ""); 
	emit("", "mov", "eax,[TRUE]", "; set eax to TRUE");
	
	if (symbolTable.count("true") == 0)
		symbolTable.insert({ "true", SymbolTableEntry("TRUE", BOOLEAN, CONSTANT, "-1", YES, 1)});
	
	emit(label2 + ":", "", "", "");
	
	if(isTemporary(operand1))
		freeTemp();
	
	if(isTemporary(operand2))
		freeTemp();
	
	contentsOfAReg = getTemp();
	symbolTable.at(contentsOfAReg).setDataType(BOOLEAN);
	
	pushOperand(contentsOfAReg);
	
}//end emitInequalityCode

void Compiler::emitLessThanCode(string operand1, string operand2){
	//op2 < op1
	
	string label1 = getLabel();
	string label2 = getLabel();
	
	if(symbolTable.at(operand1).getDataType() != symbolTable.at(operand2).getDataType())
		processError("binary '<' requires integer operands");
	
	if(isTemporary(contentsOfAReg) && contentsOfAReg != operand1 && contentsOfAReg != operand2){
		
		emit("", "mov", "[" + symbolTable.at(contentsOfAReg).getInternalName() + "],eax", "; deassign AReg");
		symbolTable.at(contentsOfAReg).setAlloc(YES);
		contentsOfAReg = "";
		
	}
	
	if(!isTemporary(contentsOfAReg) && contentsOfAReg != operand1 && contentsOfAReg != operand2)
		contentsOfAReg = "";
	
	if(contentsOfAReg != operand2)
		emit("", "mov", "eax,[" + symbolTable.at(operand2).getInternalName() + "]", "; AReg = " + operand2);
	
	
	emit("", "cmp", "eax,[" + symbolTable.at(operand1).getInternalName() + "]","; compare " + operand2 + " and " + operand1);
	
	
	emit("", "jl", label1, "; if " + operand2 + " < " + operand1 + " then jump to set eax to TRUE" );
	emit("", "mov", "eax,[FALSE]", "; else set eax to FALSE");
	
	if (symbolTable.count("false") == 0)
		symbolTable.insert({ "false", SymbolTableEntry("FALSE", BOOLEAN, CONSTANT, "0", YES, 1)});
	
	emit("", "jmp", label2, "; unconditionally jump");
	emit(label1 + ":", "", "", ""); 
	emit("", "mov", "eax,[TRUE]", "; set eax to TRUE");
	
	if (symbolTable.count("true") == 0) 
		symbolTable.insert({ "true", SymbolTableEntry("TRUE", BOOLEAN, CONSTANT, "-1", YES, 1)});
	
	emit(label2 + ":", "", "", "");
	
	if(isTemporary(operand1))
		freeTemp();
	
	if(isTemporary(operand2))
		freeTemp();
	
	contentsOfAReg = getTemp();
	symbolTable.at(contentsOfAReg).setDataType(BOOLEAN);
	
	pushOperand(contentsOfAReg);
	
}//end emitLessThanCode

void Compiler::emitLessThanOrEqualToCode(string operand1, string operand2){
	//op2 <= op1
	
	string label1 = getLabel();
	string label2 = getLabel();
	
	if(symbolTable.at(operand1).getDataType() != symbolTable.at(operand2).getDataType())
		processError("binary '<=' requires integer operands");
	
	if(isTemporary(contentsOfAReg) && contentsOfAReg != operand1 && contentsOfAReg != operand2){
		
		emit("", "mov", "[" + symbolTable.at(contentsOfAReg).getInternalName() + "],eax", "; deassign AReg");
		symbolTable.at(contentsOfAReg).setAlloc(YES);
		contentsOfAReg = "";
		
	}
	
	if(!isTemporary(contentsOfAReg) && contentsOfAReg != operand1 && contentsOfAReg != operand2)
		contentsOfAReg = "";
	
	if(contentsOfAReg != operand2)
		emit("", "mov", "eax,[" + symbolTable.at(operand2).getInternalName() + "]", "; AReg = " + operand2);
	
	
	emit("", "cmp", "eax,[" + symbolTable.at(operand1).getInternalName() + "]","; compare " + operand2 + " and " + operand1);
	
	
	emit("", "jle", label1, "; if " + operand2 + " <= " + operand1 + " then jump to set eax to TRUE" );
	emit("", "mov", "eax,[FALSE]", "; else set eax to FALSE");
	
	if(symbolTable.count("false") == 0)
		symbolTable.insert({ "false", SymbolTableEntry("FALSE", BOOLEAN, CONSTANT, "0", YES, 1)});
	
	emit("", "jmp", label2, "; unconditionally jump");
	emit(label1 + ":", "", "", ""); 
	emit("", "mov", "eax,[TRUE]", "; set eax to TRUE");
	
	if(symbolTable.count("true") == 0)
		symbolTable.insert({ "true", SymbolTableEntry("TRUE", BOOLEAN, CONSTANT, "-1", YES, 1)});
	
	emit(label2 + ":", "", "", "");
	
	if(isTemporary(operand1))
		freeTemp();
	
	if(isTemporary(operand2))
		freeTemp();
	
	contentsOfAReg = getTemp();
	symbolTable.at(contentsOfAReg).setDataType(BOOLEAN);
	
	pushOperand(contentsOfAReg);
	
}//end emitLessThanOrEqualToCode

void Compiler::emitGreaterThanCode(string operand1, string operand2){
	//op2 > op1
	
	string label1 = getLabel();
	string label2 = getLabel();
	
	if(symbolTable.at(operand1).getDataType() != symbolTable.at(operand2).getDataType())
		processError("binary '>' requires integer operands");
	
	if(isTemporary(contentsOfAReg) && contentsOfAReg != operand1 && contentsOfAReg != operand2){
		
		emit("", "mov", "[" + symbolTable.at(contentsOfAReg).getInternalName() + "],eax", "; deassign AReg");
		symbolTable.at(contentsOfAReg).setAlloc(YES);
		contentsOfAReg = "";
		
	}
	
	if(!isTemporary(contentsOfAReg) && contentsOfAReg != operand1 && contentsOfAReg != operand2)
		contentsOfAReg = "";
	
	if(contentsOfAReg != operand2)
		emit("", "mov", "eax,[" + symbolTable.at(operand2).getInternalName() + "]", "; AReg = " + operand2);
	
	
	emit("", "cmp", "eax,[" + symbolTable.at(operand1).getInternalName() + "]","; compare " + operand2 + " and " + operand1);
	
	
	emit("", "jg", label1, "; if " + operand2 + " > " + operand1 + " then jump to set eax to TRUE" );
	emit("", "mov", "eax,[FALSE]", "; else set eax to FALSE");
	
	if(symbolTable.count("false") == 0)
		symbolTable.insert({ "false", SymbolTableEntry("FALSE", BOOLEAN, CONSTANT, "0", YES, 1)});
	
	emit("", "jmp", label2, "; unconditionally jump");
	emit(label1 + ":", "", "", ""); 
	emit("", "mov", "eax,[TRUE]", "; set eax to TRUE");
	
	if(symbolTable.count("true") == 0)
		symbolTable.insert({ "true", SymbolTableEntry("TRUE", BOOLEAN, CONSTANT, "-1", YES, 1)});
	
	emit(label2 + ":", "", "", "");
	
	if(isTemporary(operand1))
		freeTemp();
	
	if(isTemporary(operand2))
		freeTemp();
	
	contentsOfAReg = getTemp();
	symbolTable.at(contentsOfAReg).setDataType(BOOLEAN);
	
	pushOperand(contentsOfAReg);
	
}//end emitGreaterThanCode

void Compiler::emitGreaterThanOrEqualToCode(string operand1, string operand2){
	//op2 >= op1
	
	string label1 = getLabel();
	string label2 = getLabel();
	
	if(symbolTable.at(operand1).getDataType() != symbolTable.at(operand2).getDataType())
		processError("binary '>=' requires integer operands");
	
	if(isTemporary(contentsOfAReg) && contentsOfAReg != operand1 && contentsOfAReg != operand2){
		
		emit("", "mov", "[" + symbolTable.at(contentsOfAReg).getInternalName() + "],eax", "; deassign AReg");
		symbolTable.at(contentsOfAReg).setAlloc(YES);
		contentsOfAReg = "";
		
	}
	
	if(!isTemporary(contentsOfAReg) && contentsOfAReg != operand1 && contentsOfAReg != operand2)
		contentsOfAReg = "";
	
	if (contentsOfAReg != operand2)
		emit("", "mov", "eax,[" + symbolTable.at(operand2).getInternalName() + "]", "; AReg = " + operand2);
	
	
	emit("", "cmp", "eax,[" + symbolTable.at(operand1).getInternalName() + "]","; compare " + operand2 + " and " + operand1);
	
	
	emit("", "jge", label1, "; if " + operand2 + " >= " + operand1 + " then jump to set eax to TRUE" );
	emit("", "mov", "eax,[FALSE]", "; else set eax to FALSE");
	
	if (symbolTable.count("false") == 0)
		symbolTable.insert({ "false", SymbolTableEntry("FALSE", BOOLEAN, CONSTANT, "0", YES, 1)});
	
	emit("", "jmp", label2, "; unconditionally jump");
	emit(label1 + ":", "", "", ""); 
	emit("", "mov", "eax,[TRUE]", "; set eax to TRUE");
	
	if (symbolTable.count("true") == 0)
		symbolTable.insert({ "true", SymbolTableEntry("TRUE", BOOLEAN, CONSTANT, "-1", YES, 1)});
	
	emit(label2 + ":", "", "", "");
	
	if(isTemporary(operand1))
		freeTemp();
	
	if(isTemporary(operand2))
		freeTemp();
	
	contentsOfAReg = getTemp();
	symbolTable.at(contentsOfAReg).setDataType(BOOLEAN);
	
	pushOperand(contentsOfAReg);
	
}//end emitGreaterThanOrEqualToCode





void Compiler::freeTemp(){
	
	currentTempNo--;
	if(currentTempNo < -1)
		processError("compiler error, currentTempNo should be >= -1");
	
}//end freeTemp

string Compiler::getTemp(){
	
	string temp;
	currentTempNo++;
	temp = "T" + to_string(currentTempNo);
	
	if(currentTempNo > maxTempNo){
		
		insert(temp, UNKNOWN, VARIABLE, "", NO, 1);
		maxTempNo++;
		
	}
	
	return temp;
	
}//end getTemp

string Compiler::getLabel(){
	
	static int labelCount = -1;
	string label;
	
	labelCount++;
	label = ".L" + to_string(labelCount);
	
	return label;
	
}//end getLabel

bool Compiler::isTemporary(string s) const{
	
	if(symbolTable.count(s) != 0)
		if(symbolTable.at(s).getInternalName()[0] == 'T' && symbolTable.at(s).getInternalName() != "TRUE")
			return true;
	
	return false;
	
}//end isTemporary



//Done:
//added 6 keywords


//To do
//add 9 tokens :=, *, (, ), <>, <, <=, >=, >

//revisions:
//EXEC_STMTS in beginEndStmt

//additional functions
//EXEC_STMTS
//ASSIGN_STMT
//READ_STMT
//READ_LIST
//WRITE_STMT
//WRITE_LIST
//EXPRESSES
//TERMS
//FACTORS
//REL_OP
//ADD_LEVEL_OP
//MULT_LEVEL_OP

//identifier left of := must be variable name and same type as right side
//operands of binary + - div mod * must be integer expressions
//operands of unary + - must be integer expressions
//operands of logical operators and or not must be boolean
//relational operands < > <= >= must be integer
//= <> may be BOTH integer or BOTH boolean (CANNOT mix)

//OPERATOR PRECEDENCE PAGE 6