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
del /q ThanLongNewCoreBridge.dll BridgeSelfTest.exe ControlSelfTest.exe ReviveSelfTest.exe ThanLongAutoTrain_NewCore_v1.2.1.exe app.res >nul 2>nul

echo [1/10] Architecture + anti-diss Revive dispatcher audit...
findstr /S /I /N /C:"CreateRemoteThread" src\*.cpp src\*.h src\*.inc >nul 2>nul
if not errorlevel 1 goto :forbid_remote
findstr /S /I /N /C:"il2cpp_thread_attach" src\*.cpp src\*.h src\*.inc >nul 2>nul
if not errorlevel 1 goto :forbid_attach
findstr /S /I /N /C:"Sleep(" src\*.cpp src\*.h src\*.inc >nul 2>nul
if not errorlevel 1 goto :forbid_sleep
findstr /I /N /C:"kGameplayMutationEnabled = false" src\controller\control_scaffold.h >nul 2>nul
if errorlevel 1 goto :global_mutation_lock_missing
findstr /I /N /C:"kReviveMutationEnabled = true" src\controller\revive_control_gate.h >nul 2>nul
if errorlevel 1 goto :revive_gate_missing
findstr /I /N /C:"static constexpr std::size_t kCapacity = 1;" src\controller\control_scaffold.h >nul 2>nul
if errorlevel 1 goto :queue_capacity_missing
findstr /I /N /C:"ReadGameSnapshot = 5" src\common\protocol.h >nul 2>nul
if errorlevel 1 goto :protocol_gate_missing
findstr /I /N /C:"ProveHookActionEnvelope = 6" src\common\protocol.h >nul 2>nul
if errorlevel 1 goto :harmless_command_missing
findstr /I /N /C:"InvokeReviveButton = 7" src\common\protocol.h >nul 2>nul
if errorlevel 1 goto :revive_command_missing
findstr /I /N /C:"kProtocolVersion = 0x00010201u" src\common\protocol.h >nul 2>nul
if errorlevel 1 goto :protocol_version_missing

rem Metadata capability probe stays read-only.
findstr /I /N /C:"runtime_invoke" src\bridge\action_capability_probe.inc >nul 2>nul
if not errorlevel 1 goto :probe_mutation_path
findstr /I /N /C:"InvokeObject(" src\bridge\action_capability_probe.inc >nul 2>nul
if not errorlevel 1 goto :probe_mutation_path
findstr /I /N /C:"InvokeScalar(" src\bridge\action_capability_probe.inc >nul 2>nul
if not errorlevel 1 goto :probe_mutation_path
findstr /I /N /C:"BridgeCommand::" src\bridge\action_capability_probe.inc >nul 2>nul
if not errorlevel 1 goto :probe_mutation_path

rem Harmless envelope stays completely gameplay-free.
for %%T in (ClickNPC HandleClickEvent StartAutoFight RequestSellItem StartPath StopPath) do (
  findstr /I /N /C:"%%T" src\bridge\harmless_action_envelope.inc >nul 2>nul
  if not errorlevel 1 goto :harmless_gameplay_token
)
findstr /I /N /C:"runtime_invoke" src\bridge\harmless_action_envelope.inc >nul 2>nul
if not errorlevel 1 goto :harmless_gameplay_token
findstr /I /N /C:"ProveUnityMainThread" src\bridge\harmless_action_envelope.inc >nul 2>nul
if errorlevel 1 goto :harmless_mainthread_gate_missing

rem Revive is the ONE gameplay mutation, but v1.2.1 FORBIDS direct HandleClickEvent invoke in WH_GETMESSAGE.
for %%T in (ClickNPC StartAutoFight RequestSellItem StartPath StopPath HandlePointerClick) do (
  findstr /I /N /C:"%%T" src\bridge\revive_action_engine.inc >nul 2>nul
  if not errorlevel 1 goto :revive_forbidden_gameplay_token
)
findstr /I /N /C:"HandleClickEvent" src\bridge\revive_action_engine.inc >nul 2>nul
if errorlevel 1 goto :revive_callback_missing
findstr /I /N /C:"g_api.runtime_invoke(handleClick" src\bridge\revive_action_engine.inc >nul 2>nul
if not errorlevel 1 goto :revive_direct_invoke_forbidden
findstr /I /N /C:"Delegate.CreateDelegate" src\bridge\revive_action_engine.inc >nul 2>nul
if errorlevel 1 goto :revive_delegate_missing
findstr /I /N /C:"MainThread.Execute(System.Action)" src\bridge\revive_action_engine.inc >nul 2>nul
if errorlevel 1 goto :revive_mainthread_queue_missing
findstr /I /N /C:"kReviveResolveBudgetMs = 120" src\bridge\revive_action_engine.inc >nul 2>nul
if errorlevel 1 goto :revive_budget_missing
findstr /I /N /C:"ReviveStageQueued" src\bridge\revive_action_engine.inc >nul 2>nul
if errorlevel 1 goto :revive_breadcrumb_missing
findstr /I /N /C:"UIObject.instances" src\bridge\revive_action_engine.inc >nul 2>nul
if errorlevel 1 goto :revive_fresh_resolver_missing
findstr /I /N /C:"dead=1" src\bridge\revive_action_engine.inc >nul 2>nul
if errorlevel 1 goto :revive_precondition_missing

