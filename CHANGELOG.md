# v1.5.10 — Internal Confirm + Revive v0.8.7
- Replaced v1.5.9 periodic/saved-coordinate map Confirm with the runtime-proven v0.8.7 internal UIButton callback path.
- Replaced coordinate Revive with internal `UIButton.HandleClickEvent()` gated by a fresh authoritative `get_IsDeath`.
- Removed Confirm checkbox, Confirm interval, Confirm capture/test coordinate and Revive capture/test coordinate.
- Visible per-account AutoFight calibration is now exactly 4 points: AUTO / ĐÁNH QUÁI / DỪNG AUTO 1 / DỪNG AUTO 2.
- Removed unused generic Bridge `ClickAt` command.
- Restored `ValidConfirmUi + confirmUiVisible` from the donor-exact `MainFindUI("MessageBox")` path; no widened `FindUI` fallback.
- Donor UI fixed RVAs are accepted only after exact PE timestamp/SizeOfImage + byte-signature checks.
- MessageBox managed string is rooted/cached with IL2CPP GCHandle to match donor behavior and avoid per-poll allocation pressure.
- Added disposed-UIObject guard and unambiguous-button selection.
- Confirm preserves the donor accent-aware matcher (`Đồng ý` is not folded into the negative `Đóng` token).
- Revive now follows the donor lock/retry cadence: failed discovery retries after 2s; after a successful callback, only one late retry is allowed after 15s if `IsDeath` is still true.
- Saving a profile deletes legacy Confirm/Revive coordinate/timer INI keys.
- Re-enabled tool-owned semantic `HandleCrossMapConfirm`; successful callback immediately hands off to Transition Freeze.
- Removed the old `current map == train map` ownership reset because it incorrectly breaks cross-map sell/recovery routes originating from the train map.
- Local pure tests: route 15/15 PASS; rotation 8/8 PASS; integration audit 33/33 PASS; static/balance audit PASS. Windows PE build and runtime remain unverified in this workspace.

# v1.5.9 — Auto Rotate Train Spots
- Per-account checkbox rotation pool over existing shared train spots.
- Defaults: rotate after **more than 10 deaths in rolling 10 minutes** (11th death triggers with limit=10).
- Defaults: rotate after **15 minutes of productive training without a new FULL-bag event**.
- Productive timer counts only alive + at target + AutoFight ON + authoritative FreeBagSpace.
- FULL-bag edge resets the no-FULL timer.
- One selected spot stays fixed; multiple spots cycle and wrap to the first.
- Live rotation reuses train recovery to stop AutoFight before pathing; death-triggered rotation changes target before revive return.
- Settings persisted per RoleID: RotationSpot list + death/window/no-FULL thresholds.
- v1.5.8 fixed periodic Confirm remains unchanged.
- Windows x64 staging CI **PASS** — run `31949891312`, job `95171640279`, artifact `9264382136`.
- Route/mount test **15/15 PASS**; rotation test **8/8 PASS**; Bridge PE/EXE/package/artifact verification PASS.
- v1.5.9 EXE SHA256 `d6bcc8fd9e02ae818499d776498877ee51099854633556febd94ee945ef8d2bc`; DLL `5d7adcb842378cdd91225ee476efb5e5e8dbe4bf67929af7afc97af6eb951a24`; source ZIP `7c1297f717ebb880932d83cccb90e0fc9236b49762ab8b14b1cc288ee37b3b1e`; artifact digest `efc36d0fb66593370b49e92e63bfabe67ce09edbbd70a34488ce6c6920ce807d`.
- Runtime auto-rotation: **NEEDS USER TEST**.

# v1.5.8 — 2026-08-16

Requested:
- abandon semantic/game-state-driven map Confirm;
- fixed periodic real click at saved Confirm coordinate, default 5s and user-configurable;
- pause/yield while map transition, unresponsive/frozen client, or another auto-click/action flow owns the mouse.

Changed:
- added per-account `ConfirmIntervalSec` (1–300s, default 5) with persistent UI control;
- added `HandlePeriodicConfirmClick` + `PeriodicConfirmBusy`;
- timer stays overdue while busy and fires on first idle tick;
- global REAL INPUT mutex guard prevents another account's recent click from being immediately preempted;
- periodic Confirm is allowed during ordinary AutoPath/travel and sell/recovery travel phases;
- removed special revive Confirm injection;
- semantic `HandleCrossMapConfirm` is no longer invoked;
- Bridge `ReadState` no longer performs `MainFindUI/FindUI("MessageBox")`; `ValidConfirmUi` remains protocol-compatible but intentionally unset.

