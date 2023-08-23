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
- DEP and ASLR: data execution prevention (code being executed in memory/ stack) and address space layout randomisation (randomy offset of memory structures)
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
- Used BeeJ's guide, literal life saver
- Client send some bytes to server, server sends content back, and spits out onto stdout
- When looking at other examples, 90% of examples were using the old gethostbyname API, I utilized the getaddrinfo api (the new more robust cool kid way) to walk through all the possible DNS resolved ip addresses (there can be multiple ones for one lookup i.e. twitter.com)
- My debugging kinda sucked for this and the rest of the labs, just used the tester along w/ print statements to see what was going on

**Transfer Lab**

Key Takeaways:
- Used BeeJ's guide, literal life saver
- Properly implemented the fact that its possible that loss COULD be expected and to continously receieve data until the returned bytes was zero
- Used getaddrinfo api again
- Learned a little file I/O

**GF-Lib Lab**

Key Takeaways:
- Built off of transfer lab
- ALOT of string manipulation / functions for parsing, got familiar with string library
- Function Pointers and Callbacks and the purpose of them - its a good way to set arguments for a function without having to specify different functions for every type of need - for example, registering an argument to write_func to be writing to a file, but not necessarily doesnt have to be a file
- Main chunk was perform (for client) - to create the socket, receive info to header, and then use registered callback to export
- For server - somewhat similar, listen for a request, parse it until we receieve "\r\n\r\n", user registers a handler callback which controls how the request will be handled

**MTGF Lab**

Key Takeaways:
- Overthought this lab, and spent a bit too much time
- Was a bit easier, but just tried to spend time understanding conceptual things
- Threads share same virtual memory space
- Pthread join - joining with main thread to wait for thread(s) to finish executing until moving on
- Learned about Mutex locks and conditionals - needed to make MT program thread safe, along with protecting data, aka race conditions
- Main thread sends signal to worker threads whether to pop from queue if queue is empty 
- Understanding Queue (wasn't trivial, very basic push and pop / enqueue and pop)