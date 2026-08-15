# v1.0.12 - SafetyGuard + ActionQueue(MAX=1) + FSM dry-run scaffold

- Protocol bumped to `0x0001000C`.
- Added one per-client control FSM state separate from scanner foundation state.
- Added SafetyGuard precondition evaluation over proven snapshot/main-thread/scanner/map identity state.
- Added `ActionQueueScaffold` with compile-time capacity exactly 1.
- Added dry-run `Revive` intent to exercise decision -> guard -> queue without gameplay mutation.
- Added compile-time `kGameplayMutationEnabled=false` plus build audit and static_assert.
- Map transition, partial observer, scanner failure and faults clear queue / fail closed.
- Dispatcher gate consumes dry-run envelopes and emits no gameplay BridgeCommand.
- Bridge gameplay surface remains unchanged; scanner stays read-only.
