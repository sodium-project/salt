#import <Foundation/Foundation.h>

#import <Metal/Metal.h>
#import <MetalKit/MetalKit.h>

#include <imgui.h>
#include <imgui_impl_osx.h>
#include <imgui_impl_metal.h>

#include <cstdio>

#include <salt/core.hpp>

// NOTE(Andrii):
//  Get rid of this hack in the future.
#define RUN_IMGUI_DEMO (1)

#ifdef RUN_IMGUI_DEMO
#    include "imgui-demo.mm"
#endif

void salt::application_main() {
    salt::trace("Hello from the playground!");
    salt::debug("Hello from the playground!");
    salt::info("Hello from the playground!");
    salt::warning("Hello from the playground!");
    salt::error("Hello from the playground!");
    salt::critical("Hello from the playground!");

#ifdef RUN_IMGUI_DEMO
    @autoreleasepool {
        [NSApplication sharedApplication]; // Creates the application object.

        AppDelegate* delegate = [AppDelegate new];
        [NSApp setDelegate:delegate];
        [NSApp run];
    }
#endif
}