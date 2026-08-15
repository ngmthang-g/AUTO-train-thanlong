# Runtime Acceptance Policy - v1.1.0

v1.1.0 is intentionally not another small manual-test checkpoint.

## Automated before runtime

GitHub CI must PASS:
1. architecture/mutation/queue audit;
2. deterministic control self-test;
3. bridge build + LoadLibrary test;
4. controller build;
5. artifact verification.

The same control self-test runs again when the controller starts. A failure blocks foundation startup.

## What is still runtime-only

Only facts that cannot be simulated replace real game evidence:
- real IL2CPP object resolution;
- real Unity main-thread context;
- real action method resolution;
- actual game state change after a mutation;
- PRE -> ACTION -> POST -> VERIFY against a live client.

## One final acceptance

Do not ask the user to test v1.1.0 phase-by-phase. Continue development and capability resolution first. When all action methods selected for the first usable auto are evidence-backed, run one acceptance session covering stable scanner, map change, death/revive, AutoPath/combat state and action POST verification.

Until then gameplay mutation remains compile-time locked.
