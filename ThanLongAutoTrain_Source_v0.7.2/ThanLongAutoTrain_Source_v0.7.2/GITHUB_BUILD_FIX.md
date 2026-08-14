# GitHub Actions build fix

Lỗi cũ xảy ra vì workflow hardcode `dist/ThanLongAutoTrain_v0.7.0.exe` trong khi `build.cmd` của source cũ tạo tên EXE theo version cụ thể.

Workflow `.github/workflows/build-windows.yml` không hardcode version nữa. Sau build nó tìm đúng một file `dist/ThanLongAutoTrain_v*.exe`, in SHA-256 và upload chính file đó làm artifact.

Nếu repository đang có workflow build cũ, xóa/disable workflow cũ hoặc thay nội dung bằng workflow mới để tránh hai workflow chạy song song.
