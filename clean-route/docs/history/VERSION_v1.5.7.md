# VERSION v1.5.7 — Route Ownership Reacquire

## Problem Reported
First Confirm works; later death/revive misses automatic Confirm; manual STOP→START restores it.

## Investigation
Comparing the two paths showed that manual STOP calls the Bridge `StopPath` action, while post-revive `ResetRuntime()` only resets controller state. If the client preserves `autoPathing=1` across revive, route logic sees an already-running path and waits instead of emitting a fresh StartPath.

## Root Cause
The controller loses `crossMapRouteArmed` during cold reset but the client may keep the previous AutoPath alive. Because `crossMapRouteArmed` is armed only after a successful tool-issued cross-map StartPath, stale Path ON can create a deadlock: route logic waits, no new StartPath is issued, and semantic Confirm fails closed even if MessageBox appears.

## Solution
Add a session Route Ownership Reset gate after Start and post-revive: require authoritative AutoPath state; StopPath stale AutoPath if ON; verify OFF; clear route/Confirm ownership; then allow normal M87 guard/mount/route. Only the next fresh tool StartPath may own the cross-map route.

## Safety
Do not weaken MessageBox unrelated-dialog protection. Do not treat a pre-existing game AutoPath as tool-owned. Maximum StopPath attempts: 3, spaced 1200 ms, then fail closed.

## Build / CI Evidence
Windows x64 staging CI **PASS** — run `31940187052`, job `95148119252`.
- Controller SHA256: `b625e3530be6856022d563d1aaef2a6b359edf8ff15a8fe8710960493f4353e3`
- EXE SHA256: `35178832d412aec03832fd4ccc7f1ab1c966353058efc5d320440fb0a15174e2`
- Bridge DLL SHA256: `81cfafa953a6f7b8a9c95965f79d618b66dd5a28c02a8ac771c08e0a6729eade`
- Source ZIP SHA256: `ddf291cb409e3b4850e4689aef2cc6ae79ee8a005e507a4932ed30f205358f8b`
- Artifact ID: `9261845602`
- Artifact digest: `sha256:3626c4e0bad0355402d63c7c0df1bdc24701b05d31a5b9857fc32b7f2beb8814`
- Existing route/mount self-test: **15/15 PASS**.

## Runtime Status
**NEEDS USER TEST.** BUILD PASS does not prove repeated-death Confirm is fixed. Test at least three consecutive death/revive/return cycles and verify each session logs ownership reset before fresh StartPath/Confirm.
