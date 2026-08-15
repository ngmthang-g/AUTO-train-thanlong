# v1.0.10 - AutoPath diagnostics + identity-stable recovery

- Protocol bumped to `0x0001000A`.
- Added read-only `autoPathProbeMask` (5-step resolver/value proof).
- AutoPath coverage can be satisfied by observing value ON, not only a 0->1 edge.
- Map transition recovery now requires two consecutive stable snapshots with the same RoleID + MapID.
- Identity changes during recovery reset the 2/2 candidate.
- RoleID/MapID changes outside transition flags reset scanner 60/60 qualification.
- Gameplay action remains LOCKED.
