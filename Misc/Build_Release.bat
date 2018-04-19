@echo off

mkdir W:\Builds\Release\ > nul 2> nul
pushd W:\Builds\Release\

set flags=-MD -nologo -fp:fast -Gm- -GR- -EHa- -Oi -W4 -wd4100 -wd4201 -wd4505 -FC -O2 -std:c++17 -arch:AVX -FeVoxel.exe
set defines=-D_CRT_SECURE_NO_WARNINGS=1 -D_HAS_EXCEPTIONS=0
set libs=-incremental:no -opt:ref winmm.lib gdi32.lib user32.lib Shell32.lib opengl32.lib glew32s.lib glfw3.lib noise.lib
set link=/LIBPATH:W:\Common\Lib /LTCG /SUBSYSTEM:WINDOWS /ENTRY:"mainCRTStartup"
set inc=-I W:\Common\Include

cl %inc% %flags% %defines% W:\Code\engine.cpp /link %libs% %link%

robocopy W:\Shaders\ W:\Builds\Release\Shaders\ /NJS /NJH /np /ndl /nfl
robocopy W:\Assets\ W:\Builds\Release\Assets\ /XC /XN /XO /NJS /NJH /np /ndl /nfl

popd
