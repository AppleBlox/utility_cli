import Cocoa
import Foundation

struct MenuItem: Codable {
    let type: String
    let title: String
    let id: String
    let icon: String?
    let isBold: Bool
    let isUnderlined: Bool
    let isDisabled: Bool
    let isChecked: Bool? // Added for checkbox support
}

struct MenuConfig: Codable {
    let trayIcon: String?
    let menuItems: [MenuItem]
    let showQuitItem: Bool
}

class TrayManager: NSObject {
    let statusItem = NSStatusBar.system.statusItem(withLength: NSStatusItem.squareLength)
    let menu = NSMenu()
    var menuItems: [MenuItem] = []
    var showQuitItem = true

    override init() {
        super.init()
        setupTray()
    }

    func setupTray() {
        if let button = statusItem.button {
            button.image = NSImage(named: "star.fill")
        }
        statusItem.menu = menu
    }

    func constructMenu() {
        menu.removeAllItems()
        for item in menuItems {
            let menuItem: NSMenuItem
            switch item.type {
            case "normal":
                menuItem = NSMenuItem(title: item.title, action: #selector(menuItemClicked(_:)), keyEquivalent: "")
                menuItem.target = self
                menuItem.representedObject = item.id
            case "separator":
                menu.addItem(NSMenuItem.separator())
                continue
            case "label":
                menuItem = NSMenuItem(title: item.title, action: nil, keyEquivalent: "")
                menuItem.isEnabled = false
            case "checkbox":
                menuItem = NSMenuItem(title: item.title, action: #selector(checkboxToggled(_:)), keyEquivalent: "")
                menuItem.target = self
                menuItem.representedObject = item.id
                if let isChecked = item.isChecked {
                    menuItem.state = isChecked ? .on : .off
                }
            default:
                printAndFlush("Unknown item type: \(item.type)")
                continue
            }
            
            if let iconName = item.icon, !iconName.isEmpty {
                if let image = NSImage(contentsOfFile: iconName) ?? NSImage(systemSymbolName: iconName, accessibilityDescription: iconName) {
                    menuItem.image = image
                }
            }
            
            if item.isBold {
                menuItem.attributedTitle = NSAttributedString(string: item.title, attributes: [.font: NSFont.boldSystemFont(ofSize: NSFont.systemFontSize)])
            }
            
            if item.isUnderlined {
                let attributes: [NSAttributedString.Key: Any] = [.underlineStyle: NSUnderlineStyle.single.rawValue]
                menuItem.attributedTitle = NSAttributedString(string: item.title, attributes: attributes)
            }
            
            if item.isDisabled {
                menuItem.isEnabled = false
            }
            
            menu.addItem(menuItem)
        }

        if showQuitItem {
            menu.addItem(NSMenuItem.separator())
            let quitItem = NSMenuItem(title: "Quit", action: #selector(quit), keyEquivalent: "q")
            quitItem.target = self
            menu.addItem(quitItem)
        }
    }

    @objc func menuItemClicked(_ sender: NSMenuItem) {
        if let id = sender.representedObject as? String {
            printAndFlush("clicked: \(id)")
        }
    }

    @objc func checkboxToggled(_ sender: NSMenuItem) {
        if let id = sender.representedObject as? String {
            sender.state = sender.state == .on ? .off : .on
            printAndFlush("Checkbox toggled: \(id), new state: \(sender.state.rawValue)")
        }
    }

    @objc func quit() {
        printAndFlush("quit")
        NSApplication.shared.terminate(nil)
    }

    func loadConfig(from jsonString: String) {
        guard let data = jsonString.data(using: .utf8) else {
            printAndFlush("Failed to load configuration data.")
            return
        }

        do {
            let decoder = JSONDecoder()
            let config = try decoder.decode(MenuConfig.self, from: data)
            self.menuItems = config.menuItems
            self.showQuitItem = config.showQuitItem

            if let trayIcon = config.trayIcon, !trayIcon.isEmpty {
                setIcon(name: trayIcon)
            }
        } catch {
            printAndFlush("Failed to decode configuration: \(error)")
        }
    }

    func setIcon(name: String) {
        if let button = statusItem.button {
            if let image = NSImage(contentsOfFile: name) ?? NSImage(systemSymbolName: name, accessibilityDescription: name) {
                button.image = image
            } else {
                printAndFlush("Failed to set icon: \(name)")
            }
        }
    }

    func printAndFlush(_ message: String) {
        print(message)
        fflush(stdout)
    }
}

let app = NSApplication.shared
let trayManager = TrayManager()

if CommandLine.arguments.count > 1 {
    if CommandLine.arguments[1] == "--config" && CommandLine.arguments.count > 2 {
        trayManager.loadConfig(from: CommandLine.arguments[2])
    }
} else {
    // Load default configuration file
    let defaultConfigPath = "./menu_config.json"
    if let data = try? Data(contentsOf: URL(fileURLWithPath: defaultConfigPath)),
       let jsonString = String(data: data, encoding: .utf8) {
        trayManager.loadConfig(from: jsonString)
    }
}

trayManager.constructMenu()
app.run()
