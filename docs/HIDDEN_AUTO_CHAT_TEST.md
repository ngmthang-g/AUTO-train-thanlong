# Thần Long Hidden Auto Chat - TEST

Standalone Windows x64 test utility for validating background chat delivery before integrating chat into the main automation tool.

## Features

- Scan visible game windows whose process has `GameAssembly.dll` loaded.
- Select one game PID/window.
- Enter chat text.
- Configure repeat interval in seconds.
- Configure repeat count.
- Start / Stop.
- Does not call `SetForegroundWindow`.
- Does not use `SendInput`, mouse movement/clicks, or clipboard paste.

## Current sender

The first runtime test uses a background window-message path:

`WM_KEYDOWN/UP(VK_RETURN) -> WM_CHAR(text) -> WM_KEYDOWN/UP(VK_RETURN)`

This is intentionally isolated from the main tool. It is a compatibility probe, not the final semantic chat implementation.

Known protocol evidence from the client-data knowledge base:

- `CMD_CLIENT_CHAT = 100008` exists.
- Lua network API bridges `Network.SendPacket(...)` to `LuaSystemManager.SendPacketToServer(...)`.
- Exact chat payload/handler is still not considered verified, so this test does not invent or hard-code a packet body.

## Build

Requires Zig 0.15.2 on Windows or a host capable of Zig Windows cross-compilation.

Run:

`build-chat-test.cmd`

Output:

`dist/ThanLongHiddenAutoChat_TEST.exe`

GitHub Actions also builds and verifies the EXE.

## Runtime pass criteria

1. Start the game and leave another application focused.
2. Launch `ThanLongHiddenAutoChat_TEST.exe`.
3. Scan and select the target game PID.
4. Enter a unique message, interval, and repeat count.
5. Start.
6. Confirm the game sends exactly the expected messages while the foreground application keeps focus and mouse/keyboard input is unaffected.

If this background-message path fails in the real client, the next implementation step is to replace only the sender with the verified Lua/Network semantic path after the exact chat handler/payload is recovered.
