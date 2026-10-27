@echo off
setlocal
echo ========================================================
echo   Building Steam Clone Engine (Visual Studio 2022)
echo ========================================================

set "VS_PATH=C:\Program Files\Microsoft Visual Studio\2022\Community"
set "VCVARS=%VS_PATH%\VC\Auxiliary\Build\vcvars64.bat"
set "CMAKE_BIN=%VS_PATH%\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe"

if not exist "%VCVARS%" (
    echo [ERROR] Visual Studio 2022 vcvars64.bat not found at %VCVARS%
    exit /b 1
)

if not exist "%CMAKE_BIN%" (
    echo [ERROR] CMake not found at %CMAKE_BIN%
    exit /b 1
)

echo [1/3] Initializing x64 Developer Environment...
call "%VCVARS%" >nul

echo [2/3] Configuring CMake project...
if not exist "build" mkdir build
"%CMAKE_BIN%" -S . -B build -G "Visual Studio 17 2022" -A x64

echo [3/3] Compiling Release binaries...
"%CMAKE_BIN%" --build build --config Release

echo.
echo ========================================================
echo   Build finished! Run 'run.bat' to launch the client.
echo ========================================================
