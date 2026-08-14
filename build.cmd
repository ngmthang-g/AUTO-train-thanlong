@echo off
setlocal
cd /d "%~dp0"
where zig >nul 2>nul
if errorlevel 1 (
  echo KHONG TIM THAY ZIG 0.15.2 TRONG PATH
  exit /b 1
)
if not exist dist mkdir dist
del /q dist\*.exe dist\*.dll dist\*.res >nul 2>nul
pushd resources
zig rc /c 65001 /fo ..\dist\app.res app.rc
popd
if errorlevel 1 exit /b 1

zig c++ -target x86_64-windows-gnu -O2 -std=c++17 -DUNICODE -D_UNICODE -Wall -Wextra -Werror -shared -static -s ^
  src\bridge\bridge.cpp ^
  -Wl,--out-implib,dist\libThanLongNewCoreBridge.a -luser32 -lkernel32 ^
  -o dist\ThanLongNewCoreBridge.dll
if errorlevel 1 exit /b 1

zig c++ -target x86_64-windows-gnu -O2 -std=c++17 -DUNICODE -D_UNICODE -Wall -Wextra -Werror -municode -static -s ^
  src\controller\main.cpp dist\app.res ^
  -Wl,--subsystem,windows -lcomctl32 -luser32 -lkernel32 -lgdi32 ^
  -o dist\ThanLongAutoTrain_NewCore_v1.0.0.exe
if errorlevel 1 exit /b 1

echo BUILD THANH CONG:
echo   dist\ThanLongAutoTrain_NewCore_v1.0.0.exe
echo   dist\ThanLongNewCoreBridge.dll
