@echo off
chcp 65001 > nul
title D^&D Console RPG
if not exist game.exe (
    echo [ОШИБКА] Файл game.exe не найден! Сначала соберите проект через build.bat.
    pause
    exit /b 1
)
game.exe
pause
