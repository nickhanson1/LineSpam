# LineSpam
Esoteric programming language!

Basically is a c++ program that compiles code that is in the text file provided into my personal language - LineSpam!

The language uses only a few commands made from only 5 characters. Most commands are standard, such as comparison operators, if statements, goto statements, and operations. The unique aspect of the language is its storage mechanic. The language stores values by numbers, and each number refers to a location on a byte array, much like a turing tape or disk storage. Operations can then be used inside the makeshift "pointers" to use arrays and other techniques that use varied location of storage. 

The syntax of the language is as follows:

#####NUMBERS#####

Numbers are whole numbers stored as binary numbers, where '[' is a 1 and ']' is a zero. For example:
  [][ is equivalent to five
  [[[[[ is equivalent to 63
The compiler automatically recognizes numbers and no other special notation needs to be used to identify them.

#####NUMERIC OPERATORS#####

The numeric operators are addition, subtraction, multiplication, division, and modulo. They are represented by the following commands:
  |] is addition.
  |[ is subtraction.
  |1 is multiplication.
  |l is integer division
  || is modular arithmetic
For example
  [][ |[ [[ is 5 - 3 and will return 2
  []]][ |l [] is 17 / 2. This would return 8.5, but it is truncated to 8
Two issues that may arise
  Division is integer division, and will not be rounded, but will be truncated to the lowest integer
  Subtraction can not create negative numbers, rather it will create an overflow

#####COMPARISON OPERATORS#####
  
There are four operators for comparisons. True is represented with a 1 and false is represented with a 0. The comparisons are as follows:
  111 is ==, or equivalence
  11| is !=, or not equivalent
  11] is greater than
  11[ is less than

#####POINTERS#####

Pointers are stored between 'l[' and 'l]'. For example:
  l[ [ l] is a pointer at the location "1".
  l[ [[]] l] is the pointer at the location "12".
Expressions can also be inside pointers. For example:
  l[ []][ || []] l] is the pointer at 9 modulo 4, aka position 1. This reduces to l[ [ l].
  l[ l[ ] l] |] [] l] is the pointer at the position equal to the value of the pointer at 0 plus 2. The value at pointer 0 is evaluated     and is added to 2 to get the position for the outer pointer.
To set a pointer, the initialize operator(ll) is used
  l[ ] l] ll [][ is the same as saying "set pointer 0 equal to 5"
In the last example a pointer was nested inside another. There is no limit to the amount of pointers that can be nested in each other.\

#####GOTO AND LABELS#####

Labels are areas of code that can be jumped to. They are represented by 
  l1 followed by any number
When a goto statement is called with that number, it will be jumped to. Gotos are called by
  l| followed by the label number
If the label number is not found, the line is skipped.

#####INPUT AND OUTPUT#####

Input is represented by 1]. When the 1] command is called, the string input(given by the user in the command line arguments) is read. THe leftmost character of the input is read, converted to a number according to its value in ascii format, then the leftmost character is removed from the input, so that the next time 1] is called the compiler will retrieve the next character in the string.
Output is represented by the 1[ command. It is quite simple compared to input. Output works by printing the number immediately following it in ascii form. For example
  1[ []]]]][ is print 65, which will print an 'A'
 
#####IF STATEMENTS#####

If statements begin with a 1|[ and end with 1|]. If the expression immediately following the if is 1 the statement is considered to be true. Otherwise(if the expression is zero) it is false. If the expression is true, the code between the if and the end(or an else) is ran. If it is false, it will either skip to the 1|] or, if there is an else statement(represented by a 1l) it will skip to that and run all the code between the else and the end statement. It is pretty much a standard if-else system. For example
  1|[ l[ ] l] 111 []
    1[ []]]]][
  1l
    1[ []]]][]
  1|]
  Is the same as
  if pointer 0 equals 2
    print A
  else
    print B
  end
Note that the if statemtn consideres not only 1, but anything greater than 1 to be true. Also else statments are optional.

#####COMMAND-LINE#####

Take the compiled version of the compiler, LineSpam.exe, and run it in the command line! It takes 4 arguments, but only requires one:

LineSpam.exe -f -i -c -m

Where:
  -f is the file location of the code you want to compile
  -i is the input for the program
  -c is the size of each cell, which is what pointers are stroed in, in bytes. By default it is 2
  -m is the amount of cells you want. By default it is 16,384. 
Note that -i, -c, and -m are not required. Also note that the amount of memory the program will automatically allocate is -c * -m, or the cell size times the memory size. By default it allocated 32,768 bytes, or 32kB, of RAM.


And thats it! Its my first real programming language, and was meant as more of an exercise than anything. Enjoy! My code shouldn'yt be awful, but try to improve on it if you really want to!
