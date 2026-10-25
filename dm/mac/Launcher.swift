// Mac launcher: a settings screen that starts the Mac host on a game, the Mac counterpart of the iOS launcher (dm/ios/Sources/Launcher.swift). dm/mac/make_launcher.py builds it into
// <Game>.app beside the game's NSP folder with two files in its Resources: launcher.json, the game's settings from dm/games/settings.tsv (dm/games/settings.py, the mac rows), and
// launch.json, how to run the game ({"host", "modules", "cwd", "log", "env"}: the command the NSP script prints for --platform mac). The choices are kept in UserDefaults (the app's
// bundle ID is per game). Play runs the host with the base environment, then the settings, and hands the focus to its window; the launcher hides while the game runs and quits when it ends, or comes back with the log's
// path when the game ended with an error. DM_LAUNCHER=0 in the launcher's environment plays at once with the saved settings, as on iOS.
import AppKit
import SwiftUI

private struct Setting: Decodable, Identifiable {
    let env, kind, label, note, def: String
    let options: [[String]]?   // choice: [value, label]
    let min, max, step: Double?   // range
    var id: String { env }
    enum CodingKeys: String, CodingKey { case env, kind, label, note, def = "default", options, min, max, step }
}
private struct SettingsFile: Decodable { let settings: [Setting] }
private struct Launch: Decodable { let host, modules, cwd, log: String; let env: [String: String] }

private func info(_ key: String) -> Any? { Bundle.main.object(forInfoDictionaryKey: key) }
private func resource<T: Decodable>(_ name: String, _ type: T.Type) -> T? {
    guard let url = Bundle.main.url(forResource: name, withExtension: "json"), let data = try? Data(contentsOf: url) else { return nil }
    return try? JSONDecoder().decode(type, from: data)
}
private let defaults = UserDefaults.standard

@MainActor private final class LauncherModel: ObservableObject {
    let settings: [Setting]
    let launch: Launch?
    @Published var values: [String: String] = [:]
    @Published var running = false
    @Published var message = ""

    init() {
        settings = resource("launcher", SettingsFile.self)?.settings ?? []
        launch = resource("launch", Launch.self)
        for s in settings {
            let saved = defaults.string(forKey: "launcher." + s.env)
            values[s.env] = saved.flatMap { v in s.options == nil || s.options!.contains { $0[0] == v } ? v : nil } ?? s.def   // a saved choice the game no longer offers: its default
        }
        if launch == nil { message = "launch.json is missing from the app: build it again with dm/mac/make_launcher.py" }
        if ProcessInfo.processInfo.environment["DM_LAUNCHER"] == "0" { DispatchQueue.main.async { self.play() } }
    }

    func binding(_ s: Setting) -> Binding<String> { Binding(get: { self.values[s.env] ?? s.def }, set: { self.values[s.env] = $0 }) }

    // The host with the base environment and the settings (kept for the next time); the game's output goes to the log, which the next run replaces
    func play() {
        guard let l = launch else { return }
        guard FileManager.default.isExecutableFile(atPath: l.host) else { message = "The Mac host is missing: \(l.host)\nRun the NSP script with --platform mac again."; return }
        var env = ProcessInfo.processInfo.environment
        for (k, v) in l.env { env[k] = v }
        for s in settings { let v = values[s.env] ?? s.def; defaults.set(v, forKey: "launcher." + s.env); env[s.env] = v }
        let p = Process()
        p.executableURL = URL(fileURLWithPath: l.host); p.arguments = [l.modules]; p.environment = env
        p.currentDirectoryURL = URL(fileURLWithPath: l.cwd)
        FileManager.default.createFile(atPath: l.log, contents: nil)
        if let out = FileHandle(forWritingAtPath: l.log) { p.standardOutput = out; p.standardError = out }
        p.terminationHandler = { proc in
            Task { @MainActor in
                self.running = false
                let ok = proc.terminationReason == .exit && proc.terminationStatus == 0
                if ok { NSApp.terminate(nil); return }
                self.message = "The game ended with \(proc.terminationReason == .exit ? "status \(proc.terminationStatus)" : "signal \(proc.terminationStatus)"). Its log: \(l.log)"
                NSApp.unhide(nil); NSApp.activate(ignoringOtherApps: true)
            }
        }
        do { try p.run() } catch { message = "Cannot start the game: \(error.localizedDescription)"; return }
        running = true; message = ""
        handOver(p.processIdentifier)
    }

