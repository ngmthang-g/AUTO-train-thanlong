# Than Long Clean Route v1.5.8 — Fixed Interval Confirm Click

## Automatic map Confirm
`Auto XN map` no longer detects or waits for a MessageBox. Each running account clicks its saved **XÁC NHẬN RA MAP** coordinate on a fixed timer. Default: **5 seconds**; configurable **1–300 seconds** and persisted per RoleID in `%LOCALAPPDATA%\ThanLongCleanRoute\ThanLongCleanRoute.accounts.ini`.

The timer starts from zero after Start/runtime cold reset. If it becomes due while another flow owns the client/mouse, it remains overdue and executes on the first safe tick. Ordinary AutoPath travel does **not** block this timer.

Periodic Confirm yields while: F4 pause, map transition, Transition Freeze/read failure/unresponsive client, death/revive, M87 stop-Auto sequence, route-ownership reset, AutoFight click sequence, sell UI/click sequence, recovery click phase, or a recent REAL INPUT from any account. A global 1000 ms click guard prevents multiple accounts/flows from fighting over the mouse.

## Bridge
Core ReadState no longer calls `MainFindUI/FindUI("MessageBox")`. Map Confirm does not depend on `ValidConfirmUi`, popup visibility, route ownership, movement, portal detection, or semantic UI state.

## Other active behavior
- Map 87 Địa Phủ AutoFight-OFF guard.
- Transition Freeze + serialized Bridge requests.
- dual-boundary death-session cold restart.
- route ownership cleanup.
- 60-second AutoFight watchdog and 3-minute train-coordinate guard.
- Auto Sell, shared spots, six saved click coordinates, persistent per-account settings.

## Validation
Windows x64 staging CI **PASS** — run `31942843638`, job `95154396882`; route/mount self-test **15/15 PASS**. Runtime fixed-interval Confirm is **NEEDS USER TEST**.
