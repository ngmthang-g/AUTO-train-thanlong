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
del /q ThanLongNewCoreBridge.dll BridgeSelfTest.exe ControlSelfTest.exe ThanLongAutoTrain_NewCore_v1.1.0.exe app.res >nul 2>nul

echo [1/8] Architecture + mutation safety audit...
findstr /S /I /N /C:"CreateRemoteThread" src\*.cpp src\*.h src\*.inc >nul 2>nul
if not errorlevel 1 goto :forbid_remote
findstr /S /I /N /C:"il2cpp_thread_attach" src\*.cpp src\*.h src\*.inc >nul 2>nul
if not errorlevel 1 goto :forbid_attach
findstr /S /I /N /C:"Sleep(" src\*.cpp src\*.h src\*.inc >nul 2>nul
if not errorlevel 1 goto :forbid_sleep
findstr /I /N /C:"kGameplayMutationEnabled = false" src\controller\control_scaffold.h >nul 2>nul
if errorlevel 1 goto :mutation_lock_missing
findstr /I /N /C:"kHarmlessInfrastructureProofEnabled = true" src\controller\control_scaffold.h >nul 2>nul
if errorlevel 1 goto :harmless_proof_gate_missing
findstr /I /N /C:"static constexpr std::size_t kCapacity = 1;" src\controller\control_scaffold.h >nul 2>nul
if errorlevel 1 goto :queue_capacity_missing
findstr /I /N /C:"ReadGameSnapshot = 5" src\common\protocol.h >nul 2>nul
if errorlevel 1 goto :protocol_gate_missing
findstr /I /N /C:"ProveHookActionEnvelope = 6" src\common\protocol.h >nul 2>nul
if errorlevel 1 goto :harmless_command_missing
findstr /I /N /C:"kProtocolVersion = 0x00010102u" src\common\protocol.h >nul 2>nul
if errorlevel 1 goto :protocol_version_missing
findstr /I /N /C:"runtime_invoke" src\bridge\action_capability_probe.inc >nul 2>nul
if not errorlevel 1 goto :probe_mutation_path
findstr /I /N /C:"InvokeObject(" src\bridge\action_capability_probe.inc >nul 2>nul
if not errorlevel 1 goto :probe_mutation_path
findstr /I /N /C:"InvokeScalar(" src\bridge\action_capability_probe.inc >nul 2>nul
if not errorlevel 1 goto :probe_mutation_path
findstr /I /N /C:"BridgeCommand::" src\bridge\action_capability_probe.inc >nul 2>nul
if not errorlevel 1 goto :probe_mutation_path
for %%T in (ClickNPC HandleClickEvent StartAutoFight RequestSellItem StartPath StopPath) do (
  findstr /I /N /C:"%%T" src\bridge\harmless_action_envelope.inc >nul 2>nul
  if not errorlevel 1 goto :harmless_gameplay_token
)
findstr /I /N /C:"runtime_invoke" src\bridge\harmless_action_envelope.inc >nul 2>nul
if not errorlevel 1 goto :harmless_gameplay_token
findstr /I /N /C:"ProveUnityMainThread" src\bridge\harmless_action_envelope.inc >nul 2>nul
if errorlevel 1 goto :harmless_mainthread_gate_missing
echo Architecture audit PASS. Gameplay mutation FALSE. Queue MAX=1. Command 6 is harmless proof only. Capability probe metadata-only.
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
:mutation_lock_missing
echo REQUIRED GAMEPLAY MUTATION LOCK NOT FOUND OR NOT FALSE
goto :fail
:harmless_proof_gate_missing
echo HARMLESS INFRASTRUCTURE PROOF GATE NOT FOUND OR NOT TRUE
goto :fail
:queue_capacity_missing
echo REQUIRED ACTION QUEUE CAPACITY=1 NOT FOUND
goto :fail
:protocol_gate_missing
echo READ SNAPSHOT COMMAND GATE NOT FOUND
goto :fail
:harmless_command_missing
echo HARMLESS PROOF COMMAND=6 NOT FOUND
goto :fail
:protocol_version_missing
echo PROTOCOL VERSION 0x00010102 NOT FOUND
goto :fail
:probe_mutation_path
echo ACTION CAPABILITY PROBE MUST STAY METADATA-ONLY
goto :fail
:harmless_gameplay_token
echo HARMLESS ACTION ENVELOPE CONTAINS FORBIDDEN GAMEPLAY/INVOKE TOKEN
goto :fail
:harmless_mainthread_gate_missing
echo HARMLESS ACTION ENVELOPE MUST RE-PROVE UNITY MAIN THREAD
goto :fail

