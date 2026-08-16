# VERSION v1.5.6 — Death Session Cold Restart

## User problem
Repeated death cycles did not behave like the first cycle; automatic cross-map Confirm could disappear on later deaths.

## Architectural change
Stop maintaining a manual list of per-death resets. A death is now a lifecycle/session boundary.

### DEAD boundary
On the first authoritative `dead=1` snapshot of a session:
- call full `ResetRuntime()`;
- keep `running=true`;
- set `deathSessionLatched=true` outside RuntimeState;
- start revive timer from a clean runtime.

### ALIVE boundary
When authoritative `dead=0` returns while the session latch is set:
- call full `ResetRuntime()` again;
- clear the external latch;
- return for one tick;
- next tick resumes exactly like a fresh Start: map qualification → M87 guard → route → Confirm → train.

## Why the latch is outside RuntimeState
If the latch lived inside RuntimeState, `ResetRuntime()` would erase it and every `dead=1` tick would hard-reset again forever. Keeping it on Account makes full runtime reset safe and one-shot.

## Preserved state
`AccountProfile` is not reset: shared train spot selection, tolerance, six click points, AutoSell/NPC/macro and feature toggles remain. Bridge attachment remains.

## Preserved safety
- MapID 87 Địa Phủ AutoFight OFF guard.
- semantic MessageBox cross-map Confirm.
- Transition Freeze and serialized Bridge pending request behavior.
- no new direct combat/sell mutation API.

## Expected runtime trace
`NEW DEATH SESSION → revive → alive → POST-REVIVE COLD START → M87 AutoFight OFF → fresh route arm → MessageBox → StopPath → Confirm → Freeze`.

## Build/CI
**PASS** — Windows x64 code-validation run `31938154271`, job `95143227671`, artifact `9261270126`, digest `sha256:867e5da11ded01f96137812f1852296484d0fa70516a3d408bb6aa1066f682be`.

- Controller SHA256: `840bc0376961d2838e1ca46598c43b07d91ccec846f375efece8f2ae569b6711`
- EXE SHA256: `5651cc8f440628a298391c01a627480eccca1e27b1458f1db739f373573b204b`
- Bridge DLL SHA256: `81cfafa953a6f7b8a9c95965f79d618b66dd5a28c02a8ac771c08e0a6729eade`
- Source ZIP SHA256: `390ef2f69eb77d9a7ce9cb26b2b4d9fee7519abf3a98178feeb09de5921aa89e`
- audit PASS; route/mount self-test 15/15 PASS; PE/load/export/package verification PASS.

## Runtime
UNTESTED until repeated-death client test.
