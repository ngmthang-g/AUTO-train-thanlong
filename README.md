# Than Long Auto - NewCore v1.0.5 Foundation Validator

Muc tieu ban nay chi la qua cua nen an toan. CHUA co action game.

## Sua so voi v1.0.3

- Sua loi Windows `Bad Image / 0xc000012f` khi controller LoadLibrary bridge.
- Bridge DLL bo `-static` khi build shared library.
- Bridge duoc rut gon: khong STL container/stream, khong exception, khong RTTI, khong managed invoke.
- `build.cmd` tu build `BridgeSelfTest.exe` va goi `LoadLibraryW` vao DLL vua tao.
  Neu DLL khong load duoc tren chinh Windows dang build, build DUNG ngay va khong package DLL loi.
- Controller chan popup Bad Image va log Win32 error cu the neu LoadLibrary van that bai.
- Protocol v1.0.5; khong dung lan DLL/EXE cua v1.0.3.

## Test

1. Giai nen vao folder moi.
2. Chay `build.cmd`.
3. Chi tiep tuc neu thay `BRIDGE SELFTEST PASS` va `BUILD + LOADLIBRARY SELFTEST THANH CONG`.
4. Chay `dist\\ThanLongAutoTrain_NewCore_v1.0.5.exe` cung voi `dist\\ThanLongNewCoreBridge.dll` trong cung folder.
5. Tick client -> `Kiem tra nen`.
6. Gui lai log Validator.

## Nguyen tac kien truc van khoa

Resolver -> Read-only Scanner -> Snapshot/State Store -> Observer -> FSM -> Safety Guard -> ActionQueue(max 1) -> MainThread Dispatcher -> Internal Action Engine.

Ban nay dung o Validator. Khong `CreateRemoteThread`, khong `runtime_invoke`, khong action game, khong Sleep de doan state.
