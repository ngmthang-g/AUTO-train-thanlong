# DECISIONS

## DEC-014 — v0.8.7 internal UIButton Confirm + Revive
Date / Version: 2026-08-16 / v1.5.10
Status: ACTIVE

The user explicitly superseded periodic/saved-coordinate Confirm and coordinate Revive. v1.5.10 transplants only the v0.8.7 UI callback slice (`MainFindUI("MessageBox")`, `UIObject.instances`, UIButton inspection, `UIButton.HandleClickEvent`) into the current serialized Bridge, guarded by exact PE/signatures and controller ownership. Four AutoFight calibration clicks remain. See `DECISION_014_V087_INTERNAL_CONFIRM_REVIVE.md`.

## DEC-012 — Fixed interval Confirm
Status: SUPERSEDED by DEC-014.

## DEC-008 — Semantic real-UI proof gates automatic cross-map Confirm

Date / Version: 2026-08-16 / v1.5.2
Status: SUPERSEDED AS A WHOLE by DEC-014; semantic-proof principle retained

Decision:
Cross-map Confirm must not be triggered from portal stall/timing alone. A read-only semantic UI observer must be authoritative and report the real `MessageBox` present before any Confirm action is allowed. v1.5.10 uses internal UIButton callback instead of a saved-coordinate click.

Context:
Manual TEST click works, but actual automatic flow misses the correct moment. Donor v0.9.0 had an authoritative/existence Confirm UI scanner.

Alternatives:
- increase fixed delays;
- increase blind retries;
- copy v0.9.0 `UIObject.instances` scanner and old orchestration wholesale;
- invoke MessageBox callback directly (this alternative became the user-selected action in v1.5.10/DEC-014 after v0.8.7 runtime evidence).

Why Rejected:
- timing/retry approaches already failed runtime;
- wholesale donor copy risks reintroducing legacy unsafe architecture;
- at v1.5.2, callback/action change was outside that version's request; this reason is historically superseded by the explicit v1.5.10 request.

Evidence:
EVID-001, EVID-002, EVID-003.

Reason:
Preserve the proven detector invariant while using the cleaner current semantic API and current action engine.

Consequences:
- detector failure becomes fail-closed;
- runtime logs can distinguish `MessageBox ?`, OFF and ON;
- timeout remains only a debounce/failure guard.

Affected Features:
CROSS_MAP_CONFIRM, AUTO_TRAIN, AUTO_SELL return routes.

## DEC-011 — Death/revive is a full RuntimeState session boundary
Date / Version: 2026-08-16 / v1.5.6
Status: ACTIVE

Decision:
Do not maintain a growing manual list of transient fields to reset after death. On the first authoritative DEAD edge, full `ResetRuntime()` is mandatory. On the subsequent authoritative ALIVE edge, full `ResetRuntime()` is mandatory again. A death-session latch must live outside RuntimeState so the reset remains one-shot.

Reason:
The repeated-death failure proved that partial per-field reset is structurally fragile because unrelated phases such as map qualification, fight/recovery/travel/sell/watchdogs can survive a prior life. Full boundary reset gives every post-revive route the same transient state as pressing Start while preserving AccountProfile/settings and Bridge attachment.

Evidence: BUG-003, EVID-008, EVID-011.
