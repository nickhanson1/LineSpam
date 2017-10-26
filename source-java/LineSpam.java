package main;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.IOException;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.LinkedList;


public class LineSpam {
	
	private static String path = "";
	private static String code = "";
	private static String input = "";
	//Linked List that contains the commands of the code
	private static LinkedList<String> commands = new LinkedList<String>();
	//ArrayList that contains the labels in the code
	private static ArrayList<Label> labels = new ArrayList<Label>();
	//arraylist that contains the errors
	private static ArrayList<String> errors = new ArrayList<String>();
	
	//array that stores memory. automatically allocates around 270kB or so
	private static char[] memory;
	
	/*
	 * main function. Just acts as a centralized place to execute all the essential functions
	 * Arguments:
	 * 
	 * 1. File path
	 * 2. Input
	 * 3. bytes of memory allocation
	 */
	public static void main(String[] args) {
		
		try {
			path = args[0];
		}catch(NullPointerException|ArrayIndexOutOfBoundsException e) {
			System.out.println("Missing necessary arguments!");
			System.exit(0);
		}
		
		try {
			input = args[1];
		}catch(NullPointerException|ArrayIndexOutOfBoundsException e) {
			input = "";
		}
		
		try {
			if(args[2].toUpperCase().equals("D")) {
				memory = new char[16384];
			}else memory = new char[Integer.parseInt(args[2])];
		}catch (NullPointerException|ArrayIndexOutOfBoundsException e) {
			memory = new char[16384];
		}
			
		code = getCode(path);
		code = cleanseCode(code);
		
		parse(code);
		
		
		//finds and neatly packs all the pointers together
		commands = findPointers(commands);

		//finds and neatly packs all the numerical expressions together
		commands = findExpr(commands);
		//finds and neatly packs all the boolean expressions together
		
		commands = labelIf(commands);
		
		//printArray(commands);
		
		execute(commands);
	}
	
	//This function parses through the code and converts individual tokens to commands
	//pretty useful
	private static void parse(String code) {
		String tok = "";
		
		/*
		 * The state of the parser;
		 * 0 = normal
		 * 1 = number parsing
		 */
		byte state = 0;
		
		String currentCommand = "";
		
		//runs through each character of code and matches the commands
		for(char c : code.toCharArray()) {
			tok += c;
			if(state == 0) {
				if(tok.equals("l[")) {
					currentCommand = "POINTER";
					tok = "";
					commands.add(currentCommand);
					currentCommand = "";
				}else if(tok.equals("l]")) {
					currentCommand = "CLOSE_POINTER";
					tok = "";
					commands.add(currentCommand);
					currentCommand = "";
				}else if(tok.equals("ll")) {
					currentCommand = "EQUALS";
					tok = "";
					commands.add(currentCommand);
					currentCommand = "";
				}else if(tok.equals("l1")) {
					currentCommand = "LABEL";
					tok = "";
					commands.add(currentCommand);
					currentCommand = "";
				}else if(tok.equals("l|")) {
					currentCommand = "GOTO";
					tok = "";
					commands.add(currentCommand);
					currentCommand = "";
				}else if(tok.equals("|]")) {
					currentCommand = "ADD";
					commands.add(currentCommand);
					tok = "";
					currentCommand = "";
				}else if(tok.equals("|[")) {
					currentCommand = "SUBTRACT";
					commands.add(currentCommand);
					tok = "";
					currentCommand = "";
				}else if(tok.equals("|1")) {
					currentCommand = "MULTIPLY";
					commands.add(currentCommand);
					tok = "";
					currentCommand = "";
				}else if(tok.equals("|l")) {
					currentCommand = "DIVIDE";
					commands.add(currentCommand);
					tok = "";
					currentCommand = "";
				}else if(tok.equals("||")) {
					currentCommand = "MODULO";
					commands.add(currentCommand);
					tok = "";
					currentCommand = "";
				}else if(tok.equals("1]")) {
					currentCommand = "INPUT";
					commands.add(currentCommand);
					tok = "";
					currentCommand = "";
				}else if(tok.equals("1[")) {
					currentCommand = "PRINT";
					commands.add(currentCommand);
					tok = "";
					currentCommand = "";
				}else if(tok.equals("1|[")) {
					currentCommand = "IF";
					commands.add(currentCommand);
					tok = "";
					currentCommand = "";
				}else if(tok.equals("1|]")) {
					currentCommand = "END";
					commands.add(currentCommand);
					tok = "";
					currentCommand = "";
				}else if(tok.equals("1l")) {
					currentCommand = "ELSE";
					commands.add(currentCommand);
					tok = "";
					currentCommand = "";
				}else if(tok.equals("111")) {
					currentCommand = "COMPARE";
					commands.add(currentCommand);
					tok = "";
					currentCommand = "";
				}else if(tok.equals("11[")) {
					currentCommand = "LESS_THAN";
					commands.add(currentCommand);
					tok = "";
					currentCommand = "";
				}else if(tok.equals("11|")) {
					currentCommand = "NOT";
					commands.add(currentCommand);
					tok = "";
					currentCommand = "";
				}else if(tok.equals("11]")) {
					currentCommand = "GREATER_THAN";
					commands.add(currentCommand);
					tok = "";
					currentCommand = "";
				}else if(tok.equals("]") || tok.equals("[")) {
					currentCommand = "NUMBER:";
					state = 1;
				}
			}else{
				//parses numbers; if the last digit is not [ or ], then the number has ended and a new command has begun
				if(tok.substring(tok.length() - 1).equals("l") || tok.substring(tok.length() - 1).equals("|") || tok.substring(tok.length() - 1).equals("1")) {
					state = 0;
					//the current command becomes NUMBER: followed by the value of the string. that command is then added to the commands list
					currentCommand += getValue(tok.substring(0,tok.length() - 1));
					commands.add(currentCommand);
					currentCommand = "";
					tok = tok.substring(tok.length() - 1);
				}
			}	
		}
		
		//runs if the last command was a number
		if(currentCommand != "") commands.add(currentCommand + getValue(tok));
	}
	
