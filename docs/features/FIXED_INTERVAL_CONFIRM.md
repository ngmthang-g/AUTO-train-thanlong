# Feature — Fixed Interval Map Confirm

**Status: SUPERSEDED by v1.5.10 / DEC-014.**

v1.5.8/v1.5.9 used a saved Confirm coordinate and periodic real click. The user explicitly removed this design in v1.5.10 because v0.8.7 internal UIButton Confirm/Revive had better runtime behavior.

Do not restore:
- `Auto XN map` checkbox;
- Confirm coordinate capture/test;
- `ConfirmIntervalSec`;
- `HandlePeriodicConfirmClick` / `PeriodicConfirmBusy`.

See `CROSS_MAP_CONFIRM.md` and `DECISION_014_V087_INTERNAL_CONFIRM_REVIVE.md` for the active design.
