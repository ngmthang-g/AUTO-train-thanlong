# AUDIT v0.8.4

- Built-in NPC IDs are sourced from client Config, not inferred from RoleID.
- AUTO start/stop use exact TopIcon Lua actions from Interface asset.
- `EnableAutoF1` verification semantics corrected: Train => false, stopped => true.
- Auto chat opens through exact BottomIcon Lua action and is gated to stable train position.
- AUTO-stop -> navigation has a 700 ms quiet barrier so Lua/UI and AutoPath/mount/portal commands are not issued in the same worker turn.
- Cross-server target IDs: 10005 / 10007 / 10004.
- C++17 `-Wall -Wextra -Werror` syntax check: PASS in validation environment.
- Clang Static Analyzer: 0 diagnostics.
- `remote_worker.S` x86-64 Windows COFF assembly: PASS.