	//This function looks through the list of commands, finds pointers, and collects the statements inside pointers for easier use later
	private static LinkedList<String> findPointers(LinkedList<String> commands) {
		int i = 0; 

	    LinkedList<String> newCommands = new LinkedList<String>();
		String newCommand = "";
		
		while(i < commands.size()) {
			if(commands.get(i).equals("POINTER")) {
				/*
				 * keeps track of how many pointer instances are "open" per se
				 * 
				 * example: pointer, pointer, close_pointer, close_pointer
				 * 
				 * the counter goes: 1, 2, 1, 0
				 */
				
				byte pointerCounter = 0;
				newCommand = "";
				do {
					if(commands.get(i).equals("POINTER")) {
						pointerCounter++;
					}else if(commands.get(i).equals("CLOSE_POINTER")) {
						pointerCounter--;
					}
					newCommand += commands.get(i) + " ";
					i++;
				} while(pointerCounter != 0);
				newCommands.add(newCommand);
			}else{
				newCommands.add(commands.get(i));
				i++;
			}
		}
		
		return newCommands;
	}
	
	private static LinkedList<String> labelIf(LinkedList<String> commands) {
		int i = 0; 
		int level = 0;
		
	    LinkedList<String> newCommands = commands;
		
		while(i < commands.size()) {
			if(newCommands.get(i).equals("IF")) {
				level++;
				newCommands.set(i, "IF:" + level);
			}else if(newCommands.get(i).equals("ELSE")) {
				newCommands.set(i, "ELSE:" + level);
			}else if(newCommands.get(i).equals("END")) {
				newCommands.set(i, "END:"+ level);
				level--;
			}
			i++;
		}
		return newCommands;
	}
	
	//searches through the list of commands
	//finds possible expressions, validates that they are in fact expressions, then clumps the individual commands together into an EXPR commands
	//This allows for easy computation of expressions later on down the road
	private static LinkedList<String> findExpr(LinkedList<String> commands) {
		int i = 0; 

	    LinkedList<String> newCommands = new LinkedList<String>();
		String newCommand = "";
		
		while(i < commands.size()) {
			try {
				//checks to see if the commands are an operation. Ugly, I know
				if(safeSubstring(commands.get(i),0,6).equals("NUMBER") || safeSubstring(commands.get(i),0,7).equals("POINTER") || commands.get(i).equals("INPUT")) {
					if(commands.get(i + 1).equals("ADD") || commands.get(i + 1).equals("SUBTRACT") || commands.get(i + 1).equals("MULTIPLY") || 
							commands.get(i + 1).equals("DIVIDE") || commands.get(i + 1).equals("MODULO")
							|| commands.get(i + 1).equals("COMPARE") || commands.get(i + 1).equals("LESS_THAN") || commands.get(i + 1).equals("GREATER_THAN") || commands.get(i + 1).equals("NOT")) {
						newCommand = "EXPR: ";
						newCommand += (commands.get(i) + " " + commands.get(i + 1) + " " + commands.get(i + 2));
						i+=3;
						while(true && i < commands.size() - 1) {
							if(commands.get(i).equals("ADD") || commands.get(i).equals("SUBTRACT") || commands.get(i).equals("MULTIPLY") || commands.get(i).equals("DIVIDE") 
									|| commands.get(i).equals("COMPARE") || commands.get(i + 1).equals("LESS_THAN") || commands.get(i + 1).equals("GREATER_THAN")|| commands.get(i + 1).equals("NOT")) {
								newCommand += " " + commands.get(i) + " " + commands.get(i + 1);
								i+=2;
							}else{
								break;
							}
						}
						newCommands.add(newCommand);
						newCommand = "";
					}else{
						newCommands.add(commands.get(i));
						i++;
					}
				}else{
					newCommands.add(commands.get(i));
					i++;
				}
			}catch (IndexOutOfBoundsException e) {
				try {
					newCommands.add(commands.get(i));
					i++;
				} catch(IndexOutOfBoundsException e1) {
					newCommands.add(newCommand);
				}
			}
		}
		return newCommands;
	}
	
