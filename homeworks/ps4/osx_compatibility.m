#ifdef PLATFORM_DARWIN
#import <Cocoa/Cocoa.h>

 NSWindow* getFirstWindow() {
    // Get the shared application instance
    NSApplication *app = [NSApplication sharedApplication];

    // Get the array of all windows in the application
    NSArray *windows = [app windows];

    // Iterate through each window and print its title
    NSLog(@"Window count of current app: %lu", [windows count]);
    return windows[0];
}

extern void forceFocusWindow_OSX() {
    NSWindow* window = getFirstWindow();
    [[NSApplication sharedApplication] activateIgnoringOtherApps:YES];
    [[NSApp mainWindow] makeKeyAndOrderFront:nil];
}

#endif