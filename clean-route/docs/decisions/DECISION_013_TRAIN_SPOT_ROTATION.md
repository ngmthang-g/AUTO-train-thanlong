# DECISION 013 — Rotate full train spots, not raw MapIDs

Use the existing shared train-spot records (`Name + MapID + X + Y`) as rotation members.

Reasons:
1. the tool already routes to a complete target, not merely a map;
2. one map may legitimately contain multiple train positions;
3. AutoSell return routing already depends on the current train target;
4. selecting complete spots lets all existing mount/path/Confirm/recovery logic remain unchanged.

Rotation order follows the shared spot list order. Each account stores only the names of checked shared spots.