Validation:
- local route/mount self-test 15/15 PASS;
- Windows x64 staging CI **PASS** — run `31942843638`, job `95154396882`; audit + route/mount self-test 15/15 + Bridge/EXE/package verification passed.
- compile controller SHA256 `01cafd0a490a140da10db5a92ac23f7b70d794f88cb4936f002f07962dfb0ca0`; Bridge source SHA256 `64967db0d72cd584c909ce12dd4fdf20cace1128784b9e43214bd419014ed05f`.
- staging EXE SHA256 `e6862b4aad90c28e272408cde08f1aadea641a7783f63acec381702d2e688f4d`; Bridge DLL `5d7adcb842378cdd91225ee476efb5e5e8dbe4bf67929af7afc97af6eb951a24`.
- runtime: **NEEDS USER TEST**; BUILD PASS is not runtime PASS.

# v1.5.6 — Death Session Cold Restart
- Every new death is now a full RuntimeState boundary, not a partial flag reset.
- Added Account-level `deathSessionLatched` outside RuntimeState to prevent repeated hard-reset loops.
- Full `ResetRuntime()` on first authoritative DEAD edge.
- Full `ResetRuntime()` again on stable ALIVE edge; next tick behaves like a fresh Start.
- Preserves AccountProfile/settings and Bridge attachment.
- Keeps Map 87 AutoFight guard, semantic Confirm and Transition Freeze unchanged.
- Runtime repeated-death validation still required.
- Windows CI PASS: run `31938154271`, job `95143227671`, artifact `9261270126`; controller `840bc0376961d2838e1ca46598c43b07d91ccec846f375efece8f2ae569b6711`; EXE `5651cc8f440628a298391c01a627480eccca1e27b1458f1db739f373573b204b`.

# v1.5.5 — 2026-08-16

Requested:
- after the second death/revive cycle, cross-map Confirm must behave like a fresh run instead of inheriting prior route/Confirm state;
- identify Địa Phủ exactly from client DATA;
- while the character is in Địa Phủ, if authoritative AutoFight is ON, use the two existing saved `DỪNG AUTO 1/2` clicks, verify AutoFight OFF, then continue the normal return-to-train route.

Runtime evidence before change:
- v1.5.4 first tested death/return path: `MessageBox=1 + AutoPath=1 → StopPath → REAL CLICK Confirm → FREEZE ACTION` — PASS for the tested gate;
- v1.5.4 Transition Freeze also showed `ReadState` failures being held and later `CLIENT ỔN ĐỊNH LIÊN TỤC 2s → mở khóa action`;
- a later/second death cycle returned toward the train map but automatic Confirm was no longer observed. Exact causal chain is not yet proven.

Verified client data:
- `database/MAPS.csv`: `87,Địa Phủ,siwang,0,Wild,1,siwang`; therefore **Địa Phủ MapID = 87**.

Changed:
- every NEW death resets cross-map ownership, route-moved evidence, stall timer, Confirm attempts/debounce/StopPath timing, suppression timing and Địa Phủ recovery state;
- add high-priority M87 Địa Phủ AutoFight guard before normal route/Confirm/sell/recovery arbitration;
- if `ValidAutoFight` is not authoritative on M87, fail closed and wait;
- if AutoFight is ON: `DỪNG AUTO 1 → 700ms → DỪNG AUTO 2 → fresh getter verification`;
- if still ON after 1200ms, retry the full two-click sequence, capped at 3 attempts;
- only after fresh state proves AutoFight OFF may normal route logic continue;
- v1.5.4 Transition Freeze and v1.5.3 Path-ON semantic Confirm are preserved.

Build:
- Windows x64 code-validation CI **PASS** — run `31934654526`, job `95134583587`, artifact `9260311819`; controller SHA256 `73451f4fbc244c9b05ef3f86ff3daeb445b3e4bffd0be6e534bc514cb0759d8e`, EXE SHA256 `bc8fbe8e6b40dda4e84590de065cfc9f898d767aed85f1a1ed376a7c7bbc5456`, Bridge SHA256 `81cfafa953a6f7b8a9c95965f79d618b66dd5a28c02a8ac771c08e0a6729eade`. Runtime remains NEEDS USER TEST.

