from pathlib import Path
import re


def read(path):
    return Path(path).read_text(encoding="utf-8")


def write(path, text):
    Path(path).write_text(text, encoding="utf-8")


def sub_once(text, pattern, replacement, label):
    out, n = re.subn(pattern, replacement, text, count=1, flags=re.S)
    if n != 1:
        raise SystemExit(f"{label}: expected exactly one replacement, got {n}")
    return out


controller_path = "src/controller.cpp"
controller = read(controller_path)

needle = "    DWORD lastConfirmClickTick = 0;\n    DWORD lastRealInputTick = 0;"
replacement = "    DWORD lastConfirmClickTick = 0;\n    DWORD lastConfirmProbeTick = 0;\n    DWORD lastRealInputTick = 0;"
if controller.count(needle) != 1:
    raise SystemExit("controller confirm tick anchor mismatch")
controller = controller.replace(needle, replacement, 1)

scan_new = r'''    void ScanClients() {
        captureSlot_ = ClickSlot::None;
        captureMacroIndex_ = -1;
        capturePid_ = 0;
        for (auto& a : accounts_) a->bridge.Close();
        accounts_.clear();
        ListView_DeleteAllItems(clientList_);

        // Runtime hotfix: scanning must be STRICTLY PASSIVE. Enumerating windows and
        // GameAssembly.dll is enough to show clients. Do not SetWindowsHookEx, load the
        // bridge into the game, or issue ReadState merely because the tool opened or the
        // user pressed Scan. The bridge is attached only after an explicit Start.
        const auto found = FindClients();
        for (const auto& game : found) {
            auto a = std::make_unique<Account>();
            a->game = game;
            a->snapshot = {};
            a->snapshotValid = false;
            a->displayName = L"? • PID " + std::to_wstring(game.pid);
            a->profile = LoadProfile(ProfileSection(a->snapshot, game.pid));
            MigrateLegacySpot(a->profile);
            a->runtime.status = L"Đã dừng • scan thụ động";
            accounts_.push_back(std::move(a));
        }

        for (std::size_t i = 0; i < accounts_.size(); ++i) InsertAccountRow(static_cast<int>(i), *accounts_[i]);
        RefreshSpotCombo();
        if (!accounts_.empty()) {
            ListView_SetItemState(clientList_, 0, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
            ListView_EnsureVisible(clientList_, 0, FALSE);
            LoadSelectedProfileToUi();
        } else {
            ClearEditor();
        }
        Log(L"Quét thụ động thấy " + std::to_wstring(accounts_.size()) +
            L" client GameAssembly.dll • KHÔNG attach bridge / KHÔNG ReadState.");
    }

    void InsertAccountRow'''
controller = sub_once(
    controller,
    r"    void ScanClients\(\) \{.*?\n    \}\n\n    void InsertAccountRow",
    scan_new,
    "ScanClients",
)

probe_helper = r'''    bool InternalUiAction(Account& a, Command command, const wchar_t* label, bool verbose = true) {
        if (a.runtime.running && a.runtime.clientFreezeActive) {
            if (verbose) LogAccount(a, std::wstring(label ? label : L"UI nội bộ") + L" bị chặn: client/map đang FREEZE");
            return false;
        }
        Response r{};
        std::wstring error;
        const bool ok = a.bridge.Call(command, 0, 0, 0, r, error, 1600);
        if (!ok && BridgeLooksUnresponsive(error)) {
            EnterClientFreeze(a, L"Bridge internal UIButton timeout/busy", GetTickCount());
        }
        if (verbose) {
            if (ok) LogAccount(a, std::wstring(label ? label : L"UI nội bộ") + L" PASS • " + std::wstring(r.detail));
            else LogAccount(a, std::wstring(label ? label : L"UI nội bộ") + L" FAIL-CLOSED • " + error);
        }
        return ok;
    }

    bool ProbeInternalConfirmUi(Account& a, std::wstring& detail) {
        Response r{};
        std::wstring error;
        const bool ok = a.bridge.Call(Command::ProbeInternalConfirm, 0, 0, 0, r, error, 1000);
        if (!ok) {
            detail = error;
            if (BridgeLooksUnresponsive(error)) {
                EnterClientFreeze(a, L"Bridge donor Confirm probe timeout/busy", GetTickCount());
            }
            return false;
        }
        detail = r.detail;
        return true;
    }

    void TestClick'''
