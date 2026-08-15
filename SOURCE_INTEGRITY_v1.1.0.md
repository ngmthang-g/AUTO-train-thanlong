# Source integrity - NewCore v1.1.0 Integrated Acceptance + Harmless Hook Action Envelope

Clean parent: `agent/newcore-v1.0.12-control-scaffold` / `b530def4f325b2f153f99480436f6c57731ee1a2`

Final publication must remain exactly **one squashed commit** over the v1.0.12 base.

## Intended implementation blobs before final squash

- `src/common/protocol.h` `509b95081841f0b9757bc878bcf32302e38f22d8`
- `src/bridge/action_capability_probe.inc` `5d4c132410b5c60d09b250f4d2ef4b62fb97e2fd`
- `src/bridge/harmless_action_envelope.inc` `93e629d615cf67075f5b77ff4034103f8589212d`
- `src/bridge/bridge.cpp` `ce395cd1c3b4d83b55e987c83232582442c25e13`
- `src/bridge/bridge_part_08.inc` `f1a8136ed5b28d88870f097953015a99eb9f4237`
- `src/controller/control_scaffold.h` `87d201a761859dba02699c496ae31f4a3162878a`
- `src/controller/main_part_01.inc` `dc9257765b1492dfeef04cd75b3a4c8e33cbba4d`
- `src/controller/main_part_03.inc` `2ff4dfe36e20f44e963586b781c812c3749d9ce4`
- `src/controller/main_part_05.inc` `02de54f7405ddeb62ddcb0a29cc18ae9f498f2d4`
- `src/controller/main_part_07.inc` `094c9748f5eb12ae03e59596839d4c5161eedeb2`
- `src/controller/main_part_11.inc` `da5a3398062c9eca9e19e8ac6444e5c71db2c769`
- `build.cmd` `182c426cf785b0ee3dd20cae18fb42e58c2a16e0`
- `README.md` `a5bcb3fda75734afb5d50dc0f86a8b06ec2d749e`
- `CHANGELOG_v1.1.0.md` `1179f0addaa5e691963ba3a0e35dada092958722`
- `BUILD_STATUS_v1.1.0.txt` `3d7abaabd2333736a02ef6ec2109b4a067d08d50`
- `STATIC_AUDIT.txt` `b4bd22670897eaa09cd15f882c97308d2b15874e`
- `docs/ACTION_CAPABILITY_EVIDENCE.md` `766233da786a435e6e495b56b980edadbc2a8396`

## Protocol boundary

Protocol is now `0x00010102` because `BridgeResponse` gains `InfrastructureProofSnapshot`. `ReadGameSnapshot=5` is unchanged. `ProveHookActionEnvelope=6` is the only added command and is infrastructure-only.

Command 6 may mutate only `g_harmlessProofSequence`, a bridge-local monotonic counter. It may not resolve or call gameplay UI/NPC/combat/path/inventory/shop methods.

## Controller boundary

Harmless proof uses the same ActionQueue capacity=1:

`SafetyGuard PRE -> TryEnqueue -> BeginNext(active) -> command 6 -> token/sequence/thread POST -> CompleteActive`

Queue remains active while command 6 is outstanding. POST mismatch fails closed and clears the bridge/queue.

`kGameplayMutationEnabled=false` remains unchanged. Gameplay DispatcherGate still rejects all gameplay intents.

## Runtime evidence boundary

The bridge reuses `ProveUnityMainThread()` in command 6. The returned callback thread must equal the PRE hook thread; managed current/main IDs must equal their PRE values and each other.

This proves the command envelope can execute and complete on the proven hook/Unity-main-thread path. It does **not** prove Revive/NPC/AutoFight/Sell mutation.

## Resolver boundary

Probe v3 retains 30/30 required IL2CPP foundation exports. Optional class enumeration stays isolated to metadata action discovery. Prior runtime-observed MainThread/UnityMainThreadDispatcher signatures remain metadata gates only.

Publication rule: final tree must use the intended blobs above, then GitHub Actions Windows/Zig must pass on the final squashed commit before an artifact is delivered.
