@echo off

cl /nologo /Zi /I include /std:c11 quad.c /link user32.lib gdi32.lib opengl32.lib