@echo off
set path=W:\Misc;%path%
subst W: C:\Users\jason\Documents\Projects\Gamecraft
call C:/"Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvarsall.bat" x64
cd "C:/Program Files\Sublime Text 3\"
subl.exe
cd C:\Users\jason\Documents\Projects\Gamecraft