    // Activation is cooperative from macOS 14: a host this app starts cannot take the focus by itself, and once the launcher hid, its window opened behind whichever app became
    // active, so the keyboard and the controllers never reached the game. The launcher (active: Play was just pressed) waits for the host's window, yields to it and activates it,
    // then hides; after 15 s without a window it hides anyway
    private func handOver(_ pid: pid_t, tries: Int = 150) {
        if let app = NSRunningApplication(processIdentifier: pid), app.activationPolicy == .regular, hasWindow(pid) {
            NSApp.yieldActivation(to: app); app.activate(from: .current, options: [])
            NSApp.hide(nil); return
        }
        guard tries > 0, running else { NSApp.hide(nil); return }
        DispatchQueue.main.asyncAfter(deadline: .now() + 0.1) { self.handOver(pid, tries: tries - 1) }
    }
    private func hasWindow(_ pid: pid_t) -> Bool {   // an on-screen window of the host at the normal level (owner and layer need no screen-recording permission)
        let list = CGWindowListCopyWindowInfo([.optionOnScreenOnly, .excludeDesktopElements], kCGNullWindowID) as? [[String: Any]] ?? []
        return list.contains { ($0[kCGWindowOwnerPID as String] as? pid_t) == pid && ($0[kCGWindowLayer as String] as? Int) == 0 }
    }
}

private struct SettingRow: View {
    let s: Setting
    @ObservedObject var m: LauncherModel
    var body: some View {
        Section {
            switch s.kind {
            case "choice":
                Picker(s.label, selection: m.binding(s)) { ForEach(s.options ?? [], id: \.[0]) { Text($0[1]).tag($0[0]) } }
            case "range":
                let lo = s.min ?? 0, hi = s.max ?? 1
                let v = Binding<Double>(get: { Double(m.values[s.env] ?? s.def) ?? hi }, set: { m.values[s.env] = String(format: "%.2f", $0) })
                HStack {
                    Text(s.label)
                    Slider(value: v, in: lo...hi, step: s.step ?? 0.05)
                    Text("\(Int((v.wrappedValue * 100).rounded())) %").monospacedDigit().foregroundStyle(.secondary).frame(width: 56, alignment: .trailing)
                }
            default:
                Toggle(s.label, isOn: Binding(get: { (m.values[s.env] ?? s.def) == "1" }, set: { m.values[s.env] = $0 ? "1" : "0" }))
            }
        } footer: {
            if !s.note.isEmpty { Text(s.note).foregroundStyle(.secondary) }
        }
    }
}

private struct LauncherView: View {
    @ObservedObject var m: LauncherModel
    var body: some View {
        HStack(alignment: .top, spacing: 0) {
            VStack(spacing: 14) {
                Image(nsImage: NSApp.applicationIconImage).resizable().frame(width: 128, height: 128)
                Text((info("CFBundleDisplayName") as? String) ?? "Game").font(.title.weight(.bold)).multilineTextAlignment(.center)
                Text("Version \((info("CFBundleShortVersionString") as? String) ?? "")").font(.callout).foregroundStyle(.secondary)
                Button(action: m.play) { Text(m.running ? "Running…" : "Play").font(.title3.weight(.semibold)).frame(maxWidth: 200) }
                    .controlSize(.large).buttonStyle(.borderedProminent).keyboardShortcut(.defaultAction).disabled(m.running || m.launch == nil)
                if !m.message.isEmpty {
                    Text(m.message).font(.callout).foregroundStyle(.orange).multilineTextAlignment(.center).textSelection(.enabled)
                }
            }
            .padding(28)
            .frame(width: 300)
            Form { ForEach(m.settings) { SettingRow(s: $0, m: m) } }
                .formStyle(.grouped)
                .frame(minWidth: 380)
        }
        .frame(minHeight: 460)
    }
}

private final class AppDelegate: NSObject, NSApplicationDelegate {
    func applicationShouldTerminateAfterLastWindowClosed(_ sender: NSApplication) -> Bool { true }   // closing the launcher quits it (a running game keeps running)
}

@main
private struct LauncherApp: App {
    @NSApplicationDelegateAdaptor(AppDelegate.self) private var delegate
    @StateObject private var model = LauncherModel()
    var body: some Scene {
        Window((info("CFBundleDisplayName") as? String) ?? "Game", id: "launcher") { LauncherView(m: model) }
            .windowResizability(.contentSize)
    }
}
