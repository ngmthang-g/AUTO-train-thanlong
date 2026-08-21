@echo off
setlocal
cd /d "%~dp0"
where zig >nul 2>nul
if errorlevel 1 (echo KHONG TIM THAY ZIG & exit /b 1)
if not exist dist mkdir dist
del /q dist\* >nul 2>nul

echo [1/8] Clean Route v1.5.9 Auto Rotate Train Spots audit...
powershell -NoProfile -ExecutionPolicy Bypass -Command ^
  "$ErrorActionPreference='Stop'; $files=@('src/controller.cpp','src/bridge.cpp','src/protocol.h','src/route_logic.h','src/rotation_logic.h'); $s=($files|%%{Get-Content $_ -Raw}) -join [Environment]::NewLine;" ^
  "$forbidden=@('CreateRemoteThread','WriteProcessMemory','remote_worker','ClickInternalConfirm','StartAutoFight','RequestSellItem','RequestUsingSkill','SelectTarget','ProcessRemoveItem','mouse_event'); foreach($x in $forbidden){if($s -match [regex]::Escape($x)){throw ('Forbidden direct/game-input token: '+$x)}};" ^
  "$bridge=Get-Content 'src/bridge.cpp' -Raw; foreach($x in @('SendToggleRideState','StartAutoPath','StopAutoPath','get_IsDeath','get_EnableAutoF1','GetFreeBagSpace','ClickNPC')){if($bridge -notmatch [regex]::Escape($x)){throw ('Missing required route/read-only method '+$x)}}; if($bridge -match 'ReadSemanticUiPresent' -or $bridge -match 'MainFindUI'){throw 'v1.5.9 must not probe MessageBox/UI in ReadState'};" ^
  "$controller=Get-Content 'src/controller.cpp' -Raw; foreach($x in @('RegisterHotKey','VK_F8','VK_F4','LVS_EX_CHECKBOXES','Role_','ThanLongCleanRoute.spots.tsv','IDC_SPOT_COMBO','SendInput','SetForegroundWindow','SetCursorPos','ClickSlot::Confirm','ClickSlot::Revive','ClickSlot::AutoMenu','ClickSlot::Attack','ClickSlot::StopAuto1','ClickSlot::StopAuto2','IDC_CONFIRM_INTERVAL','ConfirmIntervalSec','kConfirmIntervalDefaultSec = 5','kConfirmIntervalMinSec = 1','kConfirmIntervalMaxSec = 300','kConfirmOtherClickGuardMs = 1000','HandlePeriodicConfirmClick','PeriodicConfirmBusy','lastPeriodicConfirmTick','lastRealInputTick','globalLastRealInputTick_','IDC_SELL_MACRO_LIST','IDC_SELL_NPC_X','IDC_SELL_NPC_Y','IDC_SELL_NPC_CAPTURE','SellNpcPos_','kMouseHoldMs','HandleAutoSell','kTrainPositionCheckMs = 180000','kMountRetryWaitMs = 5000','kFootWalkMaxMs = 15000','HandleTrainRecovery','HandleRobustTravel','kAutoFightRecheckMs = 60000','AutoFightCheckBusy','MigrateLegacyConfigIfNeeded','ConfigDir','FlushIni','IDC_SHUTDOWN_ENABLE','ResolveNextShutdownTarget','CheckScheduledShutdown','kClientStableResumeMs = 2000','kBridgeNudgeMs = 750','clientFreezeActive','HoldUntilClientStable','pendingSeq_','bridgeBusy','MarkReadStateFailure','kUnderworldMapId = 87','HandleUnderworldAutoFightGuard','underworldStopPhase','kUnderworldMaxStopAttempts = 3','deathSessionLatched','NEW DEATH SESSION','POST-REVIVE COLD START','HandleRouteOwnershipReset','IDC_ROTATION_LIST','RotationCount','RotateDeathLimit','RotateDeathWindowMin','RotateNoFullBagMin','SwitchToNextRotationSpot','RecordDeathForRotation','UpdateRotationEfficiency','rotationActiveTrainMs')){if($controller -notmatch [regex]::Escape($x)){throw ('Missing v1.5.9 controller implementation '+$x)}};" ^
  "$route=Get-Content 'src/route_logic.h' -Raw; foreach($x in @('MountAssistAction','DecideMountAssist','mountRetryWaitMs = 5000','footWalkMaxMs = 15000')){if($route -notmatch [regex]::Escape($x)){throw ('Route logic missing '+$x)}};" ^
  "$proto=Get-Content 'src/protocol.h' -Raw; foreach($x in @('ReadState = 1','ToggleRide = 2','StartPath = 3','StopPath = 4','ClickAt = 5','ClickNpc = 6','ValidBagSpace','targetHwnd')){if($proto -notmatch [regex]::Escape($x)){throw ('Protocol missing '+$x)}};" ^
  "if($controller -match 'HandleCrossMapConfirm\(a, now'){throw 'Semantic Confirm handler must not be invoked in v1.5.9'}; if($controller -match 'xác nhận sau Đầu thai'){throw 'Special revive Confirm click must be removed'}; if($controller -match '284,188' -or $controller -match '96,168' -or $controller -match 'displayX' -or $controller -match 'displayY'){throw 'NPC route coordinates must not be hard-coded'}; if($controller -match 'ExitWindowsEx' -or $controller -match 'InitiateSystemShutdown'){throw 'Timer must close tool only, never Windows'}; Write-Host 'CLEAN ROUTE v1.5.9 AUDIT PASS: fixed Confirm + rotating train spots + rolling death window + no-full-bag efficiency timer.'"
