@echo off
del *.pdb
del *.obj
cl -D PRODUCTION=1 -D _WIN32=1 -D CONSOLE_COLORS=1 -D STRING_LENGTH=512 ltl_interpreter.cpp /Zi /O2
rem cl -D _WIN32=1 -D CONSOLE_COLORS=0 ltl_interpreter.cpp /Zi /MT /EHsc /Oy- /Ob0 /O2
