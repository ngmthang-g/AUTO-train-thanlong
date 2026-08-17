# BUG-006 — v1.5.10 startup scan diss after donor UI integration

## Runtime evidence
First real v1.5.10 test: merely opening the EXE / scanning a live client caused the game to disconnect/crash before automation was started.

## Corrected root-cause analysis
A direct comparison against the packaged v1.5.9 source changes the initial diagnosis:

- v1.5.9 already performed `Bridge.Attach()` + immediate core `ReadState` inside `ScanClients()` and was runtime-usable.
- v1.5.9 deliberately kept MessageBox/UI probing OUT of `ReadState`; `ValidConfirmUi` stayed unset.
- v1.5.10 added the v0.8.7 raw-RVA MessageBox observer to every `ReadState`, so startup scan immediately entered donor UI code even when no portal/Confirm context existed.

Therefore the strongest new regression is the unconditional donor UI probe in generic `ReadState`, not the pre-existing bridge hook by itself. Passive scanning is retained as an additional isolation/safety improvement so future runtime failures can be localized cleanly.

## Fix
- Scan is now strictly passive: EnumWindows + GameAssembly module detection only; no bridge attach and no `ReadState`.
- Explicit Start is the first allowed bridge attach point.
- Core `ReadState` never touches donor UI RVAs.
- Added dedicated `ProbeInternalConfirm` command; donor MessageBox UI is touched only for a tool-owned cross-map route after authoritative `moving=0`.
- Revive donor scan remains gated by authoritative death state.
- Static build audit rejects any future bridge attach/read inside `ScanClients()` and rejects donor Confirm polling inside generic `ReadState`.

## Diagnostic test ladder
1. Open tool and Scan only. If the client dies here, the remaining cause is outside bridge/donor UI because this stage is passive.
2. Start exactly one account. If it dies here, isolate the hook/core IL2CPP `ReadState` path; donor Confirm UI is still untouched.
3. If Start is stable but cross-map Confirm dies, isolate the donor MessageBox/UIButton path.
4. If cross-map is stable but death/revive dies, isolate the donor Revive UIButton enumeration/callback path.

## Validation boundary
Windows CI proves compilation, self-tests, static separation, and artifact construction. It cannot prove the live client no longer disconnects. Runtime PASS requires the staged test ladder above.