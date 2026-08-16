# VERSION v1.5.3 — 2026-08-16

## A. Identity / Lineage
Version: v1.5.3
Based On: v1.5.2
Reason Created: live test showed the map confirmation popup visibly present while CleanRoute remained in `Cross-map • AutoPath đang chạy` and never reached the Confirm branch.

## B. User Runtime Evidence
- Real popup visible: PK warning for entering Cao Xương, with `Xác nhận` and `Hủy bỏ`.
- CleanRoute state at the same time: `Cross-map • AutoPath đang chạy`.
- Automatic Confirm: FAIL in v1.5.2.
- User reports supplied v0.8.7 source confirms maps fastest/reliably.

## C. Root Cause
**CONFIRMED in v1.5.2 controller source:**
`HandleCrossMapConfirm` returns immediately when `s.autoPathing` is true, before consuming `ValidConfirmUi/confirmUiVisible`.

Therefore a client state of:
`AutoPath=ON + moving=OFF + MessageBox=ON`
can never reach the click branch.

This matches the user screenshot exactly.

## D. v0.8.7 Donor Finding
The supplied real `src/main.cpp`:
- polls `MainFindUI("MessageBox")` while map state is stable;
- does not require AutoPath OFF for portal confirmation readiness;
- once portal confirmation is ready, calls `StopPathOnly()` and immediately performs Confirm;
- waits for actual map transition afterward.

Core MessageBox/Confirm functions are effectively the same in supplied v0.8.7 and v0.9.0. The useful new evidence is the **ordering around AutoPath ON**, not a different UI API.

## E. v1.5.3 Design
Keep CleanRoute architecture and current foreground saved-coordinate Confirm input.

New order:
1. cross-map route must be tool-owned;
2. MapReady/WaitingChangeMap must be stable;
3. AutoPath ON is recorded as route evidence but no longer causes early return;
4. semantic `MessageBox` is inspected even while AutoPath remains ON;
5. if MessageBox OFF: continue waiting;
6. if MessageBox ON: require authoritative moving state and `moving=OFF`;
7. debounce popup for 200 ms;
8. if Path still ON: issue one serialized `StopPath` action;
9. on a fresh snapshot, after at least 250 ms, if MessageBox is still ON, click the saved Confirm coordinate;
10. even if AutoPath's logical flag remains ON after the explicit StopPath request, do not deadlock again; MessageBox ON + route-owned + moving OFF is sufficient context;
11. then wait for MessageBox OFF / map-transition proof.

## F. Safety / Non-transplant Rule
Not copied from v0.8.7:
- remote worker;
- fixed RVAs;
- direct remote UIButton tree executor;
- broad legacy state machine.

Only the detector/action ordering is carried over.

## G. Build / CI
Windows x64 CI: **PASS**.
- Run: `31927283960`
- Job: `95116711777`
- Artifact: `9258246372`
- Artifact digest: `sha256:835950db3736df85367c1175e71c6b06e0998233143eeff7d191dcc9e35ad028`
- Controller source SHA256: `af6d3299768af1882214ab42e29da8f73c33169eaae55165f9f29d34259e6f4e`
- Bridge source SHA256: `a7f35862224e8174f36182569056871495b5575ca676e03726e8be287420c629`
- EXE SHA256: `a98188a4ce4793ed428a1fa5835bb42dd936ec5d255828bb3c9618083db4166b`
- Bridge DLL SHA256: `81cfafa953a6f7b8a9c95965f79d618b66dd5a28c02a8ac771c08e0a6729eade`
- Source ZIP SHA256: `e24136e592514b28583f806ea32951f23d8f452d7a2a9dddd821096c73f3dcb5`

Artifact inspection confirmed the final source has no Path-ON early return before semantic popup handling; `MainFindUI("MessageBox")` remains first choice and window title is v1.5.3.

## H. Runtime
v1.5.3 automatic Confirm: **RUNTIME UNTESTED**.

Desired live trace:
`Path ON → MessageBox ON → StopPath → fresh snapshot → Confirm click → MessageBox OFF → WaitingChangeMap / MapID changes`.
