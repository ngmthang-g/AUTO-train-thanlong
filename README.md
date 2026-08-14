# Than Long Auto - NewCore v1.0.6 MainThread Proof

Muc tieu duy nhat cua ban nay: chung minh duong hook-message dang thuc thi tren Unity managed main thread. CHUA co game action.

## Cach chung minh

Bridge van chi nhan toi da 1 command/client. Sau khi Hook + IL2CPP metadata PASS, bridge goi cac getter read-only:

1. `il2cpp_thread_current()` phai co gia tri. Khong `il2cpp_thread_attach`.
2. `System.Threading.SynchronizationContext.Current` phai ton tai.
3. Class hien tai phai la `UnitySynchronizationContext`.
4. Doc `UnitySynchronizationContext.MainThreadId`.
5. Doc `Thread.CurrentThread.ManagedThreadId`.
6. Hai managed thread ID phai trung nhau.

Neu bat ky buoc nao khong dung -> fail-closed, MainThread giu LOCKED.

Luu y: Unity `MainThreadId` la managed thread ID, KHONG phai Win32 TID. Vi vay v1.0.6 khong so sanh no truc tiep voi `GetCurrentThreadId()`.

## Nguyen tac kien truc van khoa

Resolver -> Read-only Scanner -> Snapshot/State Store -> Observer -> FSM -> Safety Guard -> ActionQueue(max 1) -> MainThread Dispatcher -> Internal Action Engine.

Ban nay moi chi chung minh MainThread Dispatcher transport. Khong `CreateRemoteThread`, khong `il2cpp_thread_attach`, khong ClickNPC/Sell/Heal/Revive/AutoFight, khong Sleep de doan state.

`runtime_invoke` chi duoc phep trong phase nay cho getter read-only phuc vu proof, va chi sau khi `il2cpp_thread_current()` xac nhan hook thread da la managed thread cua game.

## Test

1. Giai nen vao folder moi.
2. Chay `build.cmd`.
3. Chi tiep tuc neu `BRIDGE SELFTEST PASS` va `BUILD + LOADLIBRARY SELFTEST THANH CONG`.
4. Chay `dist\\ThanLongAutoTrain_NewCore_v1.0.6.exe`.
5. Tick client -> `Chung minh nen`.
6. PASS mong doi: `MainThread = PROVEN M...` va log co `MAINTHREAD PROVEN`.
7. Neu FAIL: gui nguyen log. Khong spam nut, khong thu action khac.
