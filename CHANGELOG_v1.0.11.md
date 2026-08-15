# v1.0.11 - Unified map-transition classification

- Protocol bumped to `0x0001000B`.
- Explicit `MapReady/WaitingChangeMap` transitions are classified as `source=flags`.
- RoleID/MapID changes while flags stay clear are classified as `source=identity`, not merely requalification.
- Both sources mark MapTransition runtime coverage and enter the same identity-stable 2/2 recovery gate.
- A late flag signal joins an identity-started transition instead of creating a second transition session.
- Scanner qualification remains 60 consecutive stable snapshots on one stable identity.
- Gameplay action remains LOCKED.
