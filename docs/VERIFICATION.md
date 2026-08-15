# Verification status - v1.0.9

## Static contract

- Protocol: `0x00010009`.
- Scanner period: 500 ms.
- Observer stable mask: core + position + moving + autoPath.
- Qualification: 60 consecutive stable scans (~30 s).
- Reset qualification on scanner failure, partial mask or map transition.
- Transition recovery remains 2/2.
- Runtime edge coverage: moving/riding/autoPath/dead/mapTransition.
- No gameplay action enabled.

## Pass definitions

`SCANNER CORE QUALIFIED` means only that the read-only scanner stayed stable for 60 consecutive observer-valid snapshots. It does **not** mean all manual state edges have been exercised.

`RUNTIME EDGE COVERAGE pending=none` means all required manual observation edges were seen at least once in this process session.

Only after both are proven in the live client may the next phase scaffold SafetyGuard/ActionQueue/FSM; gameplay mutation stays separately gated.
