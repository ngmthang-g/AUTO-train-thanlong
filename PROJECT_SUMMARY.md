# Current candidate: v1.5.10 — Internal Confirm + Revive v0.8.7

Base: **v1.5.9 Auto Rotate Train Spots**.

User-requested replacement:
- remove coordinate/timer map Confirm;
- remove coordinate Revive;
- transplant the v0.8.7 internal UIButton mechanism for those two actions;
- retain only the four visible AutoFight calibration clicks: AUTO, Attack, StopAuto1, StopAuto2.

Implementation boundary:
- controller owns timing/state/route arbitration;
- Bridge executes donor UI reads/callback only on the target game-window thread;
- donor fixed RVAs are accepted **only** for this UI slice and only after exact PE identity + byte-signature validation;
- no `CreateRemoteThread`, `WriteProcessMemory`, or donor `remote_worker` is restored;
- MessageBox string is GCHandle-cached exactly to avoid per-poll managed allocations;
- stale/disposed UIObject entries are rejected;
- ambiguous UI candidates fail closed; Confirm preserves the donor Unicode matcher so `Đồng ý` cannot collide with the negative `Đóng` label.

Cross-map order:
`owned StartPath -> MessageBox authoritative -> moving OFF -> StopPath if Path ON -> fresh delay -> internal Confirm -> Transition Freeze`.

Death order:
`authoritative DEAD -> session reset/latch -> StopPath if safely authoritative -> re-read IsDeath -> unique internal Đầu thai -> lock -> at most one late retry after 15s -> wait alive -> cold restart`.

Current validation in this workspace:
- route test 15/15 PASS;
- rotation test 8/8 PASS;
- v1.5.10 integration audit 33/33 PASS;
- source delimiter/balance audit PASS;
- Windows PE compile not run here because Zig/Windows cross toolchain is not installed.
