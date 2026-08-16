@echo off
setlocal
cd /d "%~dp0"
where zig >nul 2>nul
if errorlevel 1 (echo KHONG TIM THAY ZIG & exit /b 1)
if not exist dist mkdir dist
del /q dist\* >nul 2>nul

echo [1/7] Clean Route v1.5.6 Death Session Cold Restart audit...
powershell -NoProfile -ExecutionPolicy Bypass -Command ^
  "$ErrorActionPreference='Stop'; $files=@('src/controller.cpp','src/bridge.cpp','src/protocol.h','src/route_logic.h'); $s=($files|%%{Get-Content $_ -Raw}) -join [Environment]::NewLine;" ^
  "$forbidden=@('CreateRemoteThread','WriteProcessMemory','remote_worker','ClickInternalConfirm','StartAutoFight','RequestSellItem','RequestUsingSkill','SelectTarget','ProcessRemoveItem','mouse_event'); foreach($x in $forbidden){if($s -match [regex]::Escape($x)){throw ('Forbidden direct/game-input token: '+$x)}};" ^
  "$bridge=Get-Content 'src/bridge.cpp' -Raw; foreach($x in @('SendToggleRideState','StartAutoPath','StopAutoPath','get_IsDeath','get_EnableAutoF1','GetFreeBagSpace','ClickNPC','ReadSemanticUiPresent','LuaSystemAPI_GUI','FindUI','ValidConfirmUi','confirmUiVisible')){if($bridge -notmatch [regex]::Escape($x)){throw ('Missing required route/read-only method '+$x)}};" ^
  "$controller=Get-Content 'src/controller.cpp' -Raw; foreach($x in @('RegisterHotKey','VK_F8','VK_F4','LVS_EX_CHECKBOXES','Role_','ThanLongCleanRoute.spots.tsv','IDC_SPOT_COMBO','SendInput','SetForegroundWindow','SetCursorPos','ClickSlot::Confirm','ClickSlot::Revive','ClickSlot::AutoMenu','ClickSlot::Attack','ClickSlot::StopAuto1','ClickSlot::StopAuto2','IDC_SELL_MACRO_LIST','IDC_SELL_NPC_X','IDC_SELL_NPC_Y','IDC_SELL_NPC_CAPTURE','SellNpcPos_','kMouseHoldMs','HandleAutoSell','kTrainPositionCheckMs = 180000','kMountRetryWaitMs = 5000','kFootWalkMaxMs = 15000','HandleTrainRecovery','HandleRobustTravel','kAutoFightRecheckMs = 60000','AutoFightCheckBusy','MigrateLegacyConfigIfNeeded','ConfigDir','FlushIni','ValidConfirmUi','confirmUiVisible','confirmUiFirstSeenTick','confirmStopPathTick','MessageBox ON + Path ON','Cross-map Confirm v1.5.3','IDC_SHUTDOWN_ENABLE','ResolveNextShutdownTarget','CheckScheduledShutdown','kClientStableResumeMs = 2000','kBridgeNudgeMs = 750','clientFreezeActive','HoldUntilClientStable','pendingSeq_','bridgeBusy','MarkReadStateFailure','kUnderworldMapId = 87','HandleUnderworldAutoFightGuard','underworldStopPhase','kUnderworldMaxStopAttempts = 3','deathSessionLatched','NEW DEATH SESSION','POST-REVIVE COLD START','ResetRuntime(rt)')){if($controller -notmatch [regex]::Escape($x)){throw ('Missing v1.5.6 controller implementation '+$x)}};" ^
  "$route=Get-Content 'src/route_logic.h' -Raw; foreach($x in @('MountAssistAction','DecideMountAssist','mountRetryWaitMs = 5000','footWalkMaxMs = 15000')){if($route -notmatch [regex]::Escape($x)){throw ('Route logic missing '+$x)}};" ^
  "$proto=Get-Content 'src/protocol.h' -Raw; foreach($x in @('ReadState = 1','ToggleRide = 2','StartPath = 3','StopPath = 4','ClickAt = 5','ClickNpc = 6','ValidBagSpace','ValidConfirmUi','confirmUiVisible','targetHwnd')){if($proto -notmatch [regex]::Escape($x)){throw ('Protocol missing '+$x)}};" ^
  "if($controller -match '284,188' -or $controller -match '96,168' -or $controller -match 'displayX' -or $controller -match 'displayY'){throw 'NPC route coordinates must not be hard-coded'}; if($controller -notmatch 's\.validMask & ValidConfirmUi' -or $controller -notmatch 's\.confirmUiVisible' -or $controller -notmatch 'rt\.crossMapRouteArmed' -or $controller -notmatch 'rt\.crossMapSeenAutoPath \|\| rt\.crossMapRouteMoved'){throw 'Semantic cross-map Confirm detector guard missing'}; if($controller -match 'if \(s\.autoPathing\) \{\s*rt\.crossMapSeenAutoPath = true;\s*rt\.status = L\"Cross-map • AutoPath đang chạy\";\s*return true;'){throw 'v1.5.2 early Path-ON deadlock guard still present'}; if($controller -match 'ExitWindowsEx' -or $controller -match 'InitiateSystemShutdown'){throw 'Timer must close tool only, never Windows'}; Write-Host 'CLEAN ROUTE v1.5.6 AUDIT PASS: dual-boundary death-session ResetRuntime + external latch + M87 AutoFight guard + transition freeze + Path-ON Confirm.'"
if errorlevel 1 exit /b 1

echo [2/7] Route FSM self-test...
zig c++ -target x86_64-windows-gnu -O2 -std=c++17 -Wall -Wextra -Werror src\route_logic_test.cpp -o dist\RouteLogicTest.exe
if errorlevel 1 exit /b 1
dist\RouteLogicTest.exe
if errorlevel 1 exit /b 1

echo [3/7] Build bridge DLL...
rem IMPORTANT: do NOT combine -shared with -static here; that can emit an ar archive named .dll.
zig c++ -target x86_64-windows-gnu -O2 -std=c++17 -Wall -Wextra -Werror -shared -s ^
  src\bridge.cpp -luser32 -lkernel32 -o dist\ThanLongCleanRouteBridge.dll
if errorlevel 1 exit /b 1

echo [4/7] Verify bridge is a real PE DLL...
powershell -NoProfile -ExecutionPolicy Bypass -Command ^
  "$ErrorActionPreference='Stop'; $p='dist\ThanLongCleanRouteBridge.dll'; $b=[IO.File]::ReadAllBytes($p); if($b.Length -lt 256){throw 'Bridge DLL too small'}; if($b[0] -ne 0x4D -or $b[1] -ne 0x5A){throw 'Bridge is not PE/MZ (wrong artifact type)'}; $pe=[BitConverter]::ToInt32($b,0x3C); if($pe -lt 0 -or $pe+24 -ge $b.Length){throw 'Invalid PE header offset'}; if($b[$pe] -ne 0x50 -or $b[$pe+1] -ne 0x45 -or $b[$pe+2] -ne 0 -or $b[$pe+3] -ne 0){throw 'Missing PE signature'}; $ch=[BitConverter]::ToUInt16($b,$pe+22); if(($ch -band 0x2000) -eq 0){throw 'PE does not have DLL characteristic'}; Write-Host ('BRIDGE PE DLL PASS characteristics=0x{0:X4}' -f $ch)"
if errorlevel 1 exit /b 1

echo [5/7] Build resources...
pushd resources
zig rc /c 65001 /fo ..\dist\app.res app.rc
popd
if errorlevel 1 exit /b 1

echo [6/7] Build controller EXE...
zig c++ -target x86_64-windows-gnu -O2 -std=c++17 -Wall -Wextra -Werror -municode -static -s ^
  src\controller.cpp dist\app.res -Wl,--subsystem,windows ^
  -lcomctl32 -luser32 -lkernel32 -lgdi32 ^
  -o dist\ThanLongCleanRoute_v1.5.6.exe
if errorlevel 1 exit /b 1

echo [7/7] Done.
echo BUILD THANH CONG
