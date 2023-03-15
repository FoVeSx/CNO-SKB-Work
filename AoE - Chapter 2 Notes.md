# Chapter 0x200: Programming

**Notes:**
- Assembler: takes assembly language into machine-readable code
- Assembly language varies from architecture to architecture (Intel X86, Sparc, Arm)
- Compiler: converts high level language into machine language

**Questions:**

What is a variable?
- Simply thought of as an object that holds data that can be changed. (Not to be misconstrued with an OOP object)
- Variables that don’t change are constants

What is variable scope?
- Also referred to as context, in particular, the contexts of variables within functions

Local vs Static vs Global Scope
- Variables can have global scope, which persist across all functions.
- Variables that are local scope persist only in the function
- Static Variables are similar to global, use the static keyword, and remain intact between function calls, but they remain local within a particular function context, the initialization happens once, so even if “reinitialized”, it maintains the same value prior

What are variable types?
- Refers to the system used for declaring variables or functions of different types, the type of variable determines how much space it occupies in memory and how the bit pattern stored is implemented.
- Basic types, such char, int, float, double
- C99 added boolean type
- Void type
- Derived types, such as pointer types, array types, structure types, function types, etc.

What is a pointer?
- Similar to the EIP register, a pointer is a data type that points to a certain memory address. Useful as physical memory cannot actually be moved, and they can be defined and used like any other variable type

What can a pointer point to?
- Pointer points to something that is of the same data type, char pointer will point to somewhere in memory that is 1 byte in size, 
- Another example is for an array, a pointer can point to the index, and will point to the first index of the array
- Sizeo f pointer is dependent on machine (4 bytes for 32 bit, 8 bytes for 64 bit)

What is an array?
- Consider it a list, simply put
- Contains n elements of a specific data type
- Also referred to as a buffer

What is a string?
- There is not a string type, it is considered a char array in C.
- They are null terminated with ‘\0’
- There are string literals - sequence of characters enclosed in “”, also known as string constant

What is casting?
- A method of temporarily changing a variable’s data type, despite how it was originally defined. Typecasting only changes the datatype for that operation, and not the original variable