if errorlevel 1 exit /b 1

echo [2/8] Route FSM self-test...
zig c++ -target x86_64-windows-gnu -O2 -std=c++17 -Wall -Wextra -Werror src\route_logic_test.cpp -o dist\RouteLogicTest.exe
if errorlevel 1 exit /b 1
dist\RouteLogicTest.exe
if errorlevel 1 exit /b 1

echo [3/8] Rotation self-test...
zig c++ -target x86_64-windows-gnu -O2 -std=c++17 -Wall -Wextra -Werror src\rotation_logic_test.cpp -o dist\RotationLogicTest.exe
if errorlevel 1 exit /b 1
dist\RotationLogicTest.exe
if errorlevel 1 exit /b 1

echo [4/8] Build bridge DLL...
zig c++ -target x86_64-windows-gnu -O2 -std=c++17 -Wall -Wextra -Werror -shared -s ^
  src\bridge.cpp -luser32 -lkernel32 -o dist\ThanLongCleanRouteBridge.dll
if errorlevel 1 exit /b 1

echo [5/8] Verify bridge is a real PE DLL...
powershell -NoProfile -ExecutionPolicy Bypass -Command ^
  "$ErrorActionPreference='Stop'; $p='dist\ThanLongCleanRouteBridge.dll'; $b=[IO.File]::ReadAllBytes($p); if($b.Length -lt 256){throw 'Bridge DLL too small'}; if($b[0] -ne 0x4D -or $b[1] -ne 0x5A){throw 'Bridge is not PE/MZ'}; $pe=[BitConverter]::ToInt32($b,0x3C); if($pe -lt 0 -or $pe+24 -ge $b.Length){throw 'Invalid PE header offset'}; if($b[$pe] -ne 0x50 -or $b[$pe+1] -ne 0x45 -or $b[$pe+2] -ne 0 -or $b[$pe+3] -ne 0){throw 'Missing PE signature'}; $ch=[BitConverter]::ToUInt16($b,$pe+22); if(($ch -band 0x2000) -eq 0){throw 'PE does not have DLL characteristic'}; Write-Host ('BRIDGE PE DLL PASS characteristics=0x{0:X4}' -f $ch)"
if errorlevel 1 exit /b 1

echo [6/8] Build resources...
pushd resources
zig rc /c 65001 /fo ..\dist\app.res app.rc
popd
if errorlevel 1 exit /b 1

echo [7/8] Build controller EXE...
zig c++ -target x86_64-windows-gnu -O2 -std=c++17 -Wall -Wextra -Werror -municode -static -s ^
  src\controller.cpp dist\app.res -Wl,--subsystem,windows ^
  -lcomctl32 -luser32 -lkernel32 -lgdi32 ^
  -o dist\ThanLongCleanRoute_v1.5.9.exe
if errorlevel 1 exit /b 1

echo [8/8] Done.
echo BUILD THANH CONG
