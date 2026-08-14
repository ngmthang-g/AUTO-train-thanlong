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
if /I not "%ZIG_VERSION%"=="0.15.2" (
  echo CANH BAO: source nay duoc kiem tra cho Zig 0.15.2.
)

if not exist dist mkdir dist
del /q dist\*.exe dist\*.dll dist\*.res dist\*.lib dist\*.a >nul 2>nul
del /q ThanLongNewCoreBridge.dll ThanLongNewCoreBridge.lib libThanLongNewCoreBridge.a ThanLongAutoTrain_NewCore_v1.0.1.exe app.res >nul 2>nul

echo [1/3] Resource...
pushd resources
zig rc /c 65001 /fo ..\app.res app.rc
popd
if errorlevel 1 goto :fail

echo [2/3] Bridge DLL...
rem IMPORTANT:
rem - Do NOT request --out-implib. The controller uses LoadLibrary/GetProcAddress,
rem   so an import library is unnecessary.
rem - Build the DLL in the project root first, then move it to dist. This also
rem   avoids Windows-DLL/subdirectory import-lib edge cases in Zig 0.15.x.
zig c++ -target x86_64-windows-gnu -O2 -std=c++17 -DUNICODE -D_UNICODE -Wall -Wextra -Werror -shared -static -s ^
  src\bridge\bridge.cpp ^
  -luser32 -lkernel32 ^
  -o ThanLongNewCoreBridge.dll
if errorlevel 1 goto :fail
if not exist ThanLongNewCoreBridge.dll (
  echo LOI: compiler khong tao ThanLongNewCoreBridge.dll
  goto :fail
)
move /y ThanLongNewCoreBridge.dll dist\ThanLongNewCoreBridge.dll >nul
rem Zig/LLD may emit an unused import library next to the DLL; remove it.
del /q ThanLongNewCoreBridge.lib libThanLongNewCoreBridge.a >nul 2>nul

echo [3/3] Controller EXE...
zig c++ -target x86_64-windows-gnu -O2 -std=c++17 -DUNICODE -D_UNICODE -Wall -Wextra -Werror -municode -static -s ^
  src\controller\main.cpp app.res ^
  -Wl,--subsystem,windows -lcomctl32 -luser32 -lkernel32 -lgdi32 ^
  -o ThanLongAutoTrain_NewCore_v1.0.1.exe
if errorlevel 1 goto :fail
if not exist ThanLongAutoTrain_NewCore_v1.0.1.exe (
  echo LOI: compiler khong tao EXE
  goto :fail
)
move /y ThanLongAutoTrain_NewCore_v1.0.1.exe dist\ThanLongAutoTrain_NewCore_v1.0.1.exe >nul
move /y app.res dist\app.res >nul

echo.
echo BUILD THANH CONG:
echo   dist\ThanLongAutoTrain_NewCore_v1.0.1.exe
echo   dist\ThanLongNewCoreBridge.dll
exit /b 0

:fail
echo.
echo BUILD THAT BAI. Xem loi compiler ngay phia tren.
exit /b 1