controller = sub_once(
    controller,
    r"    bool InternalUiAction\(Account& a, Command command, const wchar_t\* label, bool verbose = true\) \{.*?\n    \}\n\n    void TestClick",
    probe_helper,
    "ProbeInternalConfirmUi",
)

start_new = r'''    void StartChecked() {
        PersistSelectedEditor();
        int started = 0;
        const int count = ListView_GetItemCount(clientList_);
        for (int i = 0; i < count && i < static_cast<int>(accounts_.size()); ++i) {
            if (!ListView_GetCheckState(clientList_, i)) continue;
            Account& a = *accounts_[static_cast<std::size_t>(i)];

            // Explicit Start is the first point where the tool is allowed to attach.
            // Immediately perform ONE core-only snapshot so RoleID-based config can be
            // loaded before automation starts. If this read fails, detach and fail closed.
            std::wstring error;
            if (!EnsureAttach(a, error)) {
                LogAccount(a, L"Không start: " + error);
                continue;
            }
            if (!ReadSnapshot(a, error, 1200)) {
                LogAccount(a, L"Không start: core ReadState thất bại • " + error);
                a.bridge.Close();
                continue;
            }

            a.displayName = DisplayName(a.snapshot, a.game.pid);
            const std::wstring identitySection = ProfileSection(a.snapshot, a.game.pid);
            if (_wcsicmp(identitySection.c_str(), a.profile.section.c_str()) != 0) {
                a.profile = LoadProfile(identitySection);
                MigrateLegacySpot(a.profile);
                if (SelectedAccount() == &a) LoadSelectedProfileToUi();
            }
            UpdateAccountRow(i, a);

            if (!a.profile.target.valid) {
                LogAccount(a, L"Không start: acc chưa chọn bãi chung.");
                a.bridge.Close();
                continue;
            }

            a.deathSessionLatched = false;
            a.rotationDeathTicks.clear();
            a.rotationMetricTick = GetTickCount();
            a.rotationActiveTrainMs = 0;
            a.rotationBagWasFull = false;
            a.runtime.running = true;
            ResetRuntime(a.runtime);
            a.runtime.running = true;
            a.runtime.routeOwnershipResetPending = true;
            a.runtime.status = L"Đang giám sát • chuẩn hóa ownership AutoPath";
            ++started;
            LogAccount(a, L"BẮT ĐẦU • core snapshot PASS • donor UI CHƯA đụng tới • bãi " +
                           a.profile.target.name + L" • M" + std::to_wstring(a.profile.target.mapID) + L" • " +
                           std::to_wstring(a.profile.target.x) + L"," + std::to_wstring(a.profile.target.y) +
                           L" • vòng " + std::to_wstring(a.profile.rotationSpots.size()) + L" bãi • chết quá " +
                           std::to_wstring(a.profile.rotateDeathLimit) + L"/" + std::to_wstring(a.profile.rotateDeathWindowMin) +
                           L" phút • chưa FULL túi " + std::to_wstring(a.profile.rotateNoFullBagMin) + L" phút");
            UpdateAccountRow(i, a);
        }
        if (started == 0) Log(L"Không có acc hợp lệ được start. Hãy tick checkbox và chọn bãi chung cho acc.");
    }

    void StopAccount'''
controller = sub_once(
    controller,
    r"    void StartChecked\(\) \{.*?\n    \}\n\n    void StopAccount",
    start_new,
    "StartChecked",
)

