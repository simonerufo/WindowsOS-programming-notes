@echo off

set SRC=triangle.cpp

set OUT=triangle.exe

cl %SRC% /Fe:%OUT% /EHsc ^
    user32.lib gdi32.lib d3d11.lib D3DCompiler.lib

