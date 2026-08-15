# Thần Long Auto - NewCore v1.0.8

## 2026-08-15 CI/source-integrity repair

GitHub Actions run #26 failed at the Build step because the first connector upload corrupted/truncated the two large C++ translation units on the remote branch. The NewCore runtime design was not the cause.

The source is now stored as ordered `.inc` fragments with tiny `.cpp` wrappers so each GitHub transport payload remains small and SHA-verifiable. `SOURCE_INTEGRITY_v1.0.8.md` records the intended full-source hashes and every fragment hash. `build.cmd` audits `.inc` files as well as `.cpp/.h`, so the transport split cannot bypass architecture rules.

No gameplay action was added or unlocked by this repair.

## Functional changes

- Continuous 500 ms read-only scanner.
- MapReady/WaitingChangeMap are read before LeaderRoleData/AutoPath.
- During map transition, deeper object reads are skipped.
- Two consecutive stable scans are required after a real transition.
- Riding, Moving and AutoPath states are observed.
- AutoPath resolves through metadata: `FGStudio.Engine.Logic.AutoPathManager -> get_Instance() -> get_IsAutoPathing()`.
- State-edge observer logs meaningful transitions without spamming position/HP changes.
- Three consecutive scanner failures fail closed.
- Gameplay actions remain LOCKED.

## Legacy donor boundary

`Legacy v0.9.0` means the old pre-NewCore auto architecture. It is donor-only for verified names, data, state semantics, guards, NPC/map/item/UI/Lua/ACK knowledge. Its executor/worker architecture is not part of NewCore.
