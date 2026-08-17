# BUG-006 — Startup scan attached bridge and could diss client

## Runtime evidence
First real v1.5.10 test: merely opening the EXE / scanning a live client caused the game to disconnect/crash before automation was started.

## Root cause
`ScanClients()` had regressed from passive enumeration into `Bridge.Attach()` + immediate `ReadState`. In v1.5.10, `ReadState` also called the v0.8.7 raw-RVA MessageBox observer on every snapshot, so donor UI code could run during startup/ordinary polling.

## Fix
- Scan is passive only: EnumWindows + GameAssembly module detection.
- Explicit Start is the first allowed bridge attach point.
- Core `ReadState` never touches donor UI RVAs.
- Added dedicated `ProbeInternalConfirm` command, called only for tool-owned cross-map routes after authoritative `moving=0`.
- Revive donor scan remains gated by authoritative death state.

## Validation boundary
CI/build can prove the separation statically, but only a live client can prove the diss regression is gone. First runtime test must be: open tool -> Scan only -> wait; then Start one account and observe core snapshot before testing cross-map/revive.
