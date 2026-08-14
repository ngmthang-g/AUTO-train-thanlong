# AUDIT v0.8.5

- Added `UIButton.get_ClickHandler()` RVA `0x52DF50`; binary signature is a direct `[this+0x100]` string getter.
- AUTO production path no longer invokes `TopIcon.AutoTrainClick` out of button-event context. It replays `AutoFightClick -> AutoTrainClick` using fresh active UIButton instances.
- Stop path replays `AutoFightClick -> AutoStopClick`.
- AUTO navigation remains isolated from combat callbacks.
- NPC treatment/shop waits use 35 ms state polling instead of 140–150 ms polling plus long blind sleeps.
- Shop `ButtonItemClicked` identification is grounded in extracted `ItemBox_Layout` and constrained to `SellItemTab` ancestry.
- Selling uses three fresh active-control passes, 45 ms sequential cadence, at most 90 clicks; inventory is probed after a batch instead of after every click.
- Built-in NPC IDs remain Mã Kiêu Minh=373 and Đỗ Thanh Đằng=339.
- Cross-server target IDs remain 10005 / 10007 / 10004.
- `remote_worker.S` assembles as x86-64 Windows COFF; arg5 remains at Win64 stack slot `[rsp+0x20]`.
- Full Windows C++ link must be executed by `build.cmd`/GitHub Actions because this Linux container has no Zig Windows SDK/linker.
