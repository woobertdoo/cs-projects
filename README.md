## What is this repo for?
This is a repository featuring all the projects I have made while getting my computer science degree. 
 
# The Projects
## CMP SCI 2250: Programming and Data Structures

### pig-latin
A basic console application that takes an input string from the user and converts it to "pig latin".
Ex: User inputs "I am a student", output is "Iway amay away tudentsay."

### array-war
This program takes two int arrays of equal length. For each index in the arrays, it checks which array has the larger element
in that index and appends that to a new array. The resulting array will have the larger element from either array for each respective index

### delimiter-checker
A console application that takes a snippet of code and checks for matching braces.

### horse-racing-sim
A basic racing simulator

## CMP SCI 2261: Object-Oriented Programming

### smart-parking
A console-based application meant to mimic a smart parking lot application. It features aspecsts of object-oriented design like classes, interfaces, and polymorphism.

## CMP SCI 2750: Linux Environment and System Programming

### character-count
A command line tool that will take a given input file, convert it to either all caps or all lowercase, and count occurences of a user-provided character

### calcTest
A basic addition program. This program highlights using multiple files, unit testing, and makefiles.

### prime-sums
This program takes an input array and outputs all the pairs of prime numbers that have a sum less than 50. This program highlights usage of process forking.

### shm-sum
This program takes an input array and outputs the first pair of numbers that has a sum of 19. This program highlights usage of shared memory between processes.

## CMP SCI 3010: Full Stack Web Development

### project-1
The first of a sequence of projects in creating a full web application. This iteration includes HTML and CSS.

### dota-quiz-react/dota-quiz-server
These two projects work in tandem to create a full stack version of project-1. This application uses react for the front end and Node.JS, Express, and PostgresSQL 


## CMP SCI 3130: Design and Analysis of Algorithms

### polynomial-evaluation
This program is made to compare the efficiencies of three different polynomial evaluation algorithms: the brute force method, Horner's method (factorization), and the repeated squares method.
It also has elements of file i/o.

## CMP SCI 4280: Program Translation Project

### Language Definition
Valid characters are ! # + $ & * ' " ( ), any uppercase or lowercase letters, and any digits
t1 tokens are single characters, which can be ! # $ & ' "
t2 tokens are + followed by any number of digits, ex: +134 +045 +23
t3 tokens are an uppercase or lowercase letter followed by any number of digits, ex: j14 k149 K103 N482
* are used to delimit comments

### Language Grammar
S -> A ( B B )
A -> " t2 | empty
B -> S | C | D | E | G
C -> # t2 | ! F
D -> $ F
E -> ' F F F B
F -> t2 | t3 | & F F
G -> t2 % F

### Grammar Semantics
Identifiers: **t2** tokens are identifiers
Numbers: **t3** tokens are integers
- Leading uppercase indicates positive integer
- Leading lowercase indicates negative integer
- Any number of leading zeros are allowed
Reserved: **t1** tokens are reserved for operations
- **!** negates the following argument. If the argument is a **t2**, the new value is stored back in memory.
- **"** allocates memory for an identifier and sets its value to 0
- **#** allocates memory for an identifier and gets its value from user input
- **$** prints the value of the identifier or integer supplied
- **'** is a loop function. It has the form `' val1 val2 n op`. If `val1` is strictly greater than `val2`, do `op` `n` times, unless `n` < 1, then do nothing
- **%** is for assignment. For `iden1 % val1`, assign the value of `val1` to `iden1`
- **&** is *preorder* addition. For example, `& val1 val2` adds the values of `val1` and `val2`. Note that this **does not** overwrite any values on its own.

### UMSL's Assembly Code
UMSL's assembly code usually stores values into the ACCumulator, and most operations are single argument.
Arguments can be labels, integers, or variable names. Variables can only store integers
**Instructions**  **Number of Args and Instruction Explainer**
BR                  1 arg, jump to label `arg` 
BRNEG               1 arg, jump to label `arg` if ACC < 0
BRZNEG              1 arg, jump to label `arg` if ACC <= 0
BRPOS               1 arg, jump to label `arg` if ACC > 0
BRZPOS              1 arg, jump to label `arg` if ACC >= 0
BRZERO              1 arg, jump to label `arg` if ACC == 0
COPY                2 args, `arg1` = `arg2`
ADD                 1 arg, ACC = ACC + `arg`
SUB                 1 arg, ACC = ACC - `arg`
DIV                 1 arg, ACC = floor(ACC / `arg`)
MULT                1 arg, ACC = ACC * `arg`
READ                1 arg, `arg1` = `input`
WRITE               1 arg, `output` = `arg1`
STOP                0 args, stops the program
STORE               1 arg, `arg` = ACC
LOAD                1 arg, ACC = `arg`
NOOP                0 args, do nothing (only useful for writing labels without having instruction on same line)

### project-p0
The first project in a set of projects used to make a simple compiler. This one takes text input and organizes it into a basic binary tree.

### project-p1
The first part of the compiler proper. This project reads input from a text file or from the console, and splits the input into tokens based on the language definition

### project-p2
The parser for our compiler. Using the given grammar, we read the tokens from our scanner to create a parse tree.

### project-p3
A static sematics symbol table generator, to make sure that any variables that are accessed have already been declared, and to prevent variables from being declared multiple times

### project-p4
The capstone compiler. This program takes everything made so far, and compiles the high-level language into UMSL's assembly code. 
