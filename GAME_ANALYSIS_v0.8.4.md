# GAME_ANALYSIS v0.8.4

## Asset findings from the supplied client

`Config.unity3d` decrypted to UnityFS and extracted successfully. Exact records:

- `<NPC ID="373" Name="Mã Kiêu Minh" ... />` and `<NPCData ID="373" MapID="5" Name="Mã Kiêu Minh" />`.
- `<NPC ID="339" Name="Ðỗ Thanh Ðằng" ... />` and `<NPCData ID="339" MapID="5" Name="Ðỗ Thanh Ðằng" />`. Translation data also contains canonical Vietnamese `Đỗ Thanh Đằng`.

Therefore runtime RoleID 1000000378 is demonstrably not convertible to ResID by subtracting 1,000,000,000: Mã Kiêu Minh's actual ID is 373.

`Interface.unity3d` contains exact AUTO quick actions:

```lua
function TopIcon:AutoTrainClick()
  local AutoTrainService = GUI.FindUI("AutoFight_Main")
  if AutoTrainService ~= nil then
    AutoTrainService:StartAutoFight(C_AutoModel.Train)
    self:ShowAutoStatus(C_AutoModel.Train)
  end
end

function TopIcon:AutoStopClick()
  local AutoTrainService = GUI.FindUI("AutoFight_Main")
  if AutoTrainService ~= nil then
    AutoTrainService:StartAutoFight(C_AutoModel.None)
    self:ShowAutoStatus(C_AutoModel.None)
  end
end
```

`AutoFight_Main:StartAutoFight(Train)` sets `Game.EnableAutoF1=false`, calls `Game.AutoRemoveFlag()` and starts `AutoTrainStart()`. `StopAllCurrentTask()` restores `Game.EnableAutoF1=true`. This proves previous ON/OFF verification was reversed.

Chat asset also provides `BottomIcon:ButtonOpenChatBoxClicked()`, which calls `GUI.CallUI("ChatBox")` when absent.

`ChatBox` asset exposes exact `ButtonSendMessageClicked()` and `ButtonCloseClicked()` callbacks; v0.8.4 resolves the script fresh for each step.
