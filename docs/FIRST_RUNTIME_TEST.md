# Runtime test - v1.0.12 SafetyGuard + ActionQueue + FSM Dry-Run

## 1. Stable scanner -> FSM

Đứng trong một map ổn định đến `SCANNER CORE QUALIFIED 60/60`.

Expected:
- scanner 1/60 -> 10/60 -> 30/60 -> 60/60;
- `FSM • SAFE_PAUSED/SCANNER_QUALIFYING -> IDLE_STABLE`;
- không có action candidate khi alive/stable.

## 2. Map transition fail-closed

Đổi map một lần bằng source=identity hoặc source=flags.

Expected:
- `FSM • ... -> MAP_TRANSITION`;
- queue được clear;
- map recovery vẫn 1/2 -> PASS 2/2 cùng RoleID + MapID;
- sau recovery FSM về `SCANNER_QUALIFYING` cho đến khi scanner đạt lại 60/60.

## 3. Dead candidate khi scanner chưa qualified

Sau đổi map, có thể để qualification chưa đủ rồi cho nhân vật chết thủ công.

Expected:
- `STATE EDGE • dead=0->1`;
- `FSM • ... -> DEAD_DETECTED`;
- `SAFETY GUARD BLOCK • intent=Revive • reason=scanner-unqualified`;
- không có queue/action game.

## 4. Dead candidate khi scanner đã qualified

Đứng yên đủ 60/60 trong trạng thái dead, hoặc chết sau khi scanner đã 60/60.

Expected đúng một dry-run envelope cho một dead episode:
- `SAFETY GUARD PASS(dry-run) • intent=Revive ... mutation permission=LOCKED`;
- `ACTION QUEUE • depth=1/1 active=0 • accepted DRY-RUN intent=Revive`;
- `DISPATCHER GATE • intent=Revive • BLOCKED ... không phát BridgeCommand/action game`;
- `ACTION QUEUE • depth=0/1 active=0`.

Không được lặp candidate mỗi 500 ms khi nhân vật vẫn dead.

## 5. Revive thủ công

Hồi sinh bằng tay.

Expected:
- `dead=1->0`;
- FSM rời `DEAD_DETECTED`;
- nếu scanner vẫn qualified -> `IDLE_STABLE`, nếu chưa -> `SCANNER_QUALIFYING`.

## Gate

PASS khi:
- không crash/disconnect;
- FSM chỉ có một state hiện tại/client;
- SafetyGuard block/pass đúng PRE conditions;
- ActionQueue không bao giờ vượt `1/1`, active luôn 0;
- dispatcher gate không phát gameplay command;
- mutation vẫn khóa compile-time.
