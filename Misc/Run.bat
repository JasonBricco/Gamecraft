@echo off

IF "%~1" == "" GOTO end
IF "%~1" == "-r" W:\Builds\Release\Voxel.exe
IF "%~1" == "-d" devenv W:\Builds\Debug\Voxel.exe

:end
