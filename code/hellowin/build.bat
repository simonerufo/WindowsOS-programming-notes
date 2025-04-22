@echo off
echo Compiling hellowin.c
cl hellowin.c user32.lib gdi32.lib winmm.lib
pause