@echo off

IF "%~1" == "" GOTO end
IF "%~1" == "-r" W:\Voxel.exe
IF "%~1" == "-d" devenv W:\Voxel.exe

:end
