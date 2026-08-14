# Verification status - v1.0.6

## Static/source verification

- Khong `CreateRemoteThread`, `remote_worker.S`, `il2cpp_thread_attach` trong source.
- Khong co gameplay action: ClickNPC, Sell, Heal, Revive, AutoFight.
- Moi client chi co toi da 1 bridge request dang chay.
- Timeout fail-closed, khong retry spam.
- Same PID nhung HWND/TID doi -> tao Session moi, khong giu hook/thread context cu.
- Main-thread proof dung Unity `SynchronizationContext` + managed thread ID, khong suy dien tu window TID.
- `runtime_invoke` chi dung cho cac getter read-only cua SynchronizationContext/Thread trong command proof.

## Runtime proof condition

Chi dat `ValidUnityMainThread` khi:
- hook TID == owner window TID;
- `il2cpp_thread_current()` != null;
- `SynchronizationContext.Current` != null;
- class name == `UnitySynchronizationContext`;
- `Thread.CurrentThread.ManagedThreadId == UnitySynchronizationContext.MainThreadId`.

Neu khong dat tat ca dieu kien tren, MainThread van LOCKED va phase action khong duoc mo.
