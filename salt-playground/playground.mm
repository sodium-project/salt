#import <Foundation/Foundation.h>
#import <Metal/Metal.h>
#import <MetalKit/MetalKit.h>

#include <salt/core.hpp>

// NOTE(Andrii):
//  Get rid of this hack in the future.
// #define RUN_IMGUI_DEMO (1)

#ifdef RUN_IMGUI_DEMO
#    include <imgui.h>
#    include <imgui_impl_metal.h>
#    include <imgui_impl_osx.h>
#    include "imgui-demo.mm"
#endif

void salt::main(application& app) {
    (void)app;

#ifdef RUN_IMGUI_DEMO
    @autoreleasepool {
        [NSApplication sharedApplication]; // Creates the application object.

        AppDelegate* delegate = [AppDelegate new];
        [NSApp setDelegate:delegate];
        [NSApp run];
    }
#endif
}