# First runtime test (do this before migrating any donor feature)

1. Build `build.cmd`; keep the EXE and `ThanLongNewCoreBridge.dll` in the same `dist` folder.
2. Start one game client and enter a normal map. Do not test multi-client first.
3. Run NewCore as Administrator, scan, tick that client, press Start.
4. Required result: `MainThread = UnitySync`. If it says `NO`/`FAULTED`, stop there; the bridge blocks mutation by design.
5. If UnitySync is proven, verify only the migrated AUTO -> Đánh quái / Dừng cycle first.
6. Let that minimal cycle run repeatedly/for a long session before enabling any sell/heal/revive/buff donor migration.

Do not bypass SAFE PAUSE or main-thread proof to make a failed test “work”; that would recreate the v0.9.0 failure mode.
