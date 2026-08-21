@echo off
setlocal EnableExtensions
cd /d "%~dp0"

where zig >nul 2>nul
if errorlevel 1 (
  echo KHONG TIM THAY ZIG TRONG PATH
  echo Yeu cau Zig 0.15.2.
  exit /b 1
)

if not exist dist mkdir dist
set "OUT=dist\ThanLongHiddenAutoChat_TEST.exe"
del /q "%OUT%" >nul 2>nul

echo Building standalone hidden auto-chat TEST...
zig c++ -target x86_64-windows-gnu -O2 -std=c++17 -DUNICODE -D_UNICODE ^
  -Wall -Wextra -Werror -municode -static ^
  src\chat_test\main.cpp ^
  -Wl,--subsystem,windows -lcomctl32 -luser32 -lkernel32 -lgdi32 ^
  -o "%OUT%"
if errorlevel 1 goto :fail
if not exist "%OUT%" goto :fail

echo BUILD PASS: %OUT%
exit /b 0

:fail
echo BUILD FAILED
exit /b 1
