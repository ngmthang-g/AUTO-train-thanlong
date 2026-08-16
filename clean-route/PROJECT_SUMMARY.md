# Current candidate: v1.5.9 — Auto Rotate Train Spots
Adds per-account shared-spot rotation driven by repeated deaths or insufficient bag-fill productivity, while preserving v1.5.8 fixed periodic Confirm and all existing route/revive/sell safety gates.

Rotation defaults: **more than 10 deaths in rolling 10 minutes** (11th death at default limit), or **15 minutes of productive train time without a new FULL-bag edge**. One checked spot remains fixed; multiple checked spots rotate round-robin in shared-list order.

Build evidence: Windows x64 staging CI **PASS** — run `31949891312`, job `95171640279`, artifact `9264382136`; route test **15/15 PASS**, rotation test **8/8 PASS**; EXE `d6bcc8fd9e02ae818499d776498877ee51099854633556febd94ee945ef8d2bc`, DLL `5d7adcb842378cdd91225ee476efb5e5e8dbe4bf67929af7afc97af6eb951a24`. Runtime rotation remains NEEDS USER TEST.

# PROJECT SUMMARY — Clean Route v1.5.8

Current candidate: **Fixed Interval Confirm Click**. Automatic map Confirm has been deliberately simplified by user decision: every running account periodically real-clicks the saved Confirm coordinate (default 5s, configurable 1–300s). No popup/game-state evidence is used to decide whether to click.

Priority contract: transition/freeze/unresponsive/death/revive and any other click-owning flow wins. The Confirm timer stays overdue while blocked and runs at the first safe tick. Normal AutoPath is allowed so a portal popup can be clicked while pathing. Global REAL INPUT guard: 1000ms.

Bridge ReadState no longer probes MessageBox/UI. Protocol compatibility fields may remain but are not used for automatic Confirm.

Preserved: M87 AutoFight-OFF guard, Transition Freeze, Bridge request serialization, death-session cold restart, route ownership cleanup, AutoFight watchdog, 3-minute train-coordinate guard, Auto Sell, shared spots and persistent six-click calibration.

Build evidence: Windows x64 staging CI PASS `31942843638` / job `95154396882`; self-test 15/15. Compile controller SHA256 `01cafd0a490a140da10db5a92ac23f7b70d794f88cb4936f002f07962dfb0ca0`; Bridge source SHA256 `64967db0d72cd584c909ce12dd4fdf20cace1128784b9e43214bd419014ed05f`. Runtime remains NEEDS USER TEST.
