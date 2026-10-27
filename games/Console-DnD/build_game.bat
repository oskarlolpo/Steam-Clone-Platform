@echo off
setlocal
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"
cd /d "C:\Users\govno\Desktop\zadaniy\ind\Console-DnD-main"
echo Compiling 64-bit native game.exe...
cl.exe /nologo /EHsc /std:c++17 /utf-8 /I. main.cpp /Fe:game.exe
if %errorlevel% == 0 (
    echo [SUCCESS] game.exe compiled successfully!
) else (
    echo [ERROR] Compilation failed!
)
