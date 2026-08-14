# Thần Long Auto - NewCore v1.0.3 Foundation Validator

Bản này là **Phase 1 read-only** của NewCore. Mục tiêu duy nhất: xác minh bridge/hook và bóc đúng metadata của hai type liên quan main-thread trước khi mở bất kỳ action game nào.

## Luật cứng

- Không `CreateRemoteThread`.
- Không `remote_worker.S`.
- Không `il2cpp_thread_attach` để biến worker thành "main thread" giả.
- Không `runtime_invoke` trong v1.0.3.
- Không AUTO / Sell / Heal / Revive / ClickNPC.
- Một client = một session = tối đa một probe đang chạy.
- Timeout => fail-closed, không retry spam.
- Nếu HWND/TID thay đổi sau scan, session cũ bị bỏ; không giữ hook/thread stale.

## Chạy test

1. Build bằng `build.cmd` với Zig 0.15.2.
2. Mở game và vào nhân vật.
3. Mở tool, tick đúng client.
4. Bấm **Kiểm tra nền**.
5. Chờ trạng thái `VALIDATOR PASS` hoặc `VALIDATOR FAIL`.
6. Gửi lại toàn bộ log ở ô dưới cho bước tiếp theo.

`MainThread` cố ý hiển thị `LOCKED` trong bản này. PASS validator **không có nghĩa** action đã được phép chạy; nó chỉ cho phép chúng ta xác định chính xác dispatcher/main-thread mechanism tiếp theo.
