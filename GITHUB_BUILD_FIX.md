# GitHub build

Workflow dùng Zig 0.15.2 và chạy `build.cmd`.

`build.cmd` tạo `dist\ThanLongAutoTrain_v0.8.9.exe`.
Bước verify của workflow tìm `dist/ThanLongAutoTrain_v*.exe`, vì vậy không cần sửa workflow mỗi lần tăng version.
