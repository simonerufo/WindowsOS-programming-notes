@echo off
REM echo Compiling hellowin.c
cl /nologo hellowin.c user32.lib gdi32.lib winmm.lib
