# utility_cli
Simple c++ scripts to perform OS level operations from anything (bash, js, etc...)

To compile, run:

```
clang++ -std=c++11 -arch arm64 -arch x86_64 -framework CoreGraphics -framework ApplicationServices window_manager.mm -o window_manager
clang -framework Foundation -framework CoreServices urlscheme.m -o urlscheme -arch x86_64 -arch arm64 -mmacosx-version-min=10.12
```

Used in [AppleBlox](https://github.com/OrigamingWasTaken/appleblox)

Note: As I'm not an expert nor do I know c++, I've used ChatGPT and Claude3.5 to write this code.