echo Architecture audit PASS. Global mutation FALSE. Revive ONLY TRUE. Direct HandleClickEvent invoke=0. Deferred MainThread.Execute(Action) required. Resolve budget=120ms.
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
:global_mutation_lock_missing
echo REQUIRED GLOBAL GAMEPLAY MUTATION LOCK NOT FALSE
goto :fail
:revive_gate_missing
echo EXCLUSIVE REVIVE MUTATION GATE NOT TRUE
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
:revive_command_missing
echo REVIVE COMMAND=7 NOT FOUND
goto :fail
:protocol_version_missing
echo PROTOCOL VERSION 0x00010201 NOT FOUND
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
:revive_forbidden_gameplay_token
echo REVIVE ENGINE CONTAINS NON-REVIVE GAMEPLAY TOKEN
goto :fail
:revive_callback_missing
echo REVIVE ENGINE DOES NOT RESOLVE UIButton.HandleClickEvent
goto :fail
:revive_direct_invoke_forbidden
echo DIRECT runtime_invoke(handleClick) IS FORBIDDEN IN v1.2.1; MUST QUEUE System.Action
goto :fail
:revive_delegate_missing
echo REVIVE ENGINE MUST CREATE A MANAGED System.Action DELEGATE
goto :fail
:revive_mainthread_queue_missing
echo REVIVE ENGINE MUST USE MainThread.Execute(System.Action)
goto :fail
:revive_budget_missing
echo REVIVE RESOLVER MUST HAVE 120ms HARD TIME BUDGET
goto :fail
:revive_breadcrumb_missing
echo REVIVE TIMEOUT BREADCRUMB STAGE MISSING
goto :fail
:revive_fresh_resolver_missing
echo REVIVE ENGINE MUST FRESH-RESOLVE UIObject.instances
goto :fail
:revive_precondition_missing
echo REVIVE ENGINE MUST PIN dead=1 PRECONDITION
goto :fail

:audit_pass

echo [2/10] Compile deterministic integrated control self-test...
zig c++ -target x86_64-windows-gnu -O2 -std=c++17 -DUNICODE -D_UNICODE ^
  -Wall -Wextra -Werror -municode -static ^
  src\controller\control_selftest.cpp -o ControlSelfTest.exe
if errorlevel 1 goto :fail

echo [3/10] Run integrated 48-check control self-test...
ControlSelfTest.exe
if errorlevel 1 (
  echo CONTROL SELF TEST FAILED. FAIL-CLOSED.
  goto :fail
)

echo [4/10] Compile exclusive Revive lifecycle self-test...
zig c++ -target x86_64-windows-gnu -O2 -std=c++17 -DUNICODE -D_UNICODE ^
  -Wall -Wextra -Werror -static ^
  src\controller\revive_selftest.cpp -o ReviveSelfTest.exe
if errorlevel 1 goto :fail

echo [5/10] Run Revive queued-dispatch/map-change POST self-test...
ReviveSelfTest.exe
if errorlevel 1 (
  echo REVIVE SELF TEST FAILED. FAIL-CLOSED.
  goto :fail
)

echo [6/10] Resource...
pushd resources
zig rc /c 65001 /fo ..\app.res app.rc
popd
if errorlevel 1 goto :fail

echo [7/10] Bridge DLL - scanner + bounded resolver + deferred MainThread Revive ONLY...
zig c++ -target x86_64-windows-gnu -O2 -std=c++17 -DUNICODE -D_UNICODE ^
  -Wall -Wextra -Werror -fno-exceptions -fno-rtti -shared ^
  src\bridge\bridge.cpp -luser32 -lkernel32 -o ThanLongNewCoreBridge.dll
if errorlevel 1 goto :fail
if not exist ThanLongNewCoreBridge.dll goto :fail

echo [8/10] Bridge LoadLibrary self-test...
zig c++ -target x86_64-windows-gnu -O2 -std=c++17 -DUNICODE -D_UNICODE ^
  -Wall -Wextra -Werror -municode -static ^
  src\bridge\selftest.cpp -lkernel32 -o BridgeSelfTest.exe
if errorlevel 1 goto :fail
BridgeSelfTest.exe "%CD%\ThanLongNewCoreBridge.dll"
if errorlevel 1 (
  echo LOI: DLL vua build khong LoadLibrary duoc tren Windows runner.
  goto :fail
)

echo [9/10] Controller EXE - v1.2.1 Revive Dispatcher Hotfix...
zig c++ -target x86_64-windows-gnu -O2 -std=c++17 -DUNICODE -D_UNICODE -Wall -Wextra -Werror -municode -static ^
  src\controller\main.cpp app.res ^
  -Wl,--subsystem,windows -lcomctl32 -luser32 -lkernel32 -lgdi32 ^
  -o ThanLongAutoTrain_NewCore_v1.2.1.exe
if errorlevel 1 goto :fail

echo [10/10] Package...
move /y ThanLongNewCoreBridge.dll dist\ThanLongNewCoreBridge.dll >nul
move /y ThanLongAutoTrain_NewCore_v1.2.1.exe dist\ThanLongAutoTrain_NewCore_v1.2.1.exe >nul
move /y app.res dist\app.res >nul
del /q BridgeSelfTest.exe ControlSelfTest.exe ReviveSelfTest.exe >nul 2>nul

echo.
echo BUILD + ALL AUTOMATED SELF-TESTS PASS:
echo   dist\ThanLongAutoTrain_NewCore_v1.2.1.exe
echo   dist\ThanLongNewCoreBridge.dll
echo.
echo Global gameplay mutation remains FALSE.
echo Revive is the ONLY enabled gameplay mutation.
echo HandleClickEvent is NEVER direct-invoked inside WH_GETMESSAGE; it is queued as System.Action via FG MainThread.Execute.
echo AutoFight/NPC/Sell/Path and every other gameplay action remain LOCKED.
exit /b 0

:fail
echo.
echo BUILD/SELFTEST THAT BAI. FAIL-CLOSED; KHONG DUNG ARTIFACT.
exit /b 1
