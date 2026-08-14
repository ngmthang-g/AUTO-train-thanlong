@echo off
setlocal
cd /d "%~dp0"
where zig >nul 2>nul
if errorlevel 1 (
  echo KHONG TIM THAY ZIG TRONG PATH
  exit /b 1
)
if not exist dist mkdir dist
del /q dist\app.res dist\ThanLongAutoTrain_v*.exe >nul 2>nul
pushd resources
zig rc /c 65001 /fo ..\dist\app.res app.rc
popd
if errorlevel 1 exit /b 1
zig c++ -target x86_64-windows-gnu -O2 -std=c++17 -Wall -Wextra -Werror -municode -static -s ^
  src\main.cpp src\remote_worker.S dist\app.res ^
  -Wl,--subsystem,windows -lcomctl32 -luser32 -lkernel32 -lgdi32 ^
  -o dist\ThanLongAutoTrain_v0.8.5.exe
if errorlevel 1 exit /b 1
echo BUILD THANH CONG: dist\ThanLongAutoTrain_v0.8.5.exe
