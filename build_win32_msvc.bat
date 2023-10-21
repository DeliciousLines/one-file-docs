@echo off
cl /O2 /TC /Feofd.exe entry_point.c
del *.obj > NUL 2> NUL