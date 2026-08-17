@echo off
setlocal
cd /d "%~dp0"
where zig >nul 2>nul
if errorlevel 1 (echo KHONG TIM THAY ZIG & exit /b 1)
if not exist dist mkdir dist
del /q dist\* >nul 2>nul

echo [1/8] Clean Route v1.5.10 Internal Confirm + Revive v0.8.7 audit...
powershell -NoProfile -ExecutionPolicy Bypass -Command ^
  "$ErrorActionPreference='Stop'; $files=@('src/controller.cpp','src/bridge.cpp','src/protocol.h','src/route_logic.h','src/rotation_logic.h'); $all=($files|%%{Get-Content -Encoding UTF8 $_ -Raw}) -join [Environment]::NewLine;" ^
  "$forbidden=@('CreateRemoteThread','WriteProcessMemory','remote_worker','StartAutoFight','RequestSellItem','RequestUsingSkill','SelectTarget','ProcessRemoveItem','mouse_event'); foreach($x in $forbidden){if($all -match [regex]::Escape($x)){throw ('Forbidden unsafe/legacy token: '+$x)}};" ^
  "$scan=[regex]::Match($controller,'(?s)void ScanClients\(\).*?void InsertAccountRow').Value; if($scan -match 'bridge\.Attach|bridge\.Call|ReadSnapshot'){throw 'ScanClients must stay passive: no bridge attach/call/snapshot'};" ^
  "$controller=Get-Content -Encoding UTF8 'src/controller.cpp' -Raw; $oldConfirm=@('enableConfirm','confirmIntervalSec','IDC_ENABLE_CONFIRM','IDC_CONFIRM_INTERVAL','IDC_CAPTURE_CONFIRM','IDC_POINT_CONFIRM','IDC_TEST_CONFIRM','ClickSlot::Confirm','ClickSlot::Revive','HandlePeriodicConfirmClick','PeriodicConfirmBusy','lastPeriodicConfirmTick'); foreach($x in $oldConfirm){if($controller -cmatch [regex]::Escape($x)){throw ('Old coordinate/timer Confirm-Revive residue: '+$x)}};" ^
  "foreach($x in @('ClickSlot::AutoMenu','ClickSlot::Attack','ClickSlot::StopAuto1','ClickSlot::StopAuto2','std::array<ClickPoint, 4>','Tự Đầu thai (nội bộ v0.8.7)','HandleCrossMapConfirm','ProbeInternalConfirmUi','Command::ClickInternalConfirm','Command::ClickInternalRevive','Command::ProbeInternalConfirm','HandleDeath','HandleUnderworldAutoFightGuard','HandleAutoSell','HandleTrainRecovery','SwitchToNextRotationSpot','RemoveLegacyConfirmReviveKeys')){if($controller -notmatch [regex]::Escape($x)){throw ('Missing required v1.5.10 controller token: '+$x)}};" ^
  "if(([regex]::Matches($controller,'HandleCrossMapConfirm\(a, now, a\.profile\.target\)')).Count -ne 1){throw 'Cross-map internal Confirm must be invoked exactly once per TickAccount path'};" ^
  "$bridge=Get-Content -Encoding UTF8 'src/bridge.cpp' -Raw; foreach($x in @('LuaMainFindUI','LuaFindUI','UIObject.instances','UIButtonHandleClick','ProbeInternalConfirm','ClickInternalConfirm','ClickInternalRevive','get_IsDeath','il2cpp_gchandle_new','il2cpp_gchandle_get_target','0x6A410C14u','0x03DCB000u')){if($bridge -notmatch [regex]::Escape($x)){throw ('Missing donor v0.8.7 bridge token: '+$x)}}; if($bridge -match 'ReadConfirmVisible'){throw 'Core ReadState must not poll donor UI on every snapshot'}; if($bridge -match 'bool ClickAt\(' -or $bridge -match 'Command::ClickAt'){throw 'Unused generic bridge coordinate ClickAt must stay removed'};" ^
  "$proto=Get-Content -Encoding UTF8 'src/protocol.h' -Raw; foreach($x in @('ReadState = 1','ToggleRide = 2','StartPath = 3','StopPath = 4','ClickNpc = 6','ClickInternalConfirm = 7','ClickInternalRevive = 8','ProbeInternalConfirm = 9','ValidConfirmUi','0x00010510u')){if($proto -notmatch [regex]::Escape($x)){throw ('Protocol missing v1.5.10 token: '+$x)}}; if($proto -match 'ClickAt ='){throw 'Protocol still exposes removed generic ClickAt'};" ^
  "$route=Get-Content -Encoding UTF8 'src/route_logic.h' -Raw; foreach($x in @('MountAssistAction','DecideMountAssist','mountRetryWaitMs = 5000','footWalkMaxMs = 15000')){if($route -notmatch [regex]::Escape($x)){throw ('Route logic missing '+$x)}};" ^
  "if($controller -match 'xác nhận sau Đầu thai'){throw 'Special post-revive coordinate Confirm must stay removed'}; if($controller -match '284,188' -or $controller -match '96,168' -or $controller -match 'displayX' -or $controller -match 'displayY'){throw 'NPC route coordinates must not be hard-coded'}; if($controller -match 'ExitWindowsEx' -or $controller -match 'InitiateSystemShutdown'){throw 'Timer must close tool only, never Windows'}; Write-Host 'CLEAN ROUTE v1.5.10 AUDIT PASS: donor v0.8.7 internal Confirm + Revive, no old Confirm/Revive coordinate/timer UI, four AutoFight click slots retained.'"
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
  -o dist\ThanLongCleanRoute_v1.5.10.exe
if errorlevel 1 exit /b 1

echo [8/8] Done.
echo BUILD THANH CONG v1.5.10
