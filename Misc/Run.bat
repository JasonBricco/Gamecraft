@echo off

IF "%~1" == "" GOTO end
IF "%~1" == "-r" W:\Gamecraft.exe
IF "%~1" == "-d" devenv W:\Gamecraft.exe

:end
