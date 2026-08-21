# DATA REPO USAGE — mandatory startup memory

External knowledge repository:

`ngmthang-g/clinent-game-than-long-DATA-2222`

This repository is the **canonical client-analysis/data/VERIFIED knowledge store** for CleanRoute development. It is **not** the CleanRoute source repository and must not be treated as build input.

## Mandatory every version/task

Before modifying CleanRoute, creating a version, investigating a regression, or doing new client analysis, read from the DATA repo:

1. `AI_PROJECT_KNOWLEDGE_PROTOCOL_V2_OPTIMIZED.md`
2. `AI_CLIENT_ANALYSIS_RULES.txt`
3. `AI_INDEX.md`
4. `AI_BOOTSTRAP.md`
5. the one context pack routed by `AI_ROUTER.md` for the current task.

The two user-controlled rule files above remain mandatory until the user explicitly replaces them.

## Research rule

`AI_CLIENT_ANALYSIS_RULES.txt` means:

- read `AI_INDEX.md` and existing knowledge first;
- do not broad reverse-engineer the client again;
- only inspect binary/native details when the exact fact required by the task is not already present in VERIFIED/database knowledge.

## Separation of responsibilities

### DATA repo
Stores expensive client knowledge:
- VERIFIED APIs/state/action semantics;
- Lua/UI/config findings;
- IDs/ResIDs/packet facts;
- map/NPC/item/skill databases;
- reverse-engineering evidence and routing indexes.

### AUTO-train-thanlong source repo
Stores the actual tool implementation and project lineage:
- `PROJECT_KNOWLEDGE.md`;
- `CHANGELOG.md`;
- `clean-route/PROJECT_SUMMARY.md`;
- `clean-route/docs/` feature/version/bug/evidence docs;
- source/build/CI files.

Do not copy the full DATA repo into source. Record only the exact facts actually used by a version, with provenance and evidence status.

## Current cross-map Confirm research example

For v1.5.2 the DATA repo establishes that semantic map/UI state should be preferred over blind timing, while the supplied v0.9.0 source is a **SOURCE-INSPECTED DONOR** only. CleanRoute may learn the detector principle from it, but must not transplant its remote-worker/action architecture.
