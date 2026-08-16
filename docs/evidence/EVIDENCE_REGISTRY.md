# EVIDENCE REGISTRY

## EVID-001 — Manual Confirm works; automatic orchestration historically failed
Type: USER_RUNTIME
Observation: saved Confirm coordinate works manually; automatic timing/state recognition was the defect.
Confidence: HIGH.

## EVID-002 — v0.8.7/v0.9.0 donor MessageBox principle
Type: SOURCE / DONOR RESEARCH
Observation: donor uses `MainFindUI("MessageBox")`, does not require Path OFF before portal-ready, and StopPaths immediately before Confirm.
Supports: semantic popup proof + correct ordering.
Boundary updated by v1.5.10 user decision: supports transplanting the exact proven UI RVAs/action slice only behind exact PE/signature gates; still does NOT support restoring the donor remote worker or wholesale action architecture.

## EVID-003 — Current client semantic UI API
Type: VERIFIED CLIENT KB
Observation: `LuaSystemAPI_GUI.MainFindUI/FindUI` exposes semantic MessageBox presence.

## EVID-004 — v1.5.3 fast runtime Confirm
Type: USER_RUNTIME / LOG
Observation: `MessageBox=1 + AutoPath=1 → StopPath → REAL CLICK Confirm → Path=OFF → destination reached`.
Supports: v1.5.3 Path-ON Confirm ordering can work end-to-end on a fast transition.
Confidence: HIGH.

## EVID-005 — v1.5.3 slow transition timeout storm
Type: USER_RUNTIME / LOG
Observation: slow loading produced repeated `ReadState fail: Bridge timeout; fail-closed` for multiple seconds after Confirm.
Supports: state is non-authoritative during slow loading and needs global action freeze/request serialization.
Does NOT prove: exact internal game-thread reason for the delay.
Confidence: HIGH.

## EVID-006 — v1.5.4 Windows code-validation CI
Type: BUILD_CI
Source: run `31931942494`, job `95128013208`.
Observation: reconstructed controller SHA256 `582c723501234caef039ea00d9ace0be0ef6a86b37874927b1ac5a6da5c98d3b`; scope audit PASS; route/mount self-test 15/15 PASS; Bridge/controller build PASS; PE/load/export and package verification PASS.
Artifact: `9259572967`, digest `sha256:f3c07bd0ca7270fcec806c9e01a28cd9b67913334a183cd1fef453e8d7a5be2a`.
Supports: v1.5.4 source/build contracts.
Does NOT prove: real slow-map Transition Freeze behavior.
Confidence: BUILD CONFIRMED.

## EVID-007 — v1.5.4 real Transition Freeze recovery
Type: USER_RUNTIME / LOG / SCREENSHOT
Observation: map transition produced FREEZE ACTION, transient ReadState/Bridge failures were held without normal auto actions, then `CLIENT ỔN ĐỊNH LIÊN TỤC 2s → mở khóa action, tiếp tục auto`.
Supports: v1.5.4 freeze/stable-resume behavior works in the tested runtime.
Confidence: HIGH for tested case.

## EVID-008 — Second death cycle missed automatic Confirm
Type: USER_RUNTIME
Observation: first death/return Confirm worked; after dying again, post-revive return routing occurred but automatic map confirmation was no longer observed.
Supports: repeated death-cycle orchestration is not yet runtime reliable.
Does NOT prove: AutoFight is the sole/root cause.
Confidence: HIGH observation / UNKNOWN root cause.

## EVID-009 — Địa Phủ map identity
Type: VERIFIED STATIC CLIENT DATA
Source: canonical DATA repo `database/MAPS.csv`.
Observation: `87,Địa Phủ,siwang,0,Wild,1,siwang`.
Supports: exact M87 guard without guessed map identity.
Confidence: VERIFIED.

## EVID-010 — v1.5.5 Windows code-validation CI
Type: BUILD_CI
Date / Version: 2026-08-16, v1.5.5
Source: GitHub Actions run `31934654526`, job `95134583587`, artifact `9260311819`.
Observation: reconstruct passed exact controller SHA256 `73451f4fbc244c9b05ef3f86ff3daeb445b3e4bffd0be6e534bc514cb0759d8e`; v1.5.5 audit PASS; route/mount self-test 15/15 PASS; Bridge/EXE compiled; PE/load/export and release packaging verification PASS. EXE SHA256 `bc8fbe8e6b40dda4e84590de065cfc9f898d767aed85f1a1ed376a7c7bbc5456`; Bridge SHA256 `81cfafa953a6f7b8a9c95965f79d618b66dd5a28c02a8ac771c08e0a6729eade`.
Supports: v1.5.5 source/build contract is reproducible on Windows CI.
Does NOT Prove: M87 AutoFight stop or second-death Confirm works in the real client.
Confidence: BUILD CONFIRMED.

## EVID-011 — v1.5.6 Death Session Cold Restart Windows CI
Type: BUILD_CI
Date / Version: 2026-08-16, v1.5.6
Source: GitHub Actions run `31938154271`, job `95143227671`, artifact `9261270126`.
Observation: exact v1.5.5 baseline SHA passed, fuzz=0 v1.5.6 patch applied, final controller SHA256 `840bc0376961d2838e1ca46598c43b07d91ccec846f375efece8f2ae569b6711`; dual-boundary reset audit PASS; route/mount 15/15 PASS; EXE/DLL/package verification PASS.
Artifact digest: `sha256:867e5da11ded01f96137812f1852296484d0fa70516a3d408bb6aa1066f682be`.
Does NOT prove: repeated-death behavior in the real game client.
Confidence: BUILD CONFIRMED.

## EVID-014 — v0.8.7 exact internal UI donor extraction for v1.5.10
Type: SOURCE / DONOR COMPARISON
Observation: supplied v0.8.7 uses `MainFindUI("MessageBox")`, `UIObject.instances`, active/interactable UIButton inspection, donor-style candidate scoring and `UIButton.HandleClickEvent()`; Revive re-reads IsDeath immediately before locating Đầu thai. Donor PE identity is timestamp `0x6A410C14` and SizeOfImage `0x03DCB000`; required UI RVAs have fixed 12-byte signatures.
Supports: v1.5.10 exact UI-slice transplant and fail-closed build matching.
Confidence: SOURCE CONFIRMED.

## EVID-015 — v1.5.10 local source validation
Type: LOCAL_BUILD_LOGIC / STATIC
Observation: route logic test 15/15 PASS; rotation logic test 8/8 PASS; v1.5.10 static residue/contract audit PASS; lexical delimiter balance PASS.
Does NOT prove: Windows PE compilation or real-client runtime behavior.
Confidence: LOCAL VERIFIED.
