# v1.0.9 - Scanner Qualification

- Protocol bumped to `0x00010009`.
- Replaced immediate scanner PASS with a 60-consecutive-stable-snapshot qualification gate (~30 s at 500 ms).
- Scanner failure and map transition reset qualification.
- Added read-only runtime edge coverage tracking for Moving, Riding, AutoPath, Dead and MapTransition.
- Added qualification milestone logs at 1/60, 10/60 and 30/60; PASS at 60/60.
- Preserved donor map-transition guard and 2/2 stable recovery.
- Preserved fail-closed 3-consecutive-failure rule.
- No gameplay mutation/action added.
