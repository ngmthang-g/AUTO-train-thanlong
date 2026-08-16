# DECISION-012 — Automatic map Confirm is periodic, not semantic

**Status: SUPERSEDED in v1.5.10 by DEC-014.**

Historical decision for v1.5.8/v1.5.9: automatic map Confirm used a periodic real click at a saved coordinate.

The user explicitly reversed this decision after confirming v0.8.7 internal Confirm + Đầu thai was substantially smoother. v1.5.10 removes the coordinate/timer implementation entirely.
