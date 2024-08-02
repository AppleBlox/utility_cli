# utility_cli
Simple c++ scripts and others to perform OS level operations from anything (bash, js, etc...)

To compile, run:

```
clang++ -std=c++11 -arch arm64 -arch x86_64 -framework CoreGraphics -framework ApplicationServices window_manager.mm -o window_manager
clang -framework Foundation -framework CoreServices urlscheme.m -o urlscheme -arch x86_64 -arch arm64 -mmacosx-version-min=10.12
swiftc -target arm64-apple-macos11 tray_builder.swift -o tray_builder_arm64
swiftc -target x86_64-apple-macos11 tray_builder.swift -o tray_builder_x64
lipo -create -output tray_builder tray_builder_arm64 tray_builder_x64
```

Used in [AppleBlox](https://github.com/OrigamingWasTaken/appleblox)

Note: As I'm not an expert nor do I know c++, I've used ChatGPT and Claude3.5 to write this code.

### Exemple for tray_builder

```
./tray_builder --config '{"trayIcon":"star.fill","menuItems":[{"type":"normal","title":"Open","id":"open","icon":"star.fill","isBold":false,"isUnderlined":false,"isDisabled":false},{"type":"separator","title":"","id":"","icon":null,"isBold":false,"isUnderlined":false,"isDisabled":false},{"type":"label","title":"Label","id":"label","icon":null,"isBold":false,"isUnderlined":false,"isDisabled":false},{"type":"checkbox","title":"Checkbox Item","id":"checkbox_item","icon":null,"isBold":false,"isUnderlined":false,"isDisabled":false,"isChecked":true},{"type":"normal","title":"Disabled Item","id":"disabled_item","icon":null,"isBold":false,"isUnderlined":false,"isDisabled":true}],"showQuitItem":false}'
```