cross_new = r'''    bool HandleCrossMapConfirm(Account& a, DWORD now, const TargetProfile& spot) {
        RuntimeState& rt = a.runtime;
        const Snapshot& s = a.snapshot;

        // Only a cross-map route explicitly started by this tool may even probe the
        // donor UI. Generic ReadState never touches donor RVAs.
        if (!rt.crossMapRouteArmed) return false;
        if ((s.validMask & ValidMapTransition) && (!s.mapReady || s.waitingChangeMap)) {
            rt.status = L"Cross-map • game đang transition thật";
            return true;
        }

        if (s.autoPathing) rt.crossMapSeenAutoPath = true;
        const bool routeEvidence = rt.crossMapSeenAutoPath || rt.crossMapRouteMoved;
        if (!routeEvidence) {
            rt.status = L"Cross-map đã arm • chưa có bằng chứng route → cho phép retry";
            return false;
        }

        // After a successful Confirm, do not probe/click again. Wait for the real map
        // transition. A timeout only re-arms pathing; it never authorizes another blind click.
        if (rt.lastConfirmClickTick != 0) {
            if (!Elapsed(now, rt.lastConfirmClickTick, 5000)) {
                rt.status = L"Confirm đã gửi • chờ chuyển map thật";
                return true;
            }
            rt.crossMapSeenAutoPath = false;
            rt.crossMapRouteMoved = false;
            rt.confirmAttempts = 0;
            rt.lastConfirmClickTick = 0;
            rt.lastConfirmProbeTick = 0;
            rt.confirmUiFirstSeenTick = 0;
            rt.confirmStopPathTick = 0;
            rt.suppressRouteSinceTick = 0;
            rt.status = L"Confirm đã gửi nhưng map chưa đổi • cho phép AutoPath re-arm";
            return false;
        }

        // The core snapshot must prove the character is physically stopped before any
        // donor UI probe. This prevents raw donor UI access during ordinary training,
        // startup, movement, map loading, selling, etc.
        if ((s.validMask & ValidMoving) == 0) {
            rt.status = L"Cross-map • moving observer chưa authoritative → KHÔNG probe UI";
            return true;
        }
        if (s.moving) {
            rt.lastConfirmProbeTick = 0;
            rt.confirmUiFirstSeenTick = 0;
            rt.confirmStopPathTick = 0;
            rt.status = L"Cross-map • nhân vật còn di chuyển → KHÔNG probe UI";
            return true;
        }

        if (rt.lastConfirmProbeTick != 0 && !Elapsed(now, rt.lastConfirmProbeTick, 600)) {
            rt.status = L"Cross-map • đã dừng • debounce donor UI probe";
            return true;
        }
        rt.lastConfirmProbeTick = now;
        std::wstring probeDetail;
        if (!ProbeInternalConfirmUi(a, probeDetail)) {
            if (!rt.clientFreezeActive) {
                rt.confirmUiFirstSeenTick = 0;
                rt.confirmStopPathTick = 0;
                rt.status = s.autoPathing
                    ? L"Cross-map • Path ON • chưa có Confirm donor authoritative"
                    : L"Cross-map • đã dừng • chờ MessageBox Confirm thật";
            }
            return true;
        }

        if (rt.confirmUiFirstSeenTick == 0) {
            rt.confirmUiFirstSeenTick = now;
            rt.status = L"Donor Confirm READY • debounce popup 200ms";
            return true;
        }
        if (!Elapsed(now, rt.confirmUiFirstSeenTick, 200)) {
            rt.status = L"Donor Confirm READY • chờ debounce 200ms";
            return true;
        }

        // Preserve the v0.8.7 ordering: if AutoPath still reports ON at the portal,
        // stop it in one serialized cycle, then use a fresh snapshot before clicking.
        if (s.autoPathing && rt.confirmStopPathTick == 0) {
            if (SendDecision(a, Action::StopPath, spot, L"cổng trước Xác nhận")) {
                rt.confirmStopPathTick = now;
                rt.status = L"Confirm READY + Path ON → đã StopPath • chờ snapshot mới";
                LogAccount(a, L"INTERNAL CONFIRM v0.8.7: dedicated probe PASS • StopPath trước callback Confirm.");
            } else {
                rt.status = L"Confirm READY + Path ON • StopPath đang cooldown/fail-closed";
            }
            return true;
        }
        if (rt.confirmStopPathTick != 0 && !Elapsed(now, rt.confirmStopPathTick, 250)) {
            rt.status = L"Đã StopPath • chờ 250ms rồi Confirm";
            return true;
        }

        const bool debounceReady = rt.lastConfirmClickTick == 0 || Elapsed(now, rt.lastConfirmClickTick, 2500);
        if (debounceReady && rt.confirmAttempts < 2) {
            if (InternalUiAction(a, Command::ClickInternalConfirm, L"XÁC NHẬN RA MAP NỘI BỘ v0.8.7")) {
                rt.lastConfirmClickTick = now;
                ++rt.confirmAttempts;
                rt.suppressRouteSinceTick = now;
                rt.status = L"Donor Confirm READY → UIButton callback đã gửi • chờ UI/map đổi";
                LogAccount(a, L"INTERNAL CONFIRM v0.8.7: dedicated probe + callback " +
                              std::to_wstring(rt.confirmAttempts) + L"/2 • Path=" +
                              std::wstring(s.autoPathing ? L"ON(after StopPath)" : L"OFF") + L".");
                EnterClientFreeze(a, L"đã click Confirm • chờ map/client ổn định", now);
            }
        }
        return true;
    }

    bool HandleFightClicks'''
