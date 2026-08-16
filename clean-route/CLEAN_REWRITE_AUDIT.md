# Clean Route v1.5.8 — Fixed Interval Confirm audit

- Saved Confirm coordinate is per-account and persistent.
- `ConfirmIntervalSec`: default 5s, valid range 1–300s.
- Automatic Confirm does not invoke semantic `HandleCrossMapConfirm`.
- Bridge ReadState does not call `ReadSemanticUiPresent` or `MainFindUI`; MessageBox detection is not required by this feature.
- Transition/WaitingChangeMap/Freeze/read failure blocks periodic Confirm.
- Death/revive, M87 stop-Auto, route-ownership reset, active AutoFight click sequence, sell click/UI phases and recovery click phases block periodic Confirm.
- Ordinary AutoPath/travel may coexist with periodic Confirm.
- Any successful REAL INPUT records per-account/global click time; periodic Confirm yields at least 1000 ms after another click.
- A due timer is deferred, not discarded, while busy.
- Special revive Confirm injection is removed.
- Existing architecture/safety preserved: serialized Bridge requests, Transition Freeze, death-session cold restart, M87 guard, Auto Sell, shared spots and persistent account profiles.
- No direct item-sell/combat mutation API added.

Windows x64 staging CI **PASS** — run `31942843638`, job `95154396882`; audit PASS, route/mount self-test 15/15, Bridge PE/load/export, EXE build/package/verify PASS. Runtime remains **NEEDS USER TEST**.
