#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include "math.h"
#include <sstream>
#include <sstream>

#include <fstream>
#include <stdlib.h>
#include <windows.h>
#include <Lmcons.h>

using namespace std;

struct Label;

//vectors
vector<string> commands;
vector<Label> labels;

string input;

string code;
int cellSize = 2, memSize = 16384;

//byte arrays for memory
byte* memory;
byte* buffer;

byte* tempStorage1;
byte* tempStorage2;
byte* tempStorage3;

//Math functions
void addBytes(byte*, byte*);
void subtractBytes(byte*, byte*);
void multiplyBytes(byte*, byte*);
void divideBytes(byte*, byte*);
void moduloBytes(byte*, byte*);
void compareBytes(byte*, byte*);
void compare(byte*, byte*);

//functions
void setPointer(int, byte*);
byte* getPointer(int);
int eval(string);
void removeWhitespace(string s, vector<string>*);
//integer to byte array conversion
byte* int_to_byte(int);
int byte_to_int(byte*);
byte* llong_to_byte(long long);
int getBinaryValue(string*);
//string conversion
string to_string(int);
int to_int(string);
long long to_llong(string);

//test function
void printVector(vector<string>*);

//struct that is used in a Label vector; stores label names and their locations
struct Label {
    int name;
    int position;
};

//finds pointers and "lumps" them into one string in the command function
//this allows for easier and quicker execution of the code
void findPointers(vector<string>* com) {
    unsigned int i = 0;
    vector<string> commands = *com;
    vector<string> newCommands;

    while(i < commands.size()) {
        if(commands.at(i).compare("POINTER") == 0) {
            int counter = 0;
            string newCommand = "";
            do {
                if(commands.at(i).compare("POINTER") == 0) {
                    counter++;
                }else if(commands.at(i).compare("CLOSE_POINTER") == 0) {
                    counter--;
                }
                newCommand += commands.at(i) + " ";
                i++;
            } while(counter != 0);
            newCommands.push_back(newCommand);
        }else{
            newCommands.push_back(commands.at(i));
            i++;
        }
    }
    *com = newCommands;
}

//function checks if the inputed string is a NUMBER, EXPR, INPUT, or POINTER
//if any is true, then it can be evaluated as a number
bool isNumber(string s) {
    if(s.substr(0,4).compare("EXPR") == 0 || s.substr(0,5).compare("INPUT") == 0 || s.substr(0,6).compare("NUMBER") ==0 || s.substr(0,7).compare("POINTER") == 0)
        return true;
    return false;
}

//This function parses the code and lumps together numeric expressions
//for example {"3", "+", "4"} would be lumped into {"3 + 4"}
void findExpr(vector<string>* com) {
    unsigned int i = 0;
    vector<string> commands = *com;
    vector<string> newCommands;
    string newCommand = "";

    while(i < commands.size()) {
        try {
            string currentCommand = commands.at(i);
            if(isNumber(currentCommand)) {
                string nextCommand = commands.at(i + 1);
                if(nextCommand.compare("ADD") == 0 || nextCommand.compare("SUBTRACT") == 0 || nextCommand.compare("MULTIPLY") == 0 || nextCommand.compare("DIVIDE") == 0 ||
                   nextCommand.compare("MODULO") == 0 || nextCommand.compare("COMPARE") == 0 || nextCommand.compare("NOT") == 0 || nextCommand.compare("GREATER_THAN") == 0 ||
                   nextCommand.compare("LESS_THAN") == 0) {
                        newCommand = "EXPR: ";
                        newCommand += commands.at(i) + " " + commands.at(i + 1) + " " + commands.at(i + 2);
                        i+=3;
                        while(true && i < commands.size()) {
                            nextCommand = commands.at(i + 1);
                            if(nextCommand.compare("ADD") == 0 || nextCommand.compare("SUBTRACT") == 0 || nextCommand.compare("MULTIPLY") == 0 || nextCommand.compare("DIVIDE") == 0 ||
                               nextCommand.compare("MODULO") == 0 || nextCommand.compare("COMPARE") == 0 || nextCommand.compare("NOT") == 0 || nextCommand.compare("GREATER_THAN") == 0 ||
                               nextCommand.compare("LESS_THAN") == 0 ) {
                                    newCommand += " " + commands.at(i) + " " + commands.at(i + 1);
                                    i+=2;
                            }else{
                                break;
                            }
                        }
                        newCommands.push_back(newCommand);
                        newCommand = "";
                }else{
                    newCommands.push_back(commands.at(i));
                    i++;
                }
            }else{
                newCommands.push_back(commands.at(i));
                i++;
            }
        }catch(const exception &e) {
            newCommands.push_back(commands.at(i));
            i++;
        }
    }

    *com = newCommands;

}