controller = sub_once(
    controller,
    r"    bool HandleCrossMapConfirm\(Account& a, DWORD now, const TargetProfile& spot\) \{.*?\n    \}\n\n    bool HandleFightClicks",
    cross_new,
    "HandleCrossMapConfirm",
)
write(controller_path, controller)


bridge_path = "src/bridge.cpp"
bridge = read(bridge_path)
bridge = sub_once(
    bridge,
    r"bool ReadConfirmVisible\(std::int32_t& visible, wchar_t\* detail, std::size_t cap\) \{.*?\n\}\n\nstruct Classes",
    "struct Classes",
    "remove ReadConfirmVisible",
)
bridge = sub_once(
    bridge,
    r"\n    // v1\.5\.10: restore the v0\.8\.7 semantic MessageBox observer\..*?\n    if \(ReadConfirmVisible\(confirmVisible, optionalDetail, _countof\(optionalDetail\)\)\) \{\n        s\.confirmUiVisible = confirmVisible \? 1 : 0;\n        s\.validMask \|= ValidConfirmUi;\n    \}\n",
    "\n    // Runtime hotfix: core ReadState must never touch donor UI RVAs.\n    // Confirm UI is probed only by Command::ProbeInternalConfirm in cross-map context.\n",
    "remove donor UI from ReadState",
)

confirm_impl = r'''bool ResolveInternalConfirmButton(std::uint64_t& button, std::wstring& label,
                                  wchar_t* detail, std::size_t cap) {
    button = 0;
    label.clear();
    std::uint64_t root = 0;
    if (!FindMessageBox(root, detail, cap) || !root) {
        SetText(detail, cap, L"MessageBox Confirm chưa tồn tại");
        return false;
    }
    std::vector<std::uint64_t> buttons;
    if (!CollectTreeButtons(root, buttons, detail, cap)) return false;
    struct Candidate { std::uint64_t object; int score; std::wstring label; };
    std::vector<Candidate> candidates;
    const std::vector<std::wstring> positive{
        L"đồng ý", L"dong y", L"xác nhận", L"xac nhan", L"confirm",
        L"buttonok", L"btnok", L"buttonyes", L"btnyes", L"yes"};
    const std::vector<std::wstring> negative{
        L"hủy", L"huy", L"không", L"khong", L"cancel", L"buttonno",
        L"btnno", L"đóng", L"close", L"thoát", L"thoat"};
    for (std::uint64_t candidate : buttons) {
        std::wstring name, text;
        if (!InspectDonorButton(candidate, name, text)) continue;
        const std::wstring candidateLabel = DonorTrim(name + L" " + text);
        const std::wstring normalized = DonorLower(candidateLabel);
        int score = DonorContainsAny(normalized, positive) ? 20 : 0;
        if (DonorContainsAny(normalized, negative)) score -= 100;
        const std::wstring exactName = DonorTrim(DonorLower(name));
        const std::wstring exactText = DonorTrim(DonorLower(text));
        if (exactName == L"ok" || exactText == L"ok" || exactText == L"có" || exactText == L"yes") score += 20;
        candidates.push_back({candidate, score, candidateLabel});
    }
    if (candidates.empty()) {
        SetText(detail, cap, L"MessageBox không có UIButton Confirm hoạt động");
        return false;
    }
    std::sort(candidates.begin(), candidates.end(), [](const Candidate& a, const Candidate& b){ return a.score > b.score; });
    const bool unambiguous = candidates.size() == 1
        ? candidates.front().score >= 0
        : candidates.front().score > 0 && candidates.front().score > candidates[1].score;
    if (!unambiguous) {
        SetText(detail, cap, L"MessageBox không có duy nhất một nút Confirm authoritative");
        return false;
    }
    button = candidates.front().object;
    label = candidates.front().label;
    return true;
}

bool ProbeInternalConfirm(wchar_t* detail, std::size_t cap) {
    Classes c{};
    if (!ResolveClasses(c, detail, cap) || !SafeForAction(c, detail, cap)) return false;
    std::uint64_t button = 0;
    std::wstring label;
    if (!ResolveInternalConfirmButton(button, label, detail, cap)) return false;
    SetText(detail, cap, L"v0.8.7 Confirm probe READY: ");
    Append(detail, cap, label.c_str());
    return true;
}

bool ClickInternalConfirm(wchar_t* detail, std::size_t cap) {
    Classes c{};
    if (!ResolveClasses(c, detail, cap) || !SafeForAction(c, detail, cap)) return false;
    std::uint64_t button = 0;
    std::wstring label;
    if (!ResolveInternalConfirmButton(button, label, detail, cap)) return false;
    if (!ClickDonorButton(button)) return false;
    SetText(detail, cap, L"v0.8.7 UIButton.HandleClickEvent Confirm PASS: ");
    Append(detail, cap, label.c_str());
    return true;
}

bool ClickInternalRevive'''
bridge = sub_once(
    bridge,
    r"bool ClickInternalConfirm\(wchar_t\* detail, std::size_t cap\) \{.*?\n\}\n\nbool ClickInternalRevive",
    confirm_impl,
    "Confirm probe/click split",
)

