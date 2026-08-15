# Thần Long Auto - NewCore

This repository is a clean NewCore rebuild. Legacy v0.9.0 means the old pre-NewCore auto and is used only as a donor of verified data/semantics, never as an executor architecture.

## v1.0.8 current gate

v1.0.8 implements a continuous read-only Scanner/State Store/Observer layer. Gameplay actions remain locked.

Pipeline remains fixed:

`Resolver -> Read-only Scanner -> GameSnapshot/State Store -> Observer -> State Machine -> Safety Guard -> Action Queue (MAX=1) -> Main Thread Dispatcher -> Internal Action Engine`

Current scanner observes stable-map state including RoleID, MapID, position when metadata-resolved, HP/MaxHP, dead, riding, moving, auto-fight, free bag space and AutoPath. `MapReady` and `WaitingChangeMap` are always read before deeper Leader/AutoPath objects; during a map transition deeper reads are skipped and recovery requires two stable snapshots.

## Source integrity note

The GitHub connector corrupted the first large single-file upload of `bridge.cpp` and `controller/main.cpp`, causing GitHub Actions run #26 to fail in the Build step. The repaired branch stores those two translation units as ordered `.inc` source fragments plus tiny wrapper `.cpp` files. This is a transport/storage workaround only. Fragment hashes and intended full-source hashes are pinned in `SOURCE_INTEGRITY_v1.0.8.md`.

`build.cmd` audits `.cpp`, `.h`, and `.inc` for forbidden old-architecture/action tokens before compiling.

## Golden rules

See `docs/GOLDEN_RULES.md`. In short: scanner only reads; no `CreateRemoteThread`/remote gameplay worker; no `il2cpp_thread_attach`; no Sleep-driven state machine; no long-lived UI pointers; metadata-first resolution; action only after PRE validation, one active mutation maximum, proven Unity main thread, real POST/ACK verification, fail-closed unknown state, and per-process context isolation.

## Build

Run `build.cmd` with Zig 0.15.2 on Windows. Successful build must also pass the bridge LoadLibrary self-test. GitHub Actions is the authoritative Windows build for connector-published commits.

Do not unlock gameplay actions until the continuous scanner and map-transition recovery pass live runtime testing.
