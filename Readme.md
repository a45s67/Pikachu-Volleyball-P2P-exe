Pikachu Volleyball P2P (.exe version)
=======================


## How to use
1. Download "Pikachu-Volleyball-P2P.zip" and unzip it.
2. Excute "new_pika_IAT_mod.exe" and select 2P mode.
3. Set the ip and port 
4. Play

> ### Note  
> + I download the .exe of this game from internet, and modify it to "new_pika_IAT_mod.exe". 
I did not trace throughly of this binary, not sure if this file is safe =口=... 
( At least built-in Windows Defence didn't warn me )
> + "connect.dll" is the compiled output of project "connect" in "connect/" with Visual Studio.
This is writen by myself, I think it is safe(?)


## Description of some important source files
- *binary-modification/inject PE/insert.c* : 
  - change the characteristics of .rsrc section to RWX and expand it for new func, new IAT.
  (At first I considered to add a new section for functions, append new DLL exports to import table directly with PE Lord. But it said there was no space. :tear )
  - insert func shellcode, expand IAT for DLL injection
  - change the flow of code
  
- *binary modification/func shellcode/func_gen.py*
  - generate the functions to do something after hook
  
- *connect/connect.slm*
  - generate the connect.dll responsible for socket intialization, communication between users.
  
## Tools

- Dynamic reverse engineering
  - x32dbg
  - Cheat Engine
- Static disassembly
  - IDA pro 7.0 
  - Ghidra
- PE related
  - PE Lord
  - PE bear
  
- Shellcode
  - pwntools


## Future work
Add the function which can search the created server under the same LAN.

## References
- 加密與解密 第四版 
  - Chap.11 PE文件格式 
  - Chap.12 注入技術
  - Chap.22 補丁技術
  - chap.23 代碼的二次開發
- [gorisanson/pikachu-volleyball](https://github.com/gorisanson/pikachu-volleyball)

- [Getting Started with Winsock](https://docs.microsoft.com/en-us/windows/win32/winsock/getting-started-with-winsock)

- [Using Dialog Boxes](https://docs.microsoft.com/en-us/windows/win32/dlgbox/using-dialog-boxes)

- [Getting Started Reversing C++ Objects with Ghidra](https://www.youtube.com/watch?v=ir2B1trR0fE)