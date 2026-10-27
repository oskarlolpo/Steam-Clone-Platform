@echo off
chcp 65001 > nul
echo ========================================================
echo Сборка проекта: Консольная D^&D RPG (C++)
echo ========================================================

where cl >nul 2>nul
if %errorlevel% == 0 goto COMPILE_MSVC

set "VCVARS="
if exist "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" set "VCVARS=C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat"
if exist "C:\Program Files\Microsoft Visual Studio\2022\Professional\VC\Auxiliary\Build\vcvarsall.bat" set "VCVARS=C:\Program Files\Microsoft Visual Studio\2022\Professional\VC\Auxiliary\Build\vcvarsall.bat"
if exist "C:\Program Files\Microsoft Visual Studio\2022\Enterprise\VC\Auxiliary\Build\vcvarsall.bat" set "VCVARS=C:\Program Files\Microsoft Visual Studio\2022\Enterprise\VC\Auxiliary\Build\vcvarsall.bat"
if exist "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvarsall.bat" set "VCVARS=C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvarsall.bat"
if exist "C:\Program Files (x86)\Microsoft Visual Studio\2019\BuildTools\VC\Auxiliary\Build\vcvarsall.bat" set "VCVARS=C:\Program Files (x86)\Microsoft Visual Studio\2019\BuildTools\VC\Auxiliary\Build\vcvarsall.bat"

if defined VCVARS (
    echo Инициализация компилятора MSVC...
    call "%VCVARS%" x64 > nul
    goto COMPILE_MSVC
)

where g++ >nul 2>nul
if %errorlevel% == 0 (
    echo Сборка через g++...
    g++ -std=c++17 -finput-charset=UTF-8 -fexec-charset=UTF-8 -I. main.cpp -o game.exe
    if %errorlevel% == 0 (
        echo [УСПЕХ] game.exe успешно скомпилирован!
        goto END
    ) else (
        echo [ОШИБКА] Ошибка при сборке через g++!
        goto END
    )
)

echo [ОШИБКА] Компилятор C++ не найден! Установите Visual Studio или MinGW.
goto END

:COMPILE_MSVC
echo Сборка через MSVC cl.exe (C++17, UTF-8)...
cl.exe /nologo /EHsc /std:c++17 /utf-8 /I. main.cpp /Fe:game.exe
if %errorlevel% == 0 (
    if exist main.obj del main.obj
    echo.
    echo ========================================================
    echo [УСПЕХ] game.exe успешно скомпилирован!
    echo Для запуска игры запустите run.bat
    echo ========================================================
) else (
    echo.
    echo [ОШИБКА] Сборка завершилась с ошибкой!
)

:END
pause