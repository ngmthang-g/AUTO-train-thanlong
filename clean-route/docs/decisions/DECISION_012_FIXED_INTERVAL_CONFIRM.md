# DECISION-012 — Automatic map Confirm is periodic, not semantic

Supersede semantic MessageBox-driven automatic Confirm for current CleanRoute. The user prefers deterministic periodic real input at the saved Confirm coordinate. Keep transition/freeze and click-ownership guards, but do not require UI/game evidence that a Confirm popup exists. Bridge ReadState must not probe MessageBox for this feature.