old_case = "            case Command::ClickInternalRevive:\n                ok = ClickInternalRevive(detail, _countof(detail));\n                r.errorCode = ok ? 0 : 1801; break;"
new_case = "            case Command::ClickInternalRevive:\n                ok = ClickInternalRevive(detail, _countof(detail));\n                r.errorCode = ok ? 0 : 1801; break;\n            case Command::ProbeInternalConfirm:\n                ok = ProbeInternalConfirm(detail, _countof(detail));\n                r.errorCode = ok ? 0 : 1901; break;"
if bridge.count(old_case) != 1:
    raise SystemExit("bridge command switch anchor mismatch")
bridge = bridge.replace(old_case, new_case, 1)
write(bridge_path, bridge)


protocol_path = "src/protocol.h"
protocol = read(protocol_path)
old = "    ClickInternalConfirm = 7,\n    ClickInternalRevive = 8,\n};"
new = "    ClickInternalConfirm = 7,\n    ClickInternalRevive = 8,\n    ProbeInternalConfirm = 9,\n};"
if protocol.count(old) != 1:
    raise SystemExit("protocol command anchor mismatch")
protocol = protocol.replace(old, new, 1)
write(protocol_path, protocol)


build_path = "build.cmd"
build = read(build_path)
build = build.replace(
    "'ClickInternalConfirm = 7','ClickInternalRevive = 8','ValidConfirmUi'",
    "'ClickInternalConfirm = 7','ClickInternalRevive = 8','ProbeInternalConfirm = 9','ValidConfirmUi'",
    1,
)
build = build.replace(
    "'HandleCrossMapConfirm','Command::ClickInternalConfirm','Command::ClickInternalRevive'",
    "'HandleCrossMapConfirm','ProbeInternalConfirmUi','Command::ClickInternalConfirm','Command::ClickInternalRevive','Command::ProbeInternalConfirm'",
    1,
)
anchor = "  \"$bridge=Get-Content -Encoding UTF8 'src/bridge.cpp' -Raw; foreach($x in @('LuaMainFindUI','LuaFindUI','UIObject.instances','UIButtonHandleClick','ReadConfirmVisible','ClickInternalConfirm','ClickInternalRevive','get_IsDeath','il2cpp_gchandle_new','il2cpp_gchandle_get_target','0x6A410C14u','0x03DCB000u')){if($bridge -notmatch [regex]::Escape($x)){throw ('Missing donor v0.8.7 bridge token: '+$x)}}; if($bridge -match 'bool ClickAt\\(' -or $bridge -match 'Command::ClickAt'){throw 'Unused generic bridge coordinate ClickAt must stay removed'};\" ^"
replacement = "  \"$bridge=Get-Content -Encoding UTF8 'src/bridge.cpp' -Raw; foreach($x in @('LuaMainFindUI','LuaFindUI','UIObject.instances','UIButtonHandleClick','ProbeInternalConfirm','ClickInternalConfirm','ClickInternalRevive','get_IsDeath','il2cpp_gchandle_new','il2cpp_gchandle_get_target','0x6A410C14u','0x03DCB000u')){if($bridge -notmatch [regex]::Escape($x)){throw ('Missing donor v0.8.7 bridge token: '+$x)}}; if($bridge -match 'ReadConfirmVisible'){throw 'Core ReadState must not poll donor UI on every snapshot'}; if($bridge -match 'bool ClickAt\\(' -or $bridge -match 'Command::ClickAt'){throw 'Unused generic bridge coordinate ClickAt must stay removed'};\" ^"
if anchor not in build:
    raise SystemExit("build bridge audit anchor mismatch")
