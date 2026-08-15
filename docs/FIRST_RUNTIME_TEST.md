# First runtime test - v1.0.7

Expected sequence:

1. ValidateNative PASS
2. InspectFgMainThread PASS
3. InspectUnityDispatcher PASS
4. MAINTHREAD PROVEN
5. ReadGameSnapshot
6. SNAPSHOT PASS

Expected UI core fields: RoleID, Map, HP, Bag, Dead, AutoFight.

`X,Y` is optional in this probe. If it is `?`, send the full log; the next step is to inspect the managed role backing type, not to restore hardcoded offsets.

If any core field fails, the session enters VALIDATOR FAIL and closes the bridge. This is intentional fail-closed behavior.
