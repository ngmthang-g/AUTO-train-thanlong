# AUDIT v0.8.9

## Removal audit

Các symbol/đường cũ sau đã được grep toàn bộ `src/main.cpp` và đều có 0 kết quả:

`ChatPing`, `ClickGamePoint`, `TypeUnicode`, `CalibrateChat`, `CapturePoint`, `AutoChat`, `autoChat`, `OpenAutoRootButton`, `FindChatOpen`, `FindChatPanelAction`, `InvokeRectLua`, `UIRectTransform`, `UIInput`, `NavigationMode`, `NormalizedPoint`, `RectLua`, `PointerEventData`, `WM_MOUSEMOVE`, `WM_LBUTTONDOWN`, `WM_LBUTTONUP`, `WM_CHAR`.

RVA/signature của `UIInput.set_Text` và `UIRectTransform.get_OnPointerClickHandler`, class caches/scanners tương ứng cũng đã xóa.

## Build/static checks

- `clang++ -std=c++17 -Wall -Wextra -Werror -fsyntax-only` với Win32 declaration stubs: PASS.
- `src/remote_worker.S` assemble target `x86_64-pc-windows-msvc`: PASS, output COFF x86-64.
- 62/62 byte signatures còn lại trong `Validate()` khớp đúng `GameAssembly(1).dll` của client được cung cấp.
- Không còn namespace-scope RVA constexpr bị khai báo mà không dùng.
- Source giảm từ 5626 dòng (v0.8.8 upload) xuống 4895 dòng trước bước tài liệu, chủ yếu do loại code chat/UIRect/dead path.

## Lưu ý còn lại

`PostMessageW` vẫn còn đúng một mục đích: Ctrl+Tab fallback của chế độ đánh bằng skill nếu `SelectTarget(RoleID)` nội bộ thất bại. Nó không thuộc các tính năng user yêu cầu xóa trong v0.8.9. Không còn mouse-message hay `WM_CHAR` trong source.
