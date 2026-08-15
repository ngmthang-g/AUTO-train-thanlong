# Thần Long - Auto Ra Bãi v1.0.0

Bản tối giản được dựng riêng cho đúng client đã phân tích.

## Chỉ có một luồng

`chết -> Đầu thai -> lên ngựa -> AutoPath tới MapID/X/Y đã lưu -> tới bãi -> xuống ngựa -> đứng yên`

Không target quái, không đánh quái, không AUTO combat, không NPC, không bán đồ, không trị liệu, không buff.

## Sử dụng

1. Chạy tool quyền Administrator.
2. Quét client và chọn nhân vật.
3. Đứng tại bãi muốn lưu, bấm **ĐỌC VỊ TRÍ**.
4. Nhập tên bãi và bấm **LƯU MAP + X/Y HIỆN TẠI**.
5. Tick bãi đã lưu và client cần chạy.
6. Bấm **BẮT ĐẦU TỰ RA BÃI**.

Tool tự đọc MapID/X/Y, IsDeath, Riding, Moving, MapReady/WaitingChangeMap và AutoPath state. Khi ở đúng bãi trong sai số cấu hình, tool dừng AutoPath và xuống ngựa.

## Client khóa cứng

Build kiểm tra PE timestamp/SizeOfImage và byte signatures của các hàm route/mount/revive/UI trước khi chạy. Sai GameAssembly sẽ fail closed.

## Nguồn donor

Bản này cố ý dựa trên execution path của Legacy v0.9.0 đã có AutoPath/ngựa/Đầu thai, nhưng cắt bỏ toàn bộ nghiệp vụ không liên quan khỏi state machine và giao diện.
