# CHANGELOG

## v1.5.0 — 2026-08-16

### Added
- 180-second train Map/X/Y check cadence after AUTO→Đánh quái becomes active.
- Travel fallback: mount #1 → wait 5s → mount #2 → wait 5s → walk up to 15s → repeat.
- Shared travel fallback for train correction, Auto Sell trip, and return to train.
- Bounded automatic Confirm retries for cross-map and post-Đầu-thai flows.
- `PROJECT_KNOWLEDGE.md` and this changelog.

### Changed
- Train deviation no longer triggers route correction every controller tick while stable fighting is active.
- Cross-map confirm state is retained through a retry window instead of being released too early.

### Fixed
- Automatic Confirm flow now retries despite manual Test already succeeding.
- Mount failure can no longer leave correction/sell travel waiting indefinitely for riding state.

### Build
- GitHub Actions run `31907163682`: PASS (audit, Route FSM 8/8, bridge/EXE build, package, verify, upload).

### Known Issues
- Runtime test is still required for the 3-minute cadence, walking fallback, and automatic Confirm timing.
- Sell UI remains coordinate-macro based.

## v1.4.2 — 2026-08-16

### Changed
- Removed all default sell-NPC X/Y values.
- Added user-entered/captured NPC positions shared by NPC preset.

### Runtime Findings
- Six fixed click points: manual capture/test PASS.
- Automatic `XÁC NHẬN RA MAP`: real flow reported unreliable despite manual TEST PASS.

## v1.4.1 — 2026-08-16

### Deprecated
- Hard-coded NPC X/Y attempt. Replaced by v1.4.2 user-configured coordinates.

## v1.4.0 — 2026-08-16

### Added
- Foreground mouse-lock input, F4 pause, six click slots, BagSpace-based Auto Sell, ClickNPC, dynamic sell macro.