:audit_pass

echo [2/8] Compile deterministic integrated control self-test...
zig c++ -target x86_64-windows-gnu -O2 -std=c++17 -DUNICODE -D_UNICODE ^
  -Wall -Wextra -Werror -municode -static ^
  src\controller\control_selftest.cpp -o ControlSelfTest.exe
if errorlevel 1 goto :fail

echo [3/8] Run integrated FSM/Guard/Queue/Watchdog/Postcondition/HarmlessEnvelope self-test...
ControlSelfTest.exe
if errorlevel 1 (
  echo CONTROL SELF TEST FAILED. FAIL-CLOSED.
  goto :fail
)

echo [4/8] Resource...
pushd resources
zig rc /c 65001 /fo ..\app.res app.rc
popd
if errorlevel 1 goto :fail

echo [5/8] Bridge DLL - scanner/main-thread + metadata resolver + harmless hook envelope...
zig c++ -target x86_64-windows-gnu -O2 -std=c++17 -DUNICODE -D_UNICODE ^
  -Wall -Wextra -Werror -fno-exceptions -fno-rtti -shared ^
  src\bridge\bridge.cpp -luser32 -lkernel32 -o ThanLongNewCoreBridge.dll
if errorlevel 1 goto :fail
if not exist ThanLongNewCoreBridge.dll goto :fail

echo [6/8] Bridge LoadLibrary self-test...
zig c++ -target x86_64-windows-gnu -O2 -std=c++17 -DUNICODE -D_UNICODE ^
  -Wall -Wextra -Werror -municode -static ^
  src\bridge\selftest.cpp -lkernel32 -o BridgeSelfTest.exe
if errorlevel 1 goto :fail
BridgeSelfTest.exe "%CD%\ThanLongNewCoreBridge.dll"
if errorlevel 1 (
  echo LOI: DLL vua build khong LoadLibrary duoc tren Windows runner.
  goto :fail
)

echo [7/8] Controller EXE - v1.1.0 Integrated Acceptance...
zig c++ -target x86_64-windows-gnu -O2 -std=c++17 -DUNICODE -D_UNICODE -Wall -Wextra -Werror -municode -static ^
  src\controller\main.cpp app.res ^
  -Wl,--subsystem,windows -lcomctl32 -luser32 -lkernel32 -lgdi32 ^
  -o ThanLongAutoTrain_NewCore_v1.1.0.exe
if errorlevel 1 goto :fail

echo [8/8] Package...
move /y ThanLongNewCoreBridge.dll dist\ThanLongNewCoreBridge.dll >nul
move /y ThanLongAutoTrain_NewCore_v1.1.0.exe dist\ThanLongAutoTrain_NewCore_v1.1.0.exe >nul
move /y app.res dist\app.res >nul
del /q BridgeSelfTest.exe ControlSelfTest.exe >nul 2>nul

echo.
echo BUILD + ALL AUTOMATED SELF-TESTS PASS:
echo   dist\ThanLongAutoTrain_NewCore_v1.1.0.exe
echo   dist\ThanLongNewCoreBridge.dll
echo.
echo Gameplay mutation is STILL compile-time disabled.
echo Harmless hook/main-thread action envelope is infrastructure-only and has no gameplay call.
echo Current-client gameplay action support remains metadata-only/donor-only until later runtime proof.
exit /b 0

:fail
echo.
echo BUILD/SELFTEST THAT BAI. FAIL-CLOSED; KHONG DUNG ARTIFACT.
exit /b 1
