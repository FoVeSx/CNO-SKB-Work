- PECOFF File Format
- Using WinDBG
- Process Execution Block (PEB) and Thread Execution - Block (TEB) data structures
- InLoadOrderModuleList (look at the PEB)

- write hello world application that prints hello world using print(f) with an empty import table

-  open your compiled binary with CFF Explorer and check the import section, which should be empty

- printf function can be found in Microsoft Visual C runtime, can determine the correct dll by compiling a hello world program that uses printf(), and use the Microsoft Dumpbin utility to see which dll the printf() function comes from: dumpbin /imports

- dont use any Windows syscalls directly, use WinDBG to help debug

compile using:
https://stackoverflow.com/questions/22875900/how-to-compile-c-windows-exe-without-import-table/41385038#41385038

windows coding conventions:
https://learn.microsoft.com/en-us/windows/win32/learnwin32/windows-coding-conventions

windows data types:
https://learn.microsoft.com/en-us/windows/win32/winprog/windows-data-types

understanding SAL (source code annotation lang):
https://learn.microsoft.com/en-us/cpp/code-quality/understanding-sal?view=msvc-170

WinDBG commands:
- bp (set a breakpoint at address)
    -   bp 0x401000
    -   bp kernel32!recv
- b1 (list breakpoints)
- be/bd (enable/disable breakpoints)
- .bpcmds (list breakpoint commands)
- dd (display memory at address)
    -   dd 0x7fabcd
    -   dd esp
    -   L## : unit count modifier- control how many  elements are displayed
            -   By default, dd will should 20
            -   dd esp L1 : view one element
- r (view register)

Breakpoint Docs:
https://learn.microsoft.com/en-us/windows-hardware/drivers/debugger/bp--bu--bm--set-breakpoint-

Display Memory Docs:
https://learn.microsoft.com/en-us/windows-hardware/drivers/debugger/d--da--db--dc--dd--dd--df--dp--dq--du--dw--dw--dyb--dyd--display-memor

--------------------------------------------------------------------------------------------------------------------------------------------------

PEB - Process Environment Block

The PEB is a user-mode data structure that can be used by application (and by extend by malware) to get information such as list of loaded modules, process startup arguments, heap address, check whether programs are being debugged, find image base address of imported DLLS.. etc

https://www.aldeid.com/wiki/PEB-Process-Environment-Block

1. (FS: 0x30) find the PEB
2. (PEB + 0XC) find the PEB_LDR_DATA structure
3. (reg + 0xC / 0x14/ 0x1C) walk the module linked lists
4. (reg + 0x24 / 0x2C or offset by 8)hash each module name
5. (reg + 0x18 or flag) return the image base or flag if target module is found

Kernel - program layer between userspace and H/W, program layer to translate operations between these two, every OS has a different kernel

Manages: CPU, R/W IO Devices, Scheduling, Memory, Security

