# Lab2 BombLab

### Before starting 
The video from `recitation 04` is very helpful. You can start working on this lab after watching `Machine Prog: Control`, but may want to finish the lecture of `Machine Prog: Data` to finish the entire lab.  


### Tips
1. You can create a disassembly listing with `objdump -d bomb > disassembled.s`.  
2. You can dump symbols with `objdump -t bomb > symbolTable`.  
3. You can dump strings with `strings bomb > printableStrings.txt`.  

- Some useful gdb references:

`x` command: http://visualgdb.com/gdbreference/commands/x  
`print` command: http://visualgdb.com/gdbreference/commands/print  


### Some Reverse Engineering Pointers
- Get a"feel" for the data the function is operating on
  - https://margin.re/2021/11/an-opinionated-guide-on-how-to-reverse-engineer-software-part-1/ 
- Determine the functions inputs / outputs 

- [Getting a feel for a program](https://youtu.be/GbG7zoJn-EE?t=1777)
  - The whole video is worth watching just for getting a sense of RE interviews in general
  - It is extremely unlikely you will have to interview at this level of detail, but there are some good pointers here all the same
