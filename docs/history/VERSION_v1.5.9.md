# VERSION v1.5.9 — Auto Rotate Train Spots

## Requested behavior
- If an account dies more than 10 times inside 10 minutes, immediately move its train target to the next selected train spot.
- Allow 1, 2, 3, 4, 5, 6 or more selected spots from the existing shared data.
- One selected spot means stay there.
- Multiple selected spots rotate continuously and wrap back to spot 1.
- Also rotate when productive training reaches 15 minutes without a FULL-bag event.

## Implementation
- Added per-account checkbox rotation pool over shared train spots.
- Added persisted, configurable death limit, death rolling window and no-FULL-bag productive minutes.
- Death history and productive-train counters live outside RuntimeState so the v1.5.6 death-session hard resets do not erase rotation evidence.
- Death trigger is recorded only on a new authoritative death edge.
- No-FULL-bag timer accumulates only while alive, at the current target, AutoFight ON and FreeBagSpace authoritative.
- A new FULL-bag edge resets the productive no-FULL timer.
- Live efficiency rotation reuses `BeginTrainRecovery` to stop combat before traveling to the new spot.
- Death-triggered rotation changes the target before revive; normal post-revive cold-start routing then goes to the new target.
- v1.5.8 periodic saved-coordinate Confirm remains unchanged.

## Validation
Windows x64 staging CI **PASS** — run `31949891312`, job `95171640279`, artifact `9264382136`.
- CleanRoute audit PASS.
- Existing route/mount self-test: **15/15 PASS**.
- Rotation self-test: **8/8 PASS**.
- Bridge PE/DLL verification PASS; controller EXE build PASS; package/artifact verification PASS.
- Windows compile controller SHA256: `11739f9d11d056107d6733a34f7d88ae60008a0c9ace2d2eaca97409a4831bbf`.
- Canonical LF controller SHA256: `5900c02b44a62d72ba2e197cd92e8cf6736245d70bd355b9f72f331f7516d559`.
- EXE SHA256: `d6bcc8fd9e02ae818499d776498877ee51099854633556febd94ee945ef8d2bc`.
- Bridge DLL SHA256: `5d7adcb842378cdd91225ee476efb5e5e8dbe4bf67929af7afc97af6eb951a24`.
- Source ZIP SHA256: `7c1297f717ebb880932d83cccb90e0fc9236b49762ab8b14b1cc288ee37b3b1e`.
- Uploaded artifact ZIP digest: `efc36d0fb66593370b49e92e63bfabe67ce09edbbd70a34488ce6c6920ce807d`.

Runtime rotation behavior remains **NEEDS USER TEST**; CI PASS is not runtime PASS.
