# CNO-SKB-Work
Repository for all my SKB work.

**Linked List Lab**

Key Takeaways:
- Explicit memory management- outside of the heap, even managing the stack, making sure to keep ordering of code in mind
- Little bit of algorithms part - meeting time constraint and reversing a linkedlist in C, using two different pointers to make the swaps happen
- Making code a bit more robust with error handling for things such as NULL pointers

**Data Lab**

Key Takeaways:
- Playing around with bit level representation of ints and floats (numbers)
- Did most of this by hand to visualize it
- ! ~ & ^ | + << >> 
- Mathematical operations can be done by some of these operations, such as multiplying and diving by power by shifting
- MSB and LSB (for endianness)
- Skipped Floating Point, will re-visit

**Bomb Lab**

Key Takeaways:
- Setting breakpoints so damn bomb wouldn't explode and also to debug
- Inspecting registers and memory states using i r, x/d, etc
- Got decent with debugging using GDB (command line debugger tool)
- Understanding GDB - $ (constants), %registers, (%rbx) or 0x1c(%rax) for dereferencing or addressing mode
- Used commands such as r, disass, si, ni strings and objdump to view functions and strings used in code
- When observing the disassembled code, got familiar with what to look for, almost like a puzzle
- Mostly looked at the compare and jump statements prior to a call to explode bomb to see how to bypass the call of that function
- Getting familiar with x64 assembly overall, such as operand order, jump instructions

**Attack Lab**

Key Takeaways:
- Little Endian (deadbeef would be ef be ad de) and understanding address space length (8 bytes in this case with 00 representing one byte)
- Explot Security Vuln when protections arent put in place for buffer overflows
- Tricky part was that you could not circumvent validation code, you must incorporate attack strings such that when ret instruction ran, it would go to the address of injected code, touch func, or a gadget
- Used GDB again and OBJDump (objdump - d)
- Used Hex2raw to generate attack strings
- Used GCC as an assembler and objdump as a disassembler to grab byte code to pass into hex2raw to generate input string for instruction sequences
- First three parts were about code injection attacks (buffer overflow, not as useful anymore)
- This is where really understanding stack offsets and how the stack is created is important (function prologue especially -> stack pointer + saved registers + local variables + base pointer + return address + caller frame (all the stuff in function))
- Last two parts were about return oriented programming
- Use ROP to execute arbitrary code, useful when stack is non-executable and randomized from one run to another
- Used gadgets, string together to form injected code, identify byte sequences within an existing program that consist of one or more instructions followed by ret (finding c3 in code and using code prior to it to do cool shit, basically using bytes to make new code)
- Basically overflow buffer until the next thing written overwrites the old return address with the new gadget to start the gadget chain

**Echo Lab**

Key Takeaways:
- Explicit memory management- outside of the heap, even managing the stack, making sure to keep ordering of code in mind
- Little bit of algorithms part - meeting time constraint and reversing a linkedlist in C, using two different pointers to make the swaps happen
- Making code a bit more robust with error handling for things such as NULL pointers

**Transfer Lab**

Key Takeaways:
- Explicit memory management- outside of the heap, even managing the stack, making sure to keep ordering of code in mind
- Little bit of algorithms part - meeting time constraint and reversing a linkedlist in C, using two different pointers to make the swaps happen
- Making code a bit more robust with error handling for things such as NULL pointers

**GF-Lib Lab**

Key Takeaways:
- Explicit memory management- outside of the heap, even managing the stack, making sure to keep ordering of code in mind
- Little bit of algorithms part - meeting time constraint and reversing a linkedlist in C, using two different pointers to make the swaps happen
- Making code a bit more robust with error handling for things such as NULL pointers

**MTGF Lab**

Key Takeaways:
- Explicit memory management- outside of the heap, even managing the stack, making sure to keep ordering of code in mind
- Little bit of algorithms part - meeting time constraint and reversing a linkedlist in C, using two different pointers to make the swaps happen
- Making code a bit more robust with error handling for things such as NULL pointers
