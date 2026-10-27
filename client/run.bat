@echo off
setlocal
if exist "build\bin\Release\steam_client.exe" (
    "build\bin\Release\steam_client.exe"
) else (
    echo [ERROR] Binary build\bin\Release\steam_client.exe not found!
    echo Please run build.bat first.
    pause
)
