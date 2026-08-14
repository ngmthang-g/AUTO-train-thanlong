# GAME_ANALYSIS v0.8.7

## AUTO controls recovered from Interface.unity3d

`TopIcon` layout contains a real `UIButton Name="ButAutoFight"` whose `ClickHandler` is `AutoFightClick`. The Lua function only toggles `self.AutoFightGroup.Active`.

Inside that group the `Đánh quái` button uses `ClickHandler="AutoTrainClick"`; `AutoTrainClick()` resolves `AutoFight_Main` and calls `StartAutoFight(C_AutoModel.Train)`. The Stop button uses `AutoStopClick`.

This explains the v0.8.4 runtime result: executing `AutoTrainClick` directly through `MonoBehaviourExecutor` was not equivalent to the normal UIButton event context on this client. v0.8.7 therefore invokes the actual fresh UIButton instances in the same order as a user click.

Native `UIButton.get_ClickHandler()` for this supplied GameAssembly is at RVA `0x52DF50` and is the simple getter for the backing field at `this+0x100`. That allows exact handler matching instead of guessing by text.

## Shop ItemBox recovered from Interface.unity3d

`NPCShop_SellItemTab` creates a `BagItemsGrid` on the right side. It installs a custom click callback; when quick sell is selected, clicking an ItemBox calls `RequestSellItem(dbItemData)`, which sends the shop sell request.

The shared `ItemBox_Layout` has:

```xml
<Button>
  <Property Name="ClickHandler" Value="ButtonItemClicked" />
</Button>
```

`BagItemsGrid` creates a fixed pool of 100 ItemBox controls in bag-position order. On server `RemoveItem`, the corresponding box is cleared; Equipment filtering toggles active parents and `ItemsGrid:RebuildLayout()` compacts the visible grid. Therefore the requested “same visual slot #2” is not one stable UIButton pointer: after a successful sale the next active ItemBox moves into visual slot #2. v0.8.7 re-resolves that second visible active ItemBox each second while permanently skipping the first visible slot.

The left shop/buy-back list does not use this same control path for selling; its visible item icon/button handlers differ. v0.8.7 additionally requires `SellItemTab` in the ancestor chain before treating a `ButtonItemClicked` as a bag sell candidate.

## Timing strategy

Speed is increased by polling state more frequently, not by issuing actions concurrently:

- NPC/shop/treatment state poll: 35 ms.
- Sell cadence: exactly 1 second between the 90 slot-#2 attempts.
- Short server quiet window between item passes: 70 ms.

This preserves one-action-at-a-time ordering while removing the old 140–260 ms fixed waits.


## v0.8.7 runtime finding: NPC UI must own the session
Runtime showed `Mất phản hồi trạng thái chuyển map` immediately after treatment/shop UI. The worker previously resumed the normal RefreshLive/map/mount loop as soon as the synchronous NPC chain returned, while Lua/server UI teardown could still be in progress. v0.8.7 adds a hard post-NPC quiet window plus the existing two-stable-scan recovery gate.

## Long Phá Thiên preset
Decrypted `Config.unity3d` contains `<NPC ID="463" Name="Long Phá Thiên" .../>` and `<NPCData ID="463" MapID="3" Name="Long Phá Thiên"/>`. The user reports the same NPC template in Lạc Dương Liên Server runtime MapID 10000. Therefore only the ResID/template 463 is built in; Liên Server X/Y is captured from realtime position when the user saves the healer NPC.


## v0.8.7: why the second visible ItemBox must be re-resolved
Recovered `BagItemsGrid` Lua maps items by `dbItemData.Position` to a fixed pool, then Equipment filtering toggles each ItemBox parent Active and calls `ItemsGrid:RebuildLayout()`. `RemoveItem` clears the sold ItemBox and runs the filter again. The next active equipment therefore moves into the same visual grid position, but it is not necessarily the same UIButton object. Holding the old pointer would reproduce the stale-control failure mode. The tool now keeps the pool only for the lifetime of the open shop and resolves the second active ItemBox in hierarchy order for each of 90 one-second attempts.