//finds if, else, and end statements and appends a number to the end of their name
//this allows the executor to quickly jumping to different parts of the code when if statements are encountered
void labelIfs(vector<string>* com) {
    unsigned int i = 0;
    unsigned int counter = 0;
    vector<string> commands = *com;
    while(i < commands.size()) {
        if(commands.at(i).compare("IF") == 0) {
            counter++;
            commands.insert(commands.begin() + i, commands.at(i) + ":" + to_string(counter));
            commands.erase(commands.begin() + i + 1);
        }else if(commands.at(i).compare("ELSE") == 0) {
            commands.insert(commands.begin() + i, commands.at(i) + ":" + to_string(counter));
            commands.erase(commands.begin() + i + 1);
        }else if(commands.at(i).compare("END") == 0) {
            commands.insert(commands.begin() + i, commands.at(i) + ":" + to_string(counter));
            commands.erase(commands.begin() + i + 1);
            counter--;
        }
        i++;
    }
    *com = commands;
}

//function tidies up the code by lumping commands together
void tidyUp(vector<string>* commands) {
    findPointers(commands);
    findExpr(commands);
    labelIfs(commands);
}

//this takes a pointer command and returns the value associated with that pointer.
//Will also evaluate pointers with nested pointers and expressions
byte* evalPointer(string ptr) {
    vector<string> ptrString;
    removeWhitespace(ptr, &ptrString);

    ptrString.erase(ptrString.begin());
    ptrString.pop_back();

    findPointers(&ptrString);

    string temp = "";
    for(unsigned int i = 0; i < ptrString.size(); i++) {
        temp += ptrString.at(i) + " ";
    }

    int pointer = eval(temp);
	//remove the byte_to_int to change return type to byte pointer
    return getPointer(pointer);
}

//this function returns the leftmost character of the input and then removes it from the input string
char getInput() {
    char temp;
    if(input.length() >= 1) {
        temp = (input.c_str())[0];
        input = input.substr(1);
    }else{
        temp = 0;
        input = "";
    }
    return temp;
}

//determines what type of number is inputted and returns it in perfectly numeric form
int findValue(string val) {
    if(val.substr(0,5).compare("INPUT") == 0) {
        return getInput();
    }else if(val.substr(0,6).compare("NUMBER") == 0) {
        return to_int(val.substr(7));
    }else if(val.substr(0,7).compare("POINTER") == 0) {
        return evalPointer(val);
    }
    return to_int(val);
}

byte* findValueByte(string val) {
    if(val.substr(0,5).compare("INPUT") == 0) {
		for(int i = 0; i < cellSize; i++) buffer[i] = 0;
        buffer[0] = getInput();
		return buffer;
    }else if(val.substr(0,6).compare("NUMBER") == 0) {
        return llong_to_byte(to_llong(val.substr(7)));
    }else if(val.substr(0,7).compare("POINTER") == 0) {
        return evalPointer(val);
    }
    return to_int(val);
}

