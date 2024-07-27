#import <Foundation/Foundation.h>
#import <CoreServices/CoreServices.h>

int main(int argc, const char * argv[]) {
    @autoreleasepool {
        if (argc != 3) {
            NSLog(@"Usage: %s <URL_SCHEME> <BUNDLE_IDENTIFIER>", argv[0]);
            return 1;
        }
        
        NSString *urlScheme = [NSString stringWithUTF8String:argv[1]];
        NSString *bundleIdentifier = [NSString stringWithUTF8String:argv[2]];
        
        OSStatus status = LSSetDefaultHandlerForURLScheme((__bridge CFStringRef)urlScheme, (__bridge CFStringRef)bundleIdentifier);
        
        if (status == noErr) {
            NSLog(@"Successfully set %@ as the default handler for %@://", bundleIdentifier, urlScheme);
            return 0;
        } else {
            NSLog(@"Failed to set default handler. Error code: %d", (int)status);
            return 1;
        }
    }
    return 0;
}
