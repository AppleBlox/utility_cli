#include <iostream>
#include <string>
#include <unordered_map>
#include <CoreGraphics/CoreGraphics.h>
#include <ApplicationServices/ApplicationServices.h>

// Cache for process IDs
std::unordered_map<std::string, pid_t> processCache;

bool requestAccessibilityPermission() {
    if (AXIsProcessTrustedWithOptions(NULL)) return true;

    const void* keys[] = { kAXTrustedCheckOptionPrompt };
    const void* values[] = { kCFBooleanTrue };
    CFDictionaryRef options = CFDictionaryCreate(kCFAllocatorDefault, keys, values, 1, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    bool trusted = AXIsProcessTrustedWithOptions(options);
    CFRelease(options);
    return trusted;
}

inline bool checkError(AXError error, const char* operation) {
    if (error == kAXErrorSuccess) return true;
    
    if (error == kAXErrorAPIDisabled) {
        std::cerr << "Error: Accessibility API is disabled. Requesting permission...\n";
        if (requestAccessibilityPermission()) {
            std::cout << "Permission granted. Please run the command again.\n";
        } else {
            std::cerr << "Permission denied. Enable in System Preferences -> Security & Privacy -> Privacy -> Accessibility\n";
        }
    } else if (error == kAXErrorAttributeUnsupported) {
        std::cerr << "Error: Attribute unsupported in " << operation << '\n';
    } else if (error == -25202) {
        std::cerr << "Error: Cannot complete operation in " << operation << "\nEnsure the application is running and accessible.\n";
    } else {
        std::cerr << "Error in " << operation << ": " << error << '\n';
    }
    return false;
}

pid_t getProcessIDByName(const std::string& appName) {
    auto it = processCache.find(appName);
    if (it != processCache.end()) return it->second;

    ProcessSerialNumber psn = { 0, kNoProcess };
    CFStringRef nameRef = CFStringCreateWithCString(NULL, appName.c_str(), kCFStringEncodingUTF8);
    if (!nameRef) {
        std::cerr << "Failed to create CFString for application name\n";
        return 0;
    }

    while (GetNextProcess(&psn) == noErr) {
        CFDictionaryRef processInfo = ProcessInformationCopyDictionary(&psn, kProcessDictionaryIncludeAllInformationMask);
        if (processInfo) {
            CFStringRef processName = (CFStringRef)CFDictionaryGetValue(processInfo, kCFBundleNameKey);
            if (processName && CFStringCompare(processName, nameRef, 0) == kCFCompareEqualTo) {
                pid_t pid;
                if (GetProcessPID(&psn, &pid) == noErr) {
                    CFRelease(processInfo);
                    CFRelease(nameRef);
                    processCache[appName] = pid;
                    return pid;
                }
            }
            CFRelease(processInfo);
        }
    }

    CFRelease(nameRef);
    return 0;
}

AXUIElementRef findWindowByName(const std::string& windowName) {
    pid_t pid = getProcessIDByName("Roblox");
    if (pid == 0) {
        std::cerr << "Failed to find process ID for Roblox\n";
        return NULL;
    }

    AXUIElementRef app = AXUIElementCreateApplication(pid);
    if (!app) {
        std::cerr << "Failed to create AXUIElement for application\n";
        return NULL;
    }

    CFArrayRef windowList;
    AXError error = AXUIElementCopyAttributeValue(app, kAXWindowsAttribute, (CFTypeRef*)&windowList);
    if (!checkError(error, "AXUIElementCopyAttributeValue(kAXWindowsAttribute)")) {
        CFRelease(app);
        return NULL;
    }

    AXUIElementRef targetWindow = NULL;
    CFIndex count = CFArrayGetCount(windowList);
    for (CFIndex i = 0; i < count; i++) {
        AXUIElementRef window = (AXUIElementRef)CFArrayGetValueAtIndex(windowList, i);
        CFStringRef title;
        if (AXUIElementCopyAttributeValue(window, kAXTitleAttribute, (CFTypeRef*)&title) == kAXErrorSuccess) {
            char buffer[256];
            if (CFStringGetCString(title, buffer, sizeof(buffer), kCFStringEncodingUTF8) && windowName == buffer) {
                targetWindow = window;
                CFRetain(targetWindow);
                CFRelease(title);
                break;
            }
            CFRelease(title);
        }
    }

    CFRelease(windowList);
    CFRelease(app);

    if (!targetWindow) std::cerr << "Window not found: " << windowName << '\n';
    return targetWindow;
}

bool moveWindow(const std::string& windowName, int x, int y) {
    AXUIElementRef window = findWindowByName(windowName);
    if (!window) return false;

    CGPoint position = { static_cast<CGFloat>(x), static_cast<CGFloat>(y) };
    AXValueRef positionRef = AXValueCreate(kAXValueTypeCGPoint, &position);
    if (!positionRef) {
        std::cerr << "Failed to create AXValue for position\n";
        CFRelease(window);
        return false;
    }

    AXError error = AXUIElementSetAttributeValue(window, kAXPositionAttribute, positionRef);
    CFRelease(positionRef);
    CFRelease(window);

    if (!checkError(error, "AXUIElementSetAttributeValue(kAXPositionAttribute)")) return false;
    std::cout << "Window moved successfully\n";
    return true;
}

bool resizeWindow(const std::string& windowName, int width, int height) {
    AXUIElementRef window = findWindowByName(windowName);
    if (!window) return false;

    CGSize size = { static_cast<CGFloat>(width), static_cast<CGFloat>(height) };
    AXValueRef sizeRef = AXValueCreate(kAXValueTypeCGSize, &size);
    if (!sizeRef) {
        std::cerr << "Failed to create AXValue for size\n";
        CFRelease(window);
        return false;
    }

    AXError error = AXUIElementSetAttributeValue(window, kAXSizeAttribute, sizeRef);
    CFRelease(sizeRef);
    CFRelease(window);

    if (!checkError(error, "AXUIElementSetAttributeValue(kAXSizeAttribute)")) return false;
    std::cout << "Window resized successfully\n";
    return true;
}

bool renameWindow(const std::string& oldName, const std::string& newName) {
    AXUIElementRef window = findWindowByName(oldName);
    if (!window) return false;

    CFStringRef newNameRef = CFStringCreateWithCString(NULL, newName.c_str(), kCFStringEncodingUTF8);
    if (!newNameRef) {
        std::cerr << "Failed to create CFString for new name\n";
        CFRelease(window);
        return false;
    }

    AXError error = AXUIElementSetAttributeValue(window, kAXTitleAttribute, newNameRef);
    CFRelease(newNameRef);
    CFRelease(window);

    if (!checkError(error, "AXUIElementSetAttributeValue(kAXTitleAttribute)")) return false;
    std::cout << "Window renamed successfully\n";
    return true;
}

int main(int argc, char* argv[]) {
    if (argc < 4) {
        std::cout << "Usage: " << argv[0] << " move|resize|rename <window_name> <param1> [param2]\n";
        return 1;
    }

    std::string command = argv[1];
    std::string windowName = argv[2];

    try {
        if (command == "move" && argc == 5) {
            return moveWindow(windowName, std::stoi(argv[3]), std::stoi(argv[4])) ? 0 : 1;
        } else if (command == "resize" && argc == 5) {
            return resizeWindow(windowName, std::stoi(argv[3]), std::stoi(argv[4])) ? 0 : 1;
        } else if (command == "rename" && argc == 4) {
            return renameWindow(windowName, argv[3]) ? 0 : 1;
        } else {
            std::cout << "Invalid command or arguments\n";
            return 1;
        }
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << '\n';
        return 1;
    }
}