//evaluates an expression
int eval(string s) {
    vector<string> ops;
    removeWhitespace(s, &ops);
    for(int i = 0; i < ops.size(); i++) {
        if(ops.at(i).compare("EXPR:") == 0 || ops.at(i).compare("") == 0) ops.erase(ops.begin() + i);
    }

    ops.shrink_to_fit();
    findPointers(&ops);

    unsigned int i = 0;

    int size = ops.size();

    while(i < ops.size()) {
        if(ops.at(i).compare("MULTIPLY") == 0) {
            int value1 = findValue(ops.at(i - 1));
            int value2 = findValue(ops.at(i + 1));
            int value3 = value1 * value2;
            ops.erase(ops.begin() + i);
            ops.insert(ops.begin() + i, to_string(value3));
            ops.erase(ops.begin() + i - 1);
            ops.erase(ops.begin() + i + 1);
        }else if(ops.at(i).compare("DIVIDE") == 0) {
            int value1 = findValue(ops.at(i - 1));
            int value2 = findValue(ops.at(i + 1));
            int value3 = value1 / value2;
            ops.erase(ops.begin() + i);
            ops.insert(ops.begin() + i, to_string(value3));
            ops.erase(ops.begin() + i - 1);
            ops.erase(ops.begin() + i + 1);
        }else if(ops.at(i).compare("MODULO") == 0) {
            int value1 = findValue(ops.at(i - 1));
            int value2 = findValue(ops.at(i + 1));
            int value3 = value1 % value2;
            ops.erase(ops.begin() + i);
            ops.insert(ops.begin() + i, to_string(value3));
            ops.erase(ops.begin() + i - 1);
            ops.erase(ops.begin() + i + 1);
        }
        i++;
        size = ops.size();
    }
    i = 0;
    while(i < ops.size()) {
        if(ops.at(i).compare("ADD") == 0) {
            int value1 = findValue(ops.at(i - 1));
            int value2 = findValue(ops.at(i + 1));
            int value3 = value1 + value2;
            ops.erase(ops.begin() + i);
            ops.insert(ops.begin() + i, to_string(value3));
            ops.erase(ops.begin() + i - 1);
            ops.erase(ops.begin() + i + 1);
        }else if(ops.at(i).compare("SUBTRACT") == 0) {
            int value1 = findValue(ops.at(i - 1));
            int value2 = findValue(ops.at(i + 1));
            int value3 = value1 - value2;
            ops.erase(ops.begin() + i);
            ops.insert(ops.begin() + i, to_string(value3));
            ops.erase(ops.begin() + i - 1);
            ops.erase(ops.begin() + i + 1);
        }
        i++;
        size = ops.size();
    }
    i = 0;
    while(i < ops.size()) {
        if(ops.at(i).compare("COMPARE") == 0) {
            int value1 = findValue(ops.at(i - 1));
            int value2 = findValue(ops.at(i + 1));
            int value3 = 0;
            if(value1 == value2) value3 = 1;
            ops.erase(ops.begin() + i);
            ops.insert(ops.begin() + i, to_string(value3));
            ops.erase(ops.begin() + i - 1);
            ops.erase(ops.begin() + i + 1);
        }else if(ops.at(i).compare("NOT") == 0) {
            int value1 = findValue(ops.at(i - 1));
            int value2 = findValue(ops.at(i + 1));
            int value3 = 1;
            if(value1 == value2) value3 = 0;
            ops.erase(ops.begin() + i);
            ops.insert(ops.begin() + i, to_string(value3));
            ops.erase(ops.begin() + i - 1);
            ops.erase(ops.begin() + i + 1);
        }else if(ops.at(i).compare("GREATER_THAN") == 0) {
            int value1 = findValue(ops.at(i - 1));
            int value2 = findValue(ops.at(i + 1));
            int value3 = 0;
            if(value1 > value2) value3 = 1;
            ops.erase(ops.begin() + i);
            ops.insert(ops.begin() + i, to_string(value3));
            ops.erase(ops.begin() + i - 1);
            ops.erase(ops.begin() + i + 1);
        }else if(ops.at(i).compare("LESS_THAN") == 0) {
            int value1 = findValue(ops.at(i - 1));
            int value2 = findValue(ops.at(i + 1));
            int value3 = 0;
            if(value1 < value2) value3 = 1;
            ops.erase(ops.begin() + i);
            ops.insert(ops.begin() + i, to_string(value3));
            ops.erase(ops.begin() + i - 1);
            ops.erase(ops.begin() + i + 1);
        }
        i++;
    }
    return findValue(ops.at(0));
}

