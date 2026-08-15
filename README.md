# Thần Long Auto - NewCore v1.0.7 Read-Only Snapshot

Mục tiêu của bản này: **chứng minh Scanner + Snapshot/State Store**, chưa mở bất kỳ action game nào.

## Luồng bắt buộc

`Hook -> IL2CPP foundation -> MainThread proof -> ReadGameSnapshot -> State Store`

Mỗi client chỉ có tối đa **1 bridge request đang chạy**. Timeout là fail-closed, không retry spam.

## Snapshot đang thử đọc

- Character name (nếu getter có sẵn)
- RoleID
- MapID
- X/Y nếu resolve được bằng metadata/getter hoặc managed `roleData` backing object
- HP / MaxHP
- Dead
- Riding
- AutoFight state (đọc `get_EnableAutoF1`, giữ semantics donor: false = train đang bật)
- Free bag space
- MapReady
- WaitingChangeMap (nếu query có sẵn)
- Moving (nếu query có sẵn)

**Không dùng donor RVA/offset cho snapshot này.** X/Y không resolve được thì hiển thị `?`, không fallback sang `+0x50/+0x54/+0x58`.

## Luật kiến trúc vẫn khóa cứng

- Không `CreateRemoteThread`
- Không `il2cpp_thread_attach`
- Không worker action trong game
- Không `Sleep()` để đoán state
- Không ClickNPC / HandleClickEvent / StartAutoFight / Sell / Heal / Revive
- Chỉ `runtime_invoke` query/getter read-only sau khi Unity main thread đã được chứng minh
- Scanner không tự quyết định hành động
- Action game vẫn **LOCKED**

## Build/Test

1. Giải nén vào folder mới.
2. Chạy `build.cmd`.
3. Build phải PASS cả Architecture audit và Bridge LoadLibrary self-test.
4. Chạy `dist\ThanLongAutoTrain_NewCore_v1.0.7.exe`.
5. Tick client -> **Kiểm tra nền + Snapshot**.
6. Gửi log từ `ReadGameSnapshot` nếu Snapshot không PASS hoặc X/Y vẫn `?`.

Bản tiếp theo chỉ được chuyển sang continuous scanner sau khi one-shot snapshot này ổn định.