	//Lights, Camera
	//ACTION!
	private static void execute(LinkedList<String> psuedo) throws StringIndexOutOfBoundsException{
		//Creates LABELS, which are parameters for GOTO statements
		//Runs before the rest of the program does
		//Adds each new label to the list of LABELS for use when GOTO statements are called later in the program
		for(int j = 0; j < psuedo.size(); j++) {
			if(psuedo.get(j).equals("LABEL")) {
				Label temp = new LineSpam.Label();
				if(isNum(psuedo.get(j + 1))) {
					temp.name = eval(psuedo.get(j + 1));
					temp.location = j;
					labels.add(temp);
				}else{
					errors.add("Label declaration at " + j + " is missing the proper arguments!");
					System.out.println("Label declaration at " + j + " is missing the proper arguments!");
				}
			}
		}
		
		int i = 0;
		
		//Program Execution
		while(i < psuedo.size()) {
			//Executes GOTO function
			//
			//GOTO: NUMBER or POINTER
			if(psuedo.get(i).length() >= 4 && psuedo.get(i).substring(0,4).equals("GOTO")) {
				if(isNum(psuedo.get(i + 1))) {
					int newLocation = getLabelLocation(eval(psuedo.get(i + 1)));
					if(newLocation != -1) {
						i = newLocation;
					}else{
						errors.add("Label " + psuedo.get(i).substring(5) + " does not exist!");
						i++;
					}
				}else{
					errors.add("GOTO command at " + i + " is missing the proper arguments!");
					System.out.println("GOTO command at " + i + " is missing the proper arguments!");
				}
				
				
			//executes PRINT function
			//
			//PRINT: NUMBER or POINTER
			}else if(psuedo.get(i).length() >= 5 && psuedo.get(i).equals("PRINT")) {
				if(isNum(psuedo.get(i + 1))) {
					System.out.print((char)(eval(psuedo.get(i + 1))));
					i+=2;
				}else{
					errors.add("Print function at " + i + " is missing proper arguments!");
					System.out.println("Print function at " + i + " is missing proper arguments!");
					i++;
				}
				
			//executed functions that may follow POINTER
			//
			//POINTER: EQUALS
			//EQUALS : POINTER or NUMBER or EXPRESSION
			}else if((psuedo.get(i).length() >= 7 && psuedo.get(i).substring(0, 7).equals("POINTER"))) {
				if(psuedo.get(i + 1).equals("EQUALS")) {
					String[] splitPtr = psuedo.get(i).split(" ");
					String temp = "";
					for(int j = 1; j < splitPtr.length - 1; j++) {
						temp += " " + splitPtr[j];
					}
					int id = eval(temp); 
					if(isNum(psuedo.get(i + 2))) {
						int value = eval(psuedo.get(i + 2));
						addPointer(id, (char)value);
						i+=3;
					}else{
						errors.add("Declaration at " + i + " is missing arguments!");
						System.out.println("Declaration at " + i + " is missing arguments!");
						i++;
					}
					
				}
			//IF statements
			}else if(psuedo.get(i).substring(0, 2).equals("IF")) {
				int level = Integer.parseInt(psuedo.get(i).substring(3));
				if(isNum(psuedo.get(i + 1))) {
					if(eval(psuedo.get(i + 1)) > 0) {
						i++;
					}else if(eval(psuedo.get(i + 1)) == 0) {
						i++;
						while(true) {
							if(safeSubstring(psuedo.get(i), 0, 4).equals("ELSE") && Integer.parseInt(psuedo.get(i).substring(5)) == level) {
								break;
							}
							if(psuedo.get(i).substring(0, 3).equals("END") && Integer.parseInt(psuedo.get(i).substring(4)) == level) {
								break;
							}
							i++;
						}
						i++;
					
					}
				}
			}else if(safeSubstring(psuedo.get(i), 0, 4).equals("ELSE")) {
				int level = Integer.parseInt(psuedo.get(i).substring(5));
				i++;
				while(true) {
					if(psuedo.get(i).substring(0, 3).equals("END") && Integer.parseInt(psuedo.get(i).substring(4)) == level) break;
					i++;
				}
				i++;
			}else{
				i++;
			}
		
		}
	}
	
