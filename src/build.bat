@echo off
cl -D PRODUCTION=1 -D _WIN32=1 -D CONSOLE_COLORS=0 -D STRING_LENGTH=512 ltl_interpreter.cpp /O2
