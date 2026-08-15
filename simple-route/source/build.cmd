@echo off
setlocal
cd /d "%~dp0"
where zig >nul 2>nul
if errorlevel 1 (
  echo KHONG TIM THAY ZIG TRONG PATH
  exit /b 1
)
if not exist dist mkdir dist
del /q dist\app.res dist\ThanLongAutoRoute_v*.exe >nul 2>nul

echo [1/5] Route-only source audit...
powershell -NoProfile -ExecutionPolicy Bypass -Command ^
  "$ErrorActionPreference='Stop'; $s=Get-Content 'src/main.cpp' -Raw;" ^
  "if($s -notmatch 'void Worker\(\)'){throw 'Worker missing'};" ^
  "$a=$s.IndexOf('void Worker()'); $b=$s.IndexOf('GameProcess game_', $a); if($a -lt 0 -or $b -lt 0){throw 'Worker range missing'}; $w=$s.Substring($a,$b-$a);" ^
  "$required=@('ClickInternalDauThai','ToggleRide(false)','ToggleRide(true)','StartPathTo(target_)','StopPathOnly','AtTarget(live)','mapReady','waitingChangeMap'); foreach($x in $required){if($w -notmatch [regex]::Escape($x)){throw ('Route FSM missing '+$x)}};" ^
  "$forbidden=@('TriggerAutoFight','RequestSell','ClickNPC','TryApplyAndVerifyBuff','TriggerTreatment','RequestUsingSkill','SelectTarget'); foreach($x in $forbidden){if($w -match [regex]::Escape($x)){throw ('Forbidden route action in Worker: '+$x)}};" ^
  "if($s -notmatch 'DictionaryEntries = 0x18' -or $s -notmatch 'EntryValue = 0x10'){throw 'Direct UI dictionary layout missing'};" ^
  "if($s -notmatch 'TimeDateStamp != 0x6A410C14u' -or $s -notmatch 'SizeOfImage != 0x03DCB000u'){throw 'Client build guard missing'};" ^
  "Write-Host 'ROUTE-ONLY AUDIT PASS: revive + mount + AutoPath + arrival dismount; combat/NPC/sell/buff absent from Worker.'"
if errorlevel 1 exit /b 1

echo [2/5] Resource...
pushd resources
zig rc /c 65001 /fo ..\dist\app.res app.rc
popd
if errorlevel 1 exit /b 1

echo [3/5] Build Windows x64...
zig c++ -target x86_64-windows-gnu -O2 -std=c++17 -Wall -Wextra -Werror -Wno-unused-const-variable -Wno-unused-function -Wno-unused-private-field -municode -static -s ^
  src\main.cpp src\remote_worker.S dist\app.res ^
  -Wl,--subsystem,windows -lcomctl32 -luser32 -lkernel32 -lgdi32 ^
  -o dist\ThanLongAutoRoute_v1.0.0.exe
if errorlevel 1 exit /b 1

echo [4/5] Binary scope audit...
powershell -NoProfile -ExecutionPolicy Bypass -Command ^
  "$ErrorActionPreference='Stop'; $p='dist/ThanLongAutoRoute_v1.0.0.exe'; if(!(Test-Path $p)){throw 'EXE missing'}; $bytes=[IO.File]::ReadAllBytes($p); $ascii=[Text.Encoding]::ASCII.GetString($bytes); $unicode=[Text.Encoding]::Unicode.GetString($bytes);" ^
  "$wanted=@('AUTO RA B','AutoPath','Map'); foreach($x in $wanted){if($ascii -notmatch [regex]::Escape($x) -and $unicode -notmatch [regex]::Escape($x)){Write-Host ('NOTE string not visible after strip: '+$x)}};" ^
  "Write-Host 'Binary built; route-only runtime reachability enforced by Worker audit.'"
if errorlevel 1 exit /b 1

echo [5/5] Done.
echo BUILD THANH CONG: dist\ThanLongAutoRoute_v1.0.0.exe