Runtime:
- **NEEDS USER TEST**, especially two consecutive death/revive/return cycles.

# v1.5.4 — 2026-08-16

Requested:
- when the game is changing map or temporarily unresponsive, send no gameplay/window actions;
- resume automation only after map/client is stable for 2 continuous seconds.

Runtime evidence:
- v1.5.3 fast transition: semantic MessageBox detected while Path ON, StopPath sent, saved Confirm clicked, destination reached — **RUNTIME PARTIAL PASS**;
- v1.5.3 slow transition: repeated `ReadState fail: Bridge timeout; fail-closed` for multiple seconds; state is non-authoritative during loading.

Changed:
- per-PID `clientFreezeActive` safety gate on explicit map transition or ReadState/Bridge timeout/busy;
- freeze immediately after first successful saved Confirm click, before `WaitingChangeMap` necessarily appears;
- no route/mount/AutoFight/AutoSell/ClickNPC/saved-coordinate action while frozen;
- BridgeClient preserves timed-out `pendingSeq` and refuses to overwrite it with a new request;
- same pending request may only receive a bounded same-wake nudge while bridge is not busy;
- resume only after fresh ReadState + MapReady + no WaitingChangeMap + responsive game window remain healthy continuously for 2000 ms;
- any failed read/transition/unresponsive-window probe resets the 2-second stability timer;
- v1.5.3 Path-ON semantic Confirm ordering remains intact.

Build:
- Windows x64 code-validation CI **PASS** — run `31931942494`, job `95128013208`, artifact `9259572967`, digest `sha256:f3c07bd0ca7270fcec806c9e01a28cd9b67913334a183cd1fef453e8d7a5be2a`;
- controller SHA256 `582c723501234caef039ea00d9ace0be0ef6a86b37874927b1ac5a6da5c98d3b`;
- EXE SHA256 `76cf90583c552549052a1c3b615c436b75f8840b313756d19c33ac6d30608e49`;
- Bridge SHA256 `81cfafa953a6f7b8a9c95965f79d618b66dd5a28c02a8ac771c08e0a6729eade`.

Runtime:
- v1.5.4 Transition Freeze / 2-second stable resume: **NEEDS USER TEST**.

# v1.5.3 — 2026-08-16

Requested:
- investigate supplied v0.8.7 because it confirms map fastest/reliably;
- fix automatic Confirm while popup is visibly present and AutoPath remains ON.

Root cause:
- **CONFIRMED:** v1.5.2 returned immediately on `s.autoPathing` before consuming semantic `confirmUiVisible`.

Changed:
- Path ON becomes route evidence instead of early-return blocker;
- MessageBox ON + moving OFF → 200 ms settle;
- if Path ON, one serialized StopPath;
- fresh snapshot + ≥250 ms → saved Confirm click if MessageBox still ON;
- do not deadlock if logical Path flag stays ON after StopPath.

Build:
- Windows x64 CI PASS — run `31927283960`, job `95116711777`.

Runtime:
- **PARTIAL PASS** — fast tested gate crossed; slow map exposed Bridge timeout storm.

# v1.5.2 — 2026-08-16

Added:
- `ValidConfirmUi + confirmUiVisible`;
- read-only `MainFindUI("MessageBox")` observer, `FindUI` fallback.

Runtime:
- **FAIL** for `MessageBox visible + AutoPath ON` because controller ordering hid the popup branch.

# v1.5.1 — 2026-08-16

Added:
- AutoFight check at arrival + every 60s while idle;
- LocalAppData persistent config and PID→Role merge.

# v1.5.0 — 2026-08-16

Added:
- 3-minute coordinate guard;
- mount 5s → mount 5s → foot 15s repeat;
- scheduled tool close.

# v1.4.2 — 2026-08-16

Changed:
- NPC X/Y user-captured; no hard-coded sell coordinates.

## v1.5.10 runtime hotfix — passive scan / lazy donor UI
- Fix first live-test regression: opening/scanning could diss the game because ScanClients attached bridge and called ReadState immediately.
- Scan now performs no bridge attach and no ReadState.
- Core ReadState no longer polls donor MessageBox UI. Dedicated ProbeInternalConfirm is lazy and cross-map-context-only.
