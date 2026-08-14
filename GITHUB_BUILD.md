# Build bằng GitHub Actions

Repo đã có workflow: `.github/workflows/build-windows.yml`.

## Cách dùng

1. Đưa **toàn bộ nội dung thư mục này** lên root của GitHub repository. Quan trọng: `.github` phải nằm ngay ở root repo.
2. Mở tab **Actions** trên GitHub.
3. Chọn **Build Windows EXE**.
4. Bấm **Run workflow**.
5. Khi job xanh, mở workflow run và tải artifact `ThanLongAutoTrain-v0.7.0-windows-x64`.
6. Giải nén artifact để lấy `ThanLongAutoTrain_v0.7.0.exe`.

Workflow cũng tự chạy khi push thay đổi vào `src/`, `resources/`, `build.cmd` hoặc chính workflow trên nhánh `main` / `master`.

## Toolchain

Workflow dùng Windows runner và tải Zig 0.15.2 x86_64 trực tiếp từ `ziglang.org`, sau đó gọi chính `build.cmd` của project.