//parses the code and adds commands to the command vector
void parse(string* code) {
    string newCode = *code;
    int length = newCode.length();
    char toChar[length + 1];
    strcpy(toChar, newCode.c_str());

    string tok = "";
    byte state = 0;

    string currentCommand = "";

    //runs through each character of code and matches the commands
    for(unsigned int i = 0; i < length; i++) {
        tok += toChar[i];
        if(state == 0) {
            if(tok.compare("l[") == 0) {
                currentCommand = "POINTER";
            }else if(tok.compare("l]") == 0) {
                currentCommand = "CLOSE_POINTER";
            }else if(tok.compare("ll") == 0) {
                currentCommand = "EQUALS";
            }else if(tok.compare("l1") == 0) {
                currentCommand = "LABEL";
            }else if(tok.compare("l|") == 0) {
                currentCommand = "GOTO";
            }else if(tok.compare("|]") == 0) {
                currentCommand = "ADD";
            }else if(tok.compare("|[") == 0) {
                currentCommand = "SUBTRACT";
            }else if(tok.compare("|1") == 0) {
                currentCommand = "MULTIPLY";
            }else if(tok.compare("|l") == 0) {
                currentCommand = "DIVIDE";
            }else if(tok.compare("||") == 0) {
                currentCommand = "MODULO";
            }else if(tok.compare("1]") == 0) {
                currentCommand = "INPUT";
            }else if(tok.compare("1[") == 0) {
                currentCommand = "PRINT";
            }else if(tok.compare("1|[") == 0) {
                currentCommand = "IF";
            }else if(tok.compare("1|]") == 0) {
                currentCommand = "END";
            }else if(tok.compare("1l") == 0) {
                currentCommand = "ELSE";
            }else if(tok.compare("111") == 0) {
                currentCommand = "COMPARE";
            }else if(tok.compare("11[") == 0) {
                currentCommand = "LESS_THAN";
            }else if(tok.compare("11|") == 0) {
                currentCommand = "NOT";
            }else if(tok.compare("11]") == 0) {
                currentCommand = "GREATER_THAN";
            }else if(tok.compare("[") == 0 || tok.compare("]") == 0) {
                currentCommand = "NUMBER:";
                state = 1;
            }
            if(currentCommand != "" && state == 0) {
                tok = "";
                commands.push_back(currentCommand);
                currentCommand = "";
            }

        }else{
            //parses numbers; if the last digit is not [ or ], then the number has ended and a new command has begun
            if(tok.at(tok.length() - 1) == 'l' || tok.at(tok.length() - 1) == '|' || tok.at(tok.length() - 1) == '1') {
                state = 0;
                //the current command becomes NUMBER: followed by the value of the string. that command is then added to the commands list
                string sub = tok.substr(0, tok.length() - 1);
                currentCommand += to_string(getBinaryValue(&sub));
                commands.push_back(currentCommand);
                currentCommand = "";
                tok = tok.at(tok.length() - 1);
            }
        }
    }

    //runs if the last command was a number
    if(currentCommand.compare("") != 0) commands.push_back(currentCommand + to_string(getBinaryValue(&tok)));
}

