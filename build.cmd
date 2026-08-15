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
del /q ThanLongNewCoreBridge.dll BridgeSelfTest.exe ThanLongAutoTrain_NewCore_v1.0.7.exe app.res >nul 2>nul

echo [1/6] Architecture safety audit...
findstr /S /I /N /C:"CreateRemoteThread" src\*.cpp src\*.h >nul 2>nul
if not errorlevel 1 goto :forbid_remote
findstr /S /I /N /C:"il2cpp_thread_attach" src\*.cpp src\*.h >nul 2>nul
if not errorlevel 1 goto :forbid_attach
findstr /S /I /N /C:"Sleep(" src\*.cpp src\*.h >nul 2>nul
if not errorlevel 1 goto :forbid_sleep
findstr /S /I /N /C:"ClickNPC" src\*.cpp src\*.h >nul 2>nul
if not errorlevel 1 goto :forbid_clicknpc
findstr /S /I /N /C:"HandleClickEvent" src\*.cpp src\*.h >nul 2>nul
if not errorlevel 1 goto :forbid_handleclick
findstr /S /I /N /C:"StartAutoFight" src\*.cpp src\*.h >nul 2>nul
if not errorlevel 1 goto :forbid_autofight
findstr /S /I /N /C:"RequestSellItem" src\*.cpp src\*.h >nul 2>nul
if not errorlevel 1 goto :forbid_sell
echo Architecture audit PASS.
goto :audit_pass

:forbid_remote
echo FORBIDDEN TOKEN FOUND: CreateRemoteThread
goto :fail
:forbid_attach
echo FORBIDDEN TOKEN FOUND: il2cpp_thread_attach
goto :fail
:forbid_sleep
echo FORBIDDEN TOKEN FOUND: Sleep(
goto :fail
:forbid_clicknpc
echo FORBIDDEN TOKEN FOUND: ClickNPC
goto :fail
:forbid_handleclick
echo FORBIDDEN TOKEN FOUND: HandleClickEvent
goto :fail
:forbid_autofight
echo FORBIDDEN TOKEN FOUND: StartAutoFight
goto :fail
:forbid_sell
echo FORBIDDEN TOKEN FOUND: RequestSellItem
goto :fail

:audit_pass

echo [2/6] Resource...
pushd resources
zig rc /c 65001 /fo ..\app.res app.rc
popd
if errorlevel 1 goto :fail

echo [3/6] Bridge DLL - read-only snapshot runtime...
rem Bridge stays on the already-proven hook/main thread. No remote worker, no thread attach, no gameplay action.
zig c++ -target x86_64-windows-gnu -O2 -std=c++17 -DUNICODE -D_UNICODE ^
  -Wall -Wextra -Werror -fno-exceptions -fno-rtti -shared ^
  src\bridge\bridge.cpp -luser32 -lkernel32 -o ThanLongNewCoreBridge.dll
if errorlevel 1 goto :fail
if not exist ThanLongNewCoreBridge.dll goto :fail

echo [4/6] Bridge LoadLibrary self-test...
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

echo [5/6] Controller EXE...
zig c++ -target x86_64-windows-gnu -O2 -std=c++17 -DUNICODE -D_UNICODE -Wall -Wextra -Werror -municode -static ^
  src\controller\main.cpp app.res ^
  -Wl,--subsystem,windows -lcomctl32 -luser32 -lkernel32 -lgdi32 ^
  -o ThanLongAutoTrain_NewCore_v1.0.7.exe
if errorlevel 1 goto :fail

echo [6/6] Package...
move /y ThanLongNewCoreBridge.dll dist\ThanLongNewCoreBridge.dll >nul
move /y ThanLongAutoTrain_NewCore_v1.0.7.exe dist\ThanLongAutoTrain_NewCore_v1.0.7.exe >nul
move /y app.res dist\app.res >nul
del /q BridgeSelfTest.exe >nul 2>nul

echo.
echo BUILD + LOADLIBRARY SELFTEST THANH CONG:
echo   dist\ThanLongAutoTrain_NewCore_v1.0.7.exe
echo   dist\ThanLongNewCoreBridge.dll
echo.
echo Sau khi chay: tick client - bam "Kiem tra nen + Snapshot".
echo Ban nay CHI DOC snapshot. Action game van KHOA.
exit /b 0

:fail
echo.
echo BUILD/SELFTEST THAT BAI. KHONG DUNG DLL neu self-test khong PASS.
exit /b 1
