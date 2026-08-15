# Verification status - v1.0.7

## Static/source verification

- Không `CreateRemoteThread`, `remote_worker.S`, `il2cpp_thread_attach` trong source path.
- Không gameplay action: ClickNPC, HandleClickEvent, StartAutoFight, RequestSellItem, Heal, Revive.
- Mỗi client chỉ có tối đa 1 bridge request đang chạy.
- Timeout fail-closed, không retry spam.
- Same PID nhưng HWND/TID đổi -> tạo Session mới, không giữ hook/thread context cũ.
- Main-thread proof dùng Unity `SynchronizationContext` + managed thread ID, không suy diễn từ window TID.
- `ReadGameSnapshot` gọi lại main-thread proof trước khi dùng read-only getter/query.
- Resolver snapshot dùng namespace/class/method metadata, không dùng gameplay RVA của donor v0.9.0.
- X/Y không có offset fallback trong phase này.

## Runtime proof condition

Foundation chỉ PASS khi:
- hook TID == owner window TID;
- `il2cpp_thread_current()` != null;
- `SynchronizationContext.Current` là `UnitySynchronizationContext`;
- `Thread.CurrentThread.ManagedThreadId == UnitySynchronizationContext.MainThreadId`.

Snapshot core chỉ PASS khi có RoleID, MapID, HP/MaxHP, Dead, Riding, AutoFight, FreeBagSpace và MapReady.

Position/name/moving/waiting-change-map là thông tin mở rộng; thiếu chúng không được phép kích hoạt action hay fallback sang offset donor.
