# Route-only audit v1.0.0

Runtime Worker được phép gọi duy nhất các nhóm action:

- `ClickInternalDauThai` khi `IsDeath=true`;
- `ClickInternalConfirm` khi route làm xuất hiện MessageBox chuyển map;
- `ToggleRide(true/false)`;
- `StartPathTo` / `StopPathOnly`.

Worker không được gọi combat, target, skill, NPC, sell, treatment hoặc buff. `build.cmd` trích riêng body của `Worker()` và fail build nếu thấy token action ngoài scope.

Revive/UI resolver dùng direct read `UIObject.instances` Dictionary layout từ donor (`DictionaryEntries`, `DictionaryCount`, `EntryValue`) thay vì managed `GetEnumerator/MoveNext` đã gây timeout ở NewCore v1.2.1.

Client guard: PE timestamp `0x6A410C14`, SizeOfImage `0x03DCB000`, cộng byte signatures cho role/map/dead/riding/mount/MapReady/MessageBox/UIButton/AutoPath.
