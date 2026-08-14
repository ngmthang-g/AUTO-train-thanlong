@echo off
setlocal EnableExtensions
cd /d "%~dp0"

where zig >nul 2>nul
if errorlevel 1 (
  echo KHONG TIM THAY ZIG TRONG PATH
  echo Yeu cau Zig 0.15.2.
  exit /b 1
)
for /f "delims=" %%V in ('zig version') do set "ZIG_VERSION=%%V"
echo Zig: %ZIG_VERSION%

if not exist dist mkdir dist
del /q dist\*.exe dist\*.dll dist\*.res dist\*.lib dist\*.a >nul 2>nul
del /q ThanLongNewCoreBridge.dll BridgeSelfTest.exe ThanLongAutoTrain_NewCore_v1.0.5.exe app.res >nul 2>nul

echo [1/5] Resource...
pushd resources
zig rc /c 65001 /fo ..\app.res app.rc
popd
if errorlevel 1 goto :fail

echo [2/5] Bridge DLL - minimal runtime...
rem Bridge intentionally has no STL containers/streams and no managed invocation.
rem Do NOT combine -static with -shared here; that combination produced a DLL
rem which built successfully on the previous revision but Windows rejected as Bad Image.
zig c++ -target x86_64-windows-gnu -O2 -std=c++17 -DUNICODE -D_UNICODE ^
  -Wall -Wextra -Werror -fno-exceptions -fno-rtti -shared ^
  src\bridge\bridge.cpp -luser32 -lkernel32 -o ThanLongNewCoreBridge.dll
if errorlevel 1 goto :fail
if not exist ThanLongNewCoreBridge.dll goto :fail

echo [3/5] Bridge LoadLibrary self-test...
rem Self-test intentionally uses C stdio only; avoid C++ wide-stdio namespace differences across Zig/MinGW.
zig c++ -target x86_64-windows-gnu -O2 -std=c++17 -DUNICODE -D_UNICODE ^
  -Wall -Wextra -Werror -municode -static ^
  src\bridge\selftest.cpp -lkernel32 -o BridgeSelfTest.exe
if errorlevel 1 goto :fail
BridgeSelfTest.exe "%CD%\ThanLongNewCoreBridge.dll"
if errorlevel 1 (
  echo.
  echo LOI: DLL vua build khong LoadLibrary duoc tren chinh Windows nay.
  echo DUNG BUILD tai day; khong chay controller voi DLL loi.
  goto :fail
)

echo [4/5] Controller EXE...
zig c++ -target x86_64-windows-gnu -O2 -std=c++17 -DUNICODE -D_UNICODE -Wall -Wextra -Werror -municode -static ^
  src\controller\main.cpp app.res ^
  -Wl,--subsystem,windows -lcomctl32 -luser32 -lkernel32 -lgdi32 ^
  -o ThanLongAutoTrain_NewCore_v1.0.5.exe
if errorlevel 1 goto :fail

echo [5/5] Package...
move /y ThanLongNewCoreBridge.dll dist\ThanLongNewCoreBridge.dll >nul
move /y ThanLongAutoTrain_NewCore_v1.0.5.exe dist\ThanLongAutoTrain_NewCore_v1.0.5.exe >nul
move /y app.res dist\app.res >nul
del /q BridgeSelfTest.exe >nul 2>nul

echo.
echo BUILD + LOADLIBRARY SELFTEST THANH CONG:
echo   dist\ThanLongAutoTrain_NewCore_v1.0.5.exe
echo   dist\ThanLongNewCoreBridge.dll
exit /b 0

:fail
echo.
echo BUILD/SELFTEST THAT BAI. KHONG DUNG DLL neu self-test khong PASS.
exit /b 1
