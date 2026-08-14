# AUDIT v0.8.7

- Added `UIButton.get_ClickHandler()` RVA `0x52DF50`; binary signature is a direct `[this+0x100]` string getter.
- AUTO production path no longer invokes `TopIcon.AutoTrainClick` out of button-event context. It replays `AutoFightClick -> AutoTrainClick` using fresh active UIButton instances.
- Stop path replays `AutoFightClick -> AutoStopClick`.
- AUTO navigation remains isolated from combat callbacks.
- NPC treatment/shop waits use 35 ms state polling instead of 140–150 ms polling plus long blind sleeps.
- Shop `ButtonItemClicked` identification is grounded in extracted `ItemBox_Layout` and constrained to `SellItemTab` ancestry.
- Selling no longer iterates every visible ItemBox. The first visible equipment ItemBox is preserved; the second visible ItemBox is re-resolved in `ItemsGrid` hierarchy order once per second for exactly 90 timing iterations. `GetFreeBagSpace` is informational only and never determines completion.
- Built-in NPC IDs remain Mã Kiêu Minh=373 and Đỗ Thanh Đằng=339.
- Cross-server target IDs remain 10005 / 10007 / 10004.
- `remote_worker.S` assembles as x86-64 Windows COFF; arg5 remains at Win64 stack slot `[rsp+0x20]`.
- Full Windows C++ link must be executed by `build.cmd`/GitHub Actions because this Linux container has no Zig Windows SDK/linker.

## v0.8.7 sell-slot audit
- `BagItemsGrid:InitializeComponents()` creates ItemBox entries in position order; `DoFilter()` hides non-equipment entries and calls `ItemsGrid:RebuildLayout()`.
- Therefore visual slot #2 is the second active ItemBox in `ItemsGrid` hierarchy order, not a stable UIButton pointer. v0.8.7 re-resolves that second visible ItemBox before every 1-second sell action.
- The first visible equipment slot is never invoked. Exactly 90 timing iterations are executed before shop/bag close.
