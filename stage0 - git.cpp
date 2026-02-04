#include <stage0.h>
#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <ctime>
#include <iomanip>
#include <set>
#include <bits/stdc++.h>
#include <cctype>



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
	
	auto now = chrono::system_clock::now();
    time_t now_time = chrono::system_clock::to_time_t(now);

    // Format time as a string
    stringstream ss;
    ss << put_time(localtime(&now_time), "%b %d %Y  %H:%M:%S");
    string currTime = ss.str();
	
	//top line
	listingFile << "STAGE0:  Ryan Dusek & Ryan Duncan       " << currTime << endl << endl;//Myself and Group member
	
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
	
    listingFile << endl 
                << "Error: Line " << displayLine << ": " << err << endl;
	
    cerr << "Error: " << err << endl;
	
    createListingTrailer();
    listingFile.flush();
	
    exit(EXIT_FAILURE);
	
}//end processError





//Methods implementing the grammar productions
void Compiler::prog(){           //stage 0, production 1
	
    if (token != "program")
        processError("keyword \"program\" expected");

    progStmt();

    if (token == "const")
        consts();

    if (token == "var")
        vars();

    if (token != "begin")
        processError("keyword \"begin\" expected");

    beginEndStmt();

    if (token[0] != END_OF_FILE)
        processError("no text may follow \"end\"");
	
}//end prog

void Compiler::progStmt(){       //stage 0, production 2
	
    string name;

    if (token != "program")
        processError("keyword \"program\" expected");

    token = nextToken();  
    name = token;

    if (!isNonKeyId(name))
        processError("program name expected");

    token = nextToken();
    if (token != ";")
        processError("semicolon expected");

    token = nextToken();
    code("program", name);
    insert(name, PROG_NAME, CONSTANT, name, NO, 0);
	
}//end progStmt

void Compiler::consts(){         //stage 0, production 3
	
	if (token != "const")
        processError("keyword \"const\" expected");

    token = nextToken();
    if (!isNonKeyId(token))
        processError("non-keyword identifier must follow \"const\"");

    constStmts();
	
}//end consts

void Compiler::vars(){           //stage 0, production 4
	
    if (token != "var")
        processError("keyword \"var\" expected");

    token = nextToken();
    if (!isNonKeyId(token))
        processError("non-keyword identifier must follow \"var\"");

    varStmts();
	
}//end vars

void Compiler::beginEndStmt(){   //stage 0, production 5
	
    if (token != "begin")
        processError("keyword \"begin\" expected");

    token = nextToken();  //consume begin

    if (token != "end")
        processError("keyword \"end\" expected");

    token = nextToken();  //consume end

    if (token != ".")
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
	
	if (!(y == "+" || y == "-" || y == "not" || isNonKeyId(y) || y == "true" || y == "false" || isInteger(y)))
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
	if (!(token == "begin" || isNonKeyId(token)))
        processError("non-keyword identifier or \"begin\" expected");
	

    if (isNonKeyId(token))
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
	
    static const set<string> keywords = {"program", "begin", "end", "var", "const", "integer", "boolean", "true", "false", "not"};
	return keywords.find(s) != keywords.end();
	
}//end isKeyword

bool Compiler::isSpecialSymbol(char c) const{ //determines if c is a special symbol
	
    static const set<char> specialSymbols = {'=', ':', ',', ';', '.', '+', '-'};
	return specialSymbols.find(c) != specialSymbols.end();
	
}//end isSpecialSymbol

bool Compiler::isNonKeyId(string s) const{    //determines if s is a non_key_id
	
    if(isKeyword(s))
		return false;
	
	for(uint i = 0; i < s.length(); i++){
		
		if(s[i] == '_' && s[i + 1] == '_')
			return false;
		
		if(i == 0 && !(islower(s[i]) || isdigit(s[i])))
			return false;
		
		if (!(isdigit(s[i]) || islower(s[i]) || s[i] == '_'))
			return false;
		
		if (i == (s.length() - 1) && (s[i] == '_'))
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
		
	}
	
	if(s.substr(0, 3) == "not"){
		
		s = s.substr(4);
		if(isBoolean(s))
			return true;
		else
			return false;
		
	}
	
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
        if (symbolTable.find(name) != symbolTable.end()){
			
            processError("symbol " + name + " is multiply defined");
            continue;  //Skip this name and move to the next one
			
        }
        
         //Check if the name is a reserved keyword
        else if (isKeyword(name)){
		
            processError("Illegal use of keyword: " + name);
            continue;  //Skip this name and move to the next one
			
        }
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

storeTypes Compiler::whichType(string name){
	
	storeTypes dataType;
	name = name.substr(0, 15);
	
	if(isLiteral(name)){
		if(isBoolean(name))
			dataType = BOOLEAN;
		else
			dataType = INTEGER;
	}
	
	else{
		if (symbolTable.find(name) != symbolTable.end())
            dataType = symbolTable.at(name).getDataType();
        else
            processError("reference to undefined symbol " + name);
	}//end outer else
	
	return dataType;
	
}//end whichType

string Compiler::whichValue(string name){
	string value;
	
	if(isLiteral(name))
		value = name;
	else{
		
        if (symbolTable.find(name) != symbolTable.end())
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
    if(comment.find("program") != string::npos){
		
        comment = comment.substr(comment.find("program")+8);
		
        if(comment.length() > 15)
            comment = comment.substr(0,15);
		
        comment = "; program " + comment;
    }
    else if(comment.length() > 15)
        comment = comment.substr(0,17);
	
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
		
		if (symbolTable.at(symbolName).getAlloc() == YES && symbolTable.at(symbolName).getMode() == CONSTANT){
			string value = symbolTable.at(symbolName).getValue();
			if (value == "true")
				value = "-1";
				
			if (value == "false")
				value = "0";
			
			emit(symbolTable.at(symbolName).getInternalName(), "dd", value, "; " + symbolName);
			
		}//end big if
	}//end for
	
	objectFile << endl;
	
	//Emit the bss section for variable allocations
	emit("SECTION", ".bss");
	for (auto it = symbolTable.begin(); it != symbolTable.end(); it++){
		
		string symbolName = it->first;
		if (symbolTable.at(symbolName).getAlloc() == YES && symbolTable.at(symbolName).getMode() == VARIABLE)
			emit(symbolTable.at(symbolName).getInternalName(), "resd", "1", "; " + symbolName);
		
	}//end for
		
}//end emitStorage

string Compiler::nextToken(){
	
	token = "";
	
	while(token == ""){
		
		if(ch == '{'){
			while(nextChar() != END_OF_FILE && ch != '}'){
				//Empty loop body to skip comment
			}
			if(ch == END_OF_FILE)
				processError("unexpected end of file within a comment");
			else
				nextChar();
		}//end if '{'
		
		else if(ch == '}')
			processError("'}' cannot begin a token");
		
		else if(isspace(ch))
			nextChar();
		
		else if(isSpecialSymbol(ch)){
			token = ch;
			nextChar();
		}//end isSpecialSymbol
		
		else if(islower(ch)){
			
			token = ch;
			while(isalnum(nextChar()) || ch == '_')
				token += ch;
			if (ch == END_OF_FILE)
				processError("unexpected end of file");
			
		}//end islower
		
		else if(isdigit(ch)){
			
			token = ch;
			while(isdigit(nextChar()))
				token += ch;
			if (ch == END_OF_FILE)
				processError("unexpected end of file");
			
		}//end else if
		
		else if(ch == END_OF_FILE)
			token = ch;
		else
			processError("illegal symbol: " + string(1, ch));
		
	}//end while token is ""
	
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
	}
	
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
