# GAME_ANALYSIS v0.8.5

## AUTO controls recovered from Interface.unity3d

`TopIcon` layout contains a real `UIButton Name="ButAutoFight"` whose `ClickHandler` is `AutoFightClick`. The Lua function only toggles `self.AutoFightGroup.Active`.

Inside that group the `Đánh quái` button uses `ClickHandler="AutoTrainClick"`; `AutoTrainClick()` resolves `AutoFight_Main` and calls `StartAutoFight(C_AutoModel.Train)`. The Stop button uses `AutoStopClick`.

This explains the v0.8.4 runtime result: executing `AutoTrainClick` directly through `MonoBehaviourExecutor` was not equivalent to the normal UIButton event context on this client. v0.8.5 therefore invokes the actual fresh UIButton instances in the same order as a user click.

Native `UIButton.get_ClickHandler()` for this supplied GameAssembly is at RVA `0x52DF50` and is the simple getter for the backing field at `this+0x100`. That allows exact handler matching instead of guessing by text.

## Shop ItemBox recovered from Interface.unity3d

`NPCShop_SellItemTab` creates a `BagItemsGrid` on the right side. It installs a custom click callback; when quick sell is selected, clicking an ItemBox calls `RequestSellItem(dbItemData)`, which sends the shop sell request.

The shared `ItemBox_Layout` has:

```xml
<Button>
  <Property Name="ClickHandler" Value="ButtonItemClicked" />
</Button>
```

`BagItemsGrid` creates a fixed pool of 100 ItemBox controls once and reuses them. On server `RemoveItem`, the corresponding box is cleared and the Equipment filter makes empty/non-equipment boxes inactive. This supports a fast three-pass strategy: successful boxes disappear from the next active scan; unsellable boxes remain and are tried at most three times.

The left shop/buy-back list does not use this same control path for selling; its visible item icon/button handlers differ. v0.8.5 additionally requires `SellItemTab` in the ancestor chain before treating a `ButtonItemClicked` as a bag sell candidate.

## Timing strategy

Speed is increased by polling state more frequently, not by issuing actions concurrently:

- NPC/shop/treatment state poll: 35 ms.
- Sell item sequential cadence: 45 ms.
- Short server quiet window between item passes: 70 ms.

This preserves one-action-at-a-time ordering while removing the old 140–260 ms fixed waits.