build = build.replace(anchor, replacement, 1)
scan_audit_anchor = "  \"$controller=Get-Content -Encoding UTF8 'src/controller.cpp' -Raw; $oldConfirm=@('enableConfirm'"
# Add a dedicated passive-scan assertion just before the existing oldConfirm audit.
insert = "  \"$scan=[regex]::Match($controller,'(?s)void ScanClients\\(\\).*?void InsertAccountRow').Value; if($scan -match 'bridge\\.Attach|bridge\\.Call|ReadSnapshot'){throw 'ScanClients must stay passive: no bridge attach/call/snapshot'};\" ^\n"
pos = build.find(scan_audit_anchor)
if pos < 0:
    raise SystemExit("build controller audit anchor mismatch")
build = build[:pos] + insert + build[pos:]
write(build_path, build)


bug = Path("docs/bugs/BUG_006_STARTUP_SCAN_BRIDGE_DISS.md")
bug.parent.mkdir(parents=True, exist_ok=True)
bug.write_text(
    "# BUG-006 — Startup scan attached bridge and could diss client\n\n"
    "## Runtime evidence\n"
    "First real v1.5.10 test: merely opening the EXE / scanning a live client caused the game to disconnect/crash before automation was started.\n\n"
    "## Root cause\n"
    "`ScanClients()` had regressed from passive enumeration into `Bridge.Attach()` + immediate `ReadState`. In v1.5.10, `ReadState` also called the v0.8.7 raw-RVA MessageBox observer on every snapshot, so donor UI code could run during startup/ordinary polling.\n\n"
    "## Fix\n"
    "- Scan is passive only: EnumWindows + GameAssembly module detection.\n"
    "- Explicit Start is the first allowed bridge attach point.\n"
    "- Core `ReadState` never touches donor UI RVAs.\n"
    "- Added dedicated `ProbeInternalConfirm` command, called only for tool-owned cross-map routes after authoritative `moving=0`.\n"
    "- Revive donor scan remains gated by authoritative death state.\n\n"
    "## Validation boundary\n"
    "CI/build can prove the separation statically, but only a live client can prove the diss regression is gone. First runtime test must be: open tool -> Scan only -> wait; then Start one account and observe core snapshot before testing cross-map/revive.\n",
    encoding="utf-8",
)

changelog = Path("CHANGELOG.md")
with changelog.open("a", encoding="utf-8") as f:
    f.write("\n## v1.5.10 runtime hotfix — passive scan / lazy donor UI\n"
            "- Fix first live-test regression: opening/scanning could diss the game because ScanClients attached bridge and called ReadState immediately.\n"
            "- Scan now performs no bridge attach and no ReadState.\n"
            "- Core ReadState no longer polls donor MessageBox UI. Dedicated ProbeInternalConfirm is lazy and cross-map-context-only.\n")

print("runtime passive-scan hotfix patched successfully")
