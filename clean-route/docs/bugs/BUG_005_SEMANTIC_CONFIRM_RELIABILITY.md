# BUG-005 — semantic Confirm reliability across repeated cycles

Observed: semantic Confirm could work on first transition but fail on later death/revive cycles; manual Stop/Start sometimes restored it. Multiple state-reset/ownership fixes did not produce reliable repeated runtime behavior.

Resolution policy: v1.5.8 supersedes semantic automatic Confirm with fixed-interval saved-coordinate clicking per explicit user request. This is a product/behavior decision, not proof that every prior inferred root cause was wrong.
