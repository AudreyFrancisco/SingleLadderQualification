#HOW ADD_MODULE WORKS (assuming you have python3):
- Make .txt file containing the register mapping of the module, either of the form "REGISTER_NAME  address#" or "REGISTER_NAME = address#" (each line is a different address). WARNING: DO NOT INCLUDE A "NUM OF REGISTERS" "REGISTER", THIS WILL RUIN DumpConfig
- Add .txt file to working directory (one with add_module.py)
- Run "add_module.py modulename registerfile.txt" where modulename is the desired name of the module and registerfile.txt is the .txt file you just made
- This will create a header TRUv1modulename.h and source TRUv1modulename.cpp in RUv1Src (just a generic wishbone module) with the register mapping you provided (the only additional function is DumpConfig, which just couts the value of each register)
- These still need to be included in the build path if you wish to actually build
- You can add "make" (like "add_module.py modulename registerfile.txt make") if you want to make format of the new files

This script is NOT sophisticated, but it's a good starting point for the lazy like me.

MADE BY RHANNIGA LOOK MOM I MADE IT