	//evaluates a pointer and the expressions inside one
	private static int evalPointer(String pointer) {
		LinkedList<String> tokens = new LinkedList<String>(Arrays.asList(pointer.split(" ")));
		
		
		tokens.removeFirstOccurrence("POINTER");
		tokens.removeLastOccurrence("CLOSE_POINTER");

		String temp = "";
		
		int i = 0;
		while(i < tokens.size()) {
			if(tokens.get(i).equals("POINTER")) {
				temp = "";
				temp += tokens.get(i) + " ";
				int startIndex = i, endIndex;
				int pointerCounter = 1;
				int tempCounter = i;
				while(pointerCounter != 0) {
					tempCounter++;
					if(tokens.get(tempCounter).equals("POINTER")) pointerCounter++;
					if(tokens.get(tempCounter).equals("CLOSE_POINTER")) pointerCounter--;
					temp += tokens.get(tempCounter) + " ";
				}
				
				endIndex = tempCounter;
				
				for(int j = startIndex; j < endIndex + 1; j++) {
					tokens.remove(startIndex);
				}
				
				tokens.add(startIndex, "NUMBER:" + evalPointer(temp));
				
			}
			i++;
		}
		
		String expr = "";
		for(int j = 0; j < tokens.size(); j++) {
			expr += tokens.get(j) + " ";
		}
		
		int insidePtr = eval(expr);
		
		return getPointerValue(insidePtr);
	}
	
	//evaluates expressions
	private static int eval(String expr) {
		LinkedList<String> ops = new LinkedList<String>(Arrays.asList(expr.split(" ")));
		ops.remove("EXPR:");
		ops = findPointers(ops);
		
		ops.remove("");
		
		int i = 0;
		
		int size = ops.size();
		
		while(i < size) {
			if(ops.get(i).equals("MULTIPLY")) {
				int value1 = findValue(ops.get(i - 1));
				int value2 = findValue(ops.get(i + 1));
				int value3 = value1 * value2;
				ops.set(i, value3 + "");
				ops.remove(i + 1);
				ops.remove(i - 1);
			}else if(ops.get(i).equals("DIVIDE")) {
				int value1 = findValue(ops.get(i - 1));
				int value2 = findValue(ops.get(i + 1));
				int value3 = (int)(value1 / value2);
				ops.set(i, value3 + "");
				ops.remove(i + 1);
				ops.remove(i - 1);
			}else if(ops.get(i).equals("MODULO")) {
				int value1 = findValue(ops.get(i - 1));
				int value2 = findValue(ops.get(i + 1));
				int value3 = value1 % value2;
				ops.set(i, value3 + "");
				ops.remove(i + 1);
				ops.remove(i - 1);
			}
			i++;
			size = ops.size();
		}
		
		i = 0;
		
		while(i < size) {
			if(ops.get(i).equals("ADD")) {
				int value1 = findValue(ops.get(i - 1));
				int value2 = findValue(ops.get(i + 1));
				int value3 = value1 + value2;
				ops.set(i, value3 + "");
				ops.remove(i + 1);
				ops.remove(i - 1);
			}else if(ops.get(i).equals("SUBTRACT")) {
				int value1 = findValue(ops.get(i - 1));
				int value2 = findValue(ops.get(i + 1));
				int value3 = value1 - value2;
				ops.set(i, value3 + "");
				ops.remove(i + 1);
				ops.remove(i - 1);
			}
			i++;
			size = ops.size();
		}
		
		i = 0;
		while(i < size) {
			if(ops.get(i).equals("COMPARE")) {
				int value1 = findValue(ops.get(i - 1));
				int value2 = findValue(ops.get(i + 1));
				int value3 = 0;
				if(value1 == value2) value3 = 1;
				ops.set(i, value3 + "");
				ops.remove(i + 1);
				ops.remove(i - 1);
			}else if(ops.get(i).equals("GREATER_THAN")) {
				int value1 = findValue(ops.get(i - 1));
				int value2 = findValue(ops.get(i + 1));
				int value3 = 0;
				if(value1 > value2) value3 = 1;
				ops.set(i, value3 + "");
				ops.remove(i + 1);
				ops.remove(i - 1);
			}else if(ops.get(i).equals("LESS_THAN")) {
				int value1 = findValue(ops.get(i - 1));
				int value2 = findValue(ops.get(i + 1));
				int value3 = 0;
				if(value1 < value2) value3 = 1;
				ops.set(i, value3 + "");
				ops.remove(i + 1);
				ops.remove(i - 1);
			}else if(ops.get(i).equals("NOT")) {
				int value1 = findValue(ops.get(i - 1));
				int value2 = findValue(ops.get(i + 1));
				int value3 = 0;
				
				if(value1 != value2) value3 = 1;
				
				ops.set(i, value3 + "");
				ops.remove(i + 1);
				ops.remove(i - 1);
			}
			i++;
			size = ops.size();
		}
		
		return findValue(ops.get(0));
		
	}
	
	//finds values given a NUMBER, POINTER, or INPUT. Used only in the eval() function
	private static int findValue(String val) {
		if(safeSubstring(val,0,6).equals("NUMBER")) {
			return Integer.parseInt(val.substring(7));
		}else if(safeSubstring(val,0,7).equals("POINTER")) {
			return evalPointer(val);
		}else if(val.equals("INPUT")) {
			return getInput();
		}
		return Integer.parseInt(val);
	}
	
	private static char getInput() {
		try {
				char ch = input.charAt(0);
				input = input.substring(1);
				return ch;
		}catch(NullPointerException|StringIndexOutOfBoundsException e) {
			return 0;
		}
	}
	
	//determines whether a command is a NUMBER, EXPR, or POINTER
	//This method is useful because I was lazy and didnt want to copy/paste all the logic expressions over and over
	private static boolean isNum(String s) {
		if(safeSubstring(s,0,4).equals("EXPR") || safeSubstring(s,0,6).equals("NUMBER") || safeSubstring(s,0,7).equals("POINTER") || s.equals("INPUT")) return true;
		return false;
	}
	
	//adds a new Pointer 
	private static void addPointer(int id, char value) {
		memory[id] = value;
	}
	
	//returns the value of a pointer
	public static int getPointerValue(int pointer) {
		return memory[pointer];
	}
	
	//gets substring while avoiding StringIndexOutOfBounds exceptions
	private static String safeSubstring(String s, int a, int b) {
		try {
			return s.substring(a, b);
		} catch (Exception e) {
			return "";
		}
	}
	
	//gets substring while avoiding StringIndexOutOfBounds exceptions
	private static String safeSubstring(String s, int a) {
		try {
			return s.substring(a);
		} catch (Exception e) {
			return "";
		}
	}
	
	//gets the Value of a number, from binary([ and ]) to decimal
	private static short getValue(String s) {
		int length = s.length();
		short value = 0;
		for(int i = 0; i < length; i++) {
			if(s.charAt(i) == '[') value += Math.pow(2, length - i - 1);
		}
		return value;
	}
	
	public static int getLabelLocation(int name) {
		for(Label l : labels) {
			if(l.name == name) return l.location;
		}
		return -1;
	}
	
	public static class Command {
		public String command;
		public short arg;
	}
	
	static class Label {
		public int name;
		public int location;
	}
	
	private static String getLastThreeChars(String s) {
		try {	
			return s.substring(s.length() - 3, s.length());
		}catch(StringIndexOutOfBoundsException e) {
			return s;
		}
	}
	
	private static String getLastTwoChars(String s) {
		try {	
			return s.substring(s.length() - 2, s.length());
		}catch(StringIndexOutOfBoundsException e) {
			return s;
		}
	}
	
	private static String removeLastTwoChars(String s) {
		try {	
			return s.substring(0, s.length() - 2);
		}catch(StringIndexOutOfBoundsException e) {
			return s;
		}
	}
	
	//getting the code stuff
	private static String getCode(String path) {
		File file = new File(path);
		try {
			BufferedReader br = new BufferedReader(new FileReader(file));
			String line = "";
			while(line != null) {
				code += line;
				line = br.readLine();
			}
			return code;
		} catch (FileNotFoundException e) {
			System.out.println("File not found! Now Exiting...");
			System.exit(0);
		} catch (IOException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
			System.exit(0);
		}
		return null;
	}
	
	private static String cleanseCode(String code) {
		String newCode = "";
		for(char c : code.toCharArray()) {
			if(c == 'l' || c == '1' || c == ']' || c == '[' || c == '|') {
				newCode += c;
			}
		}
		return newCode;
	}
	
}