//executes the code
void execute(vector<string>* com) {
    vector<string> commands = *com;
    unsigned int i = 0;
    while(i < commands.size()) {
        if(commands.at(i).compare("LABEL") == 0) {
            Label temp;
            temp.name = eval(commands.at(i + 1));
            temp.position = i;
            labels.push_back(temp);
        }
        i++;
    }

    i = 0;

    while(i < commands.size()) {
        if(commands.at(i).compare("GOTO") == 0) {
            int labelName = eval(commands.at(i + 1));
            for(Label l : labels) {
                if(l.name == labelName) {
                    i = l.position + 1;
                    break;
                }
                i++;
            }
        }else if(commands.at(i).compare("PRINT") == 0) {
            if(isNumber(commands.at(i + 1))) {
                cout << (char)eval(commands.at(i + 1));
                i+=2;
            }else {
                i++;
            }
        }else if(commands.at(i).substr(0,2).compare("IF") == 0) {
            int level = to_int(commands.at(i).substr(3));
            if(isNumber(commands.at(i + 1))) {
                if(eval(commands.at(i + 1)) > 0) {
                    i++;
                }else{
                    i++;
                    while(true) {
                        if(commands.at(i).substr(0,4).compare("ELSE") == 0 && to_int(commands.at(i).substr(5)) == level) {
                            break;
                        }
                        if(commands.at(i).substr(0,3).compare("END") == 0 && to_int(commands.at(i).substr(4)) == level) {
                            break;
                        }
                        i++;
                    }
                    i++;
                }
            }
        }else if(commands.at(i).substr(0,4).compare("ELSE") == 0) {
            int level = to_int(commands.at(i).substr(5));
            while(true) {
                if(commands.at(i).substr(0,3).compare("END") == 0 && to_int(commands.at(i).substr(4)) == level) {
                    break;
                }
                i++;
            }
            i++;
        }else if(commands.at(i).substr(0,7).compare("POINTER") == 0) {
            if(commands.at(i + 1).compare("EQUALS") == 0) {
                string innerPointer = commands.at(i).substr(0, commands.at(i).length() - 14).substr(8);
                int pointer = eval(innerPointer);
                setPointer(pointer, int_to_byte(eval(commands.at(i + 2))));
                i+=3;
            }
        }else{
            i++;
        }
    }
}

//gets the integer value from a binary in the form of brackets
int getBinaryValue(string* n) {
    string num = *n;
    int val = 0;
    for(u_int i = 0; i < num.length(); i++) {
        if(num.at(i) == '[') {
            val += pow(2, (num.length() - i - 1));
        }
    }
    return val;
}

//removes unnecessary characters from the code
void cleanseCode(string* code)
{
    string newCode = "";
    string derefCode = *code;
    char toChar[derefCode.length() + 1];
    strcpy(toChar, derefCode.c_str());
    for(char ch : toChar) {
        if(ch == '|' || ch == 'l' || ch == '1' || ch == '[' || ch == ']') {
           newCode += ch;
        }
    }
    *code = newCode;
}

//gets the text in the file from the path of the code
string getCode(string path)
{
    fstream file(path.c_str());
    if(file == false) {
        cout << "FileNotFoundError" << endl;
        return "";
    }
    string line, code = "";
    while(!file.eof()) {
        file >> line;
        code.append(line);
    }

    return code;
}

//gets the username of the windows user
//only used in the getCode() function
string getUserName()
{
    char username[UNLEN + 1];
    DWORD buffersize = sizeof(username);
    GetUserName(username, &buffersize);
    return username;
}

//main function, uses c arguments
int main(int argc, char* argv[])
{
    //gets file to compile from arguments
    if(argc > 1) code = getCode(argv[1]);
    else cout << "Missing necessary arguments!" << endl;
    //gets the input
    if(argc > 2) input = argv[2];
    else input = "";
    //gets memory size, in cells
    if(argc > 3) memSize = to_int(argv[3]);
    else memSize = 16384;
    //gets the size of cells, in bytes
    if(argc > 4) cellSize = to_int(argv[4]);
    else cellSize = 2;

    //creates the byte buffer
    buffer = new byte[memSize];
    //allocates memory for the memory array; also initializes them to 0
    memory = (byte*)calloc(memSize, cellSize);

    tempStorage1 = new byte[memSize];
    tempStorage2 = new byte[memSize];
    tempStorage3 = new byte[memSize];

    tempStorage2[0] = 200;
    tempStorage2[1] = 23;
    tempStorage1[0] = 70;
    tempStorage1[1] = 100;

    subtractBytes(tempStorage1, tempStorage2);

    cout << (int)tempStorage3[0] << endl;
    cout << (int)tempStorage3[1] << endl;

    //removes any characters that are not l,1,[,], or | from the code. This allows for the use of comments in the source code
    cleanseCode(&code);

    //parses the code. creates an vector of strings that represent the different commands in the language
    parse(&code);

    //a function that lumps expressions and pointers together, as well as organizing if statements
    tidyUp(&commands);

    //runs through the finalized command vector and runs the functions
    execute(&commands);

    return 0;
}

