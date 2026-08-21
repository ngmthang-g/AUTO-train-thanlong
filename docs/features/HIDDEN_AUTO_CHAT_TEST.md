# Hidden Auto Chat TEST

Status: **SOURCE READY / RUNTIME UNTESTED**

Purpose: isolated prototype to validate background chat input before integrating any chat feature into the main automation tool.

## Scope

Standalone executable only. It does not modify the main controller/bridge protocol.

UI:

- scan visible game windows whose process contains `GameAssembly.dll`;
- select one game PID/window;
- enter chat text;
- configure repeat interval in seconds;
- configure repeat count;
- Start / Stop.

## Current sender

The first test version intentionally uses a narrow background-window-message path:

`PostMessage(WM_KEYDOWN/WM_KEYUP VK_RETURN) -> PostMessage(WM_CHAR text) -> VK_RETURN`

It explicitly does **not** use:

- `SetForegroundWindow`;
- `SendInput`;
- cursor movement/clicks;
- clipboard paste;
- `CreateRemoteThread`;
- modifications to the existing NewCore bridge.

This mechanism is experimental because Unity may reject or ignore keyboard/character window messages while the client is not foreground/focused. Runtime testing is therefore required.

## Semantic/network research state

Frozen-client knowledge already confirms:

- `CMD_CLIENT_CHAT = 100008` exists;
- `LuaSystemAPI_Network.SendPacket` bridges to `LuaSystemManager.SendPacketToServer`;
- the client has a Lua/game semantic action layer and should ultimately prefer it over physical input;
- exact `CMD_CLIENT_CHAT` payload/channel/target serialization is **not yet VERIFIED** in the knowledge base.

A targeted `Interface.unity3d` Chat/Lua probe is being kept separate from this prototype. Do not invent the packet payload.

## Acceptance test

Run the standalone tool with the game visible but not active. Select the target client, enter a harmless test message, interval >= 5 seconds, repeat 1 or 2 times.

PASS requires all of the following:

1. the selected game sends the expected chat text;
2. the game does not need to become foreground;
3. the user's mouse remains untouched;
4. typing in another application is not stolen or injected;
5. Start/Stop and repeat count behave correctly.

If chat text is not sent while the game is unfocused, mark this path **RUNTIME FAIL** and replace the sender with the verified Lua/network route after exact Chat payload/handler proof.

## Files

- `src/chat_test/main.cpp`
- `build-chat-test.cmd`
- feature-branch CI artifact: `ThanLongHiddenAutoChat_TEST.exe`
