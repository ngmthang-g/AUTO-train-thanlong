# v0.8.7 build fix

GitHub/zig `-Wall -Wextra -Werror` exposed a source-generation regression in the first v0.8.7 package.
While replacing the old multi-item selling loop with the new "second visible Equipment slot / 90 x 1 second" routine, the replacement range accidentally removed the shared member-helper block immediately following `TrySellAtNpc`.

Restored from the last intact v0.8.6 baseline without changing the new v0.8.7 selling routine:
- `UpdateTransitionState`
- `RefreshLive`
- `UpdateFreeBagSpace`
- `UpdateLive`
- mount helpers
- target/skill helpers
- buff helpers
- chat/UI helpers including `SendChatViaUiInternal`

The old `CollectSafeBagItems` / old `TrySellAtNpc` implementation was NOT restored.
The v0.8.7 selling behavior remains: skip visible equipment slot 1, re-resolve visible slot 2 each tick, 90 ticks, 1000 ms between ticks, then close shop/bag and return to auto.
