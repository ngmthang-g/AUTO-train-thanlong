# Verification status - v1.0.8

## Static/source verification

- Protocol bumped to `0x00010008`.
- Không `CreateRemoteThread`, `remote_worker.S`, `il2cpp_thread_attach` trong NewCore source path.
- Không `Sleep()`.
- Không gameplay action token trong `src`: ClickNPC / HandleClickEvent / StartAutoFight / RequestSellItem.
- Mỗi client chỉ có tối đa 1 bridge request.
- Main-thread proof vẫn được chạy lại trước mỗi `ReadGameSnapshot`.
- AutoPath state resolve qua metadata `FGStudio.Engine.Logic.AutoPathManager.get_IsAutoPathing`.
- Supplied `global-metadata.dat` chứa class/method strings tương ứng.
- Donor role offsets không được dùng làm runtime fallback.

## Stable observer parity mask

`SCANNER PASS` chỉ khi snapshot stable có:

- RoleID
- MapID
- X/Y
- HP/MaxHP
- Dead
- Riding
- Moving
- AutoFight
- FreeBagSpace
- MapReady
- WaitingChangeMap
- AutoPathing

Character name vẫn là optional display field.

Nếu thiếu Position/Moving/AutoPath, trạng thái là `SCANNER PARTIAL`; scanner vẫn read-only và action vẫn khóa.

## Transition snapshot

Khi `MapReady=false` hoặc `WaitingChangeMap=true`, snapshot transition được coi là **thành công read-only có chủ đích**, không phải lỗi core. Nó chỉ mang transition flags và sequence; Leader/AutoPath không được query trong nhịp đó. Sau transition phải có 2 stable snapshots liên tiếp trước khi observer quay lại PASS/PARTIAL.

## Runtime proof cần test

Xem `docs/FIRST_RUNTIME_TEST.md`.

## Build note

Authoritative Windows build vẫn là `build.cmd` / GitHub Actions với Zig 0.15.2. Môi trường Linux tạo source này kdông có Zig local, nên không được tuyên bố đã link PE tại chỗ.