/*
 * Byte array operations: addition, subtraction, multiplication, division, modulus
 * No operations return anything, but modify the pointer tempStorage3
 */

int compare(byte* bArray1, byte* bArray2) {
	for(int i = cellSize - 1; i > 0; i--) {
		if(bArray1[i] > bArray2[i]) return 1;
		if(bArray1[i] < bArray2[i]) return -1
	}
	return 0;
}

void addBytes(byte* bArray1, byte* bArray2) {
    byte carry = 0;
    int sum;
    for(int i = 0; i < cellSize; i++) {
        sum = *(bArray1 + i) + *(bArray2 + i) + carry;
        tempStorage3[i] = sum;
        carry = sum - tempStorage3[i];
    }
}

void subtractBytes(byte* bArray1, byte* bArray2) {
    int sub;
    byte takeAway = 0;
    for(int i = 0; i < cellSize; i++) {
        sub = *(bArray1 + i) - *(bArray2 + i) + takeAway;
        tempStorage3[i] = sub;
        takeAway = (sub < 0) ? -1 : 0;
    }
}

void multiplyBytes(byte* bArray1, byte* bArray2) {
    int carry = 0;
    for(int i = 0; i < cellSize; i++) {
		int product = *(bArray + i) * *(bArray2 + i) + carry;
		carry = product - (product % 256);
    	tempStorage3[i] = *(bArray + i) * *(bArray2 + i) + ((carry > 0) ? carry : 0);
    }
}

void divideBytes(byte* bArray1, byte* bArray2) {
	int dividend = 0;
	tempStorage3 = *(bArray1)
	while(compare(tempStorage3, bArray2) == 1) {
		dividend++;
		subtractBytes(tempStorage3, bArray2);
	}
	tempStorage3 = int_to_byte(dividend);
}

void moduloBytes(byte* bArray1, byte* bArray2) {
	divideBytes(bArray1, bArray2);
	subtractBytes(bArray1, multiplyBytes(bArray2, tempStorage3));
}

//function purely for testing and debugging
void printVector(vector<string>* v) {
    vector<string> printMe = *v;

    for(string i : printMe) cout << i << endl;
}

//function returns the address of the associated pointer
byte* getPointer(int position) {
    for(unsigned int i = 0; i < cellSize; i++) {
        buffer[i] = memory[(position * cellSize) + i];
    }
    return buffer;
}

//sets the pointer to the new value
void setPointer(int position, byte* newPointer) {
    for(unsigned int i = 0; i < cellSize; i++) {
        memory[(position * cellSize) + i] = *(newPointer + i);
    }
}

//turns a string to int because i couldnt get stoi to work :/
int to_int(string s) {
    int iss;
    istringstream (s) >> iss;
    return iss;
}

long long to_llong(string s) {
    long long iss;
    istringstream (s) >> iss;
    return iss;
}

//turns an integer to a string. same story as above
string to_string(int i)
{
    stringstream ss;
    ss << i;
    return ss.str();
}

//uses a stringstream to split input string by whitespace
void removeWhitespace(string s, vector<string>* storage) {
    string buf;
    stringstream ss(s);

    vector<string> temp;

    while (ss >> buf)
        temp.push_back(buf);
    *storage = temp;
}

//converts integer to byte array with a size of the cellSize
byte* int_to_byte(int input) {
    for(int i = 0; i < cellSize; i++) {
        buffer[i] = (input >> (8 * i)) & 0xFF;
    }
    return buffer;
}

byte* llong_to_byte(long long input) {
	for(int i = 0; i < cellSize; i++) {
		buffer[i] = (input >> (8 * i)) & 0xFF;
	}
	return buffer;
}

//returns the integer equivalent of a byte array value
int byte_to_int(byte* input) {
    int val = 0;
    for(int i = 0; i < cellSize; i++) val += (*(input + i)) * pow(256, i);
    return val;
}
