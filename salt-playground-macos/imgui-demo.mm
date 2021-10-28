#include <imgui_impl_osx.h>

@interface AppViewController : NSViewController
@end

@interface AppViewController () <MTKViewDelegate>
@property (nonatomic, readonly, nonnull) MTKView *mtkView;
@property (nonatomic, strong, nonnull) id<MTLDevice> device;
@property (nonatomic, strong, nonnull) id<MTLCommandQueue> commandQueue;
@end

//-----------------------------------------------------------------------------------
// AppViewController
//-----------------------------------------------------------------------------------

@implementation AppViewController

-(nonnull instancetype)initWithNibName:(nullable NSString*)nibNameOrNil bundle:(nullable NSBundle*)nibBundleOrNil
{
    self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil];

    _device = MTLCreateSystemDefaultDevice();
    _commandQueue = [_device newCommandQueue];

    if (!self.device)
    {
        NSLog(@"Metal is not supported");
        abort();
    }

    // Setup Dear ImGui context
    // FIXME: This example doesn't have proper cleanup...
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsClassic();

    // Setup Renderer backend
    ImGui_ImplMetal_Init(_device);

    // Load Fonts
    // - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
    // - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
    // - If the file cannot be loaded, the function will return NULL. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
    // - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
    // - Read 'docs/FONTS.txt' for more instructions and details.
    // - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
    //io.Fonts->AddFontDefault();
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/ProggyTiny.ttf", 10.0f);
    //ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, NULL, io.Fonts->GetGlyphRangesJapanese());
    //IM_ASSERT(font != NULL);

    return self;
}

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wold-style-cast"
-(nonnull MTKView*)mtkView
{
    return (MTKView*)self.view;
}
#pragma clang diagnostic pop

-(void)loadView
{
    self.view = [[MTKView alloc] initWithFrame:CGRectMake(0, 0, 800, 600)];
}

-(void)viewDidLoad
{
    [super viewDidLoad];

    self.mtkView.device = self.device;
    self.mtkView.delegate = self;

    // Add a tracking area in order to receive mouse events whenever the mouse is within the bounds of our view
    NSTrackingArea *trackingArea = [[NSTrackingArea alloc] initWithRect:NSZeroRect
                                                                options:NSTrackingMouseMoved | NSTrackingInVisibleRect | NSTrackingActiveAlways
                                                                  owner:self
                                                               userInfo:nil];
    [self.view addTrackingArea:trackingArea];

    // If we want to receive key events, we either need to be in the responder chain of the key view,
    // or else we can install a local monitor. The consequence of this heavy-handed approach is that
    // we receive events for all controls, not just Dear ImGui widgets. If we had native controls in our
    // window, we'd want to be much more careful than just ingesting the complete event stream.
    // To match the behavior of other backends, we pass every event down to the OS.
    NSEventMask eventMask = NSEventMaskKeyDown | NSEventMaskKeyUp | NSEventMaskFlagsChanged;
    [NSEvent addLocalMonitorForEventsMatchingMask:eventMask handler:^NSEvent * _Nullable(NSEvent *event)
    {
        ImGui_ImplOSX_HandleEvent(event, self.view);
        return event;
    }];

    ImGui_ImplOSX_Init();
}

-(void)drawInMTKView:(nonnull MTKView*)view
{
    ImGuiIO& io = ImGui::GetIO();
    io.DisplaySize.x = static_cast<float>(view.bounds.size.width);
    io.DisplaySize.y = static_cast<float>(view.bounds.size.height);

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wfloat-conversion"
#pragma clang diagnostic ignored "-Wgnu-conditional-omitted-operand"
    CGFloat framebufferScale = view.window.screen.backingScaleFactor ?: NSScreen.mainScreen.backingScaleFactor;
#pragma clang diagnostic pop

    io.DisplayFramebufferScale = ImVec2(static_cast<float>(framebufferScale), static_cast<float>(framebufferScale));

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wgnu-conditional-omitted-operand"
    io.DeltaTime = 1 / float(view.preferredFramesPerSecond ?: 60);
#pragma clang diagnostic pop

    id<MTLCommandBuffer> commandBuffer = [self.commandQueue commandBuffer];

    MTLRenderPassDescriptor* renderPassDescriptor = view.currentRenderPassDescriptor;
    if (renderPassDescriptor == nil)
    {
        [commandBuffer commit];
		return;
    }

    // Start the Dear ImGui frame
    ImGui_ImplMetal_NewFrame(renderPassDescriptor);
    ImGui_ImplOSX_NewFrame(view);
    ImGui::NewFrame();

    // Our state (make them static = more or less global) as a convenience to keep the example terse.
    static bool show_demo_window = true;
    static bool show_another_window = false;
    static ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    // 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
    if (show_demo_window)
        ImGui::ShowDemoWindow(&show_demo_window);

    // 2. Show a simple window that we create ourselves. We use a Begin/End pair to created a named window.
    {
        static float f = 0.0f;
        static int counter = 0;

        ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

        ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
        ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state
        ImGui::Checkbox("Another Window", &show_another_window);

        ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
        ImGui::ColorEdit3("clear color",
                          reinterpret_cast<float*>(&clear_color)); // Edit 3 floats representing a color

        if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
            counter++;
        ImGui::SameLine();
        ImGui::Text("counter = %d", counter);

        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
                        static_cast<double>(1000.0f / ImGui::GetIO().Framerate),
                        static_cast<double>(ImGui::GetIO().Framerate));
        ImGui::End();
    }

    // 3. Show another simple window.
    if (show_another_window)
    {
        ImGui::Begin("Another Window", &show_another_window);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
        ImGui::Text("Hello from another window!");
        if (ImGui::Button("Close Me"))
            show_another_window = false;
        ImGui::End();
    }

    // Rendering
    ImGui::Render();
    ImDrawData* draw_data = ImGui::GetDrawData();

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdouble-promotion"
    renderPassDescriptor.colorAttachments[0].clearColor = MTLClearColorMake(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
#pragma clang diagnostic pop
    
    id <MTLRenderCommandEncoder> renderEncoder = [commandBuffer renderCommandEncoderWithDescriptor:renderPassDescriptor];
    [renderEncoder pushDebugGroup:@"Dear ImGui rendering"];
    ImGui_ImplMetal_RenderDrawData(draw_data, commandBuffer, renderEncoder);
    [renderEncoder popDebugGroup];
    [renderEncoder endEncoding];

	// Present
    [commandBuffer presentDrawable:view.currentDrawable];
    [commandBuffer commit];
}

-(void)mtkView:(nonnull MTKView*)view drawableSizeWillChange:(CGSize)size
{
}

//-----------------------------------------------------------------------------------
// Input processing
//-----------------------------------------------------------------------------------

// Forward Mouse/Keyboard events to Dear ImGui OSX backend.
// Other events are registered via addLocalMonitorForEventsMatchingMask()
-(void)mouseDown:(nonnull NSEvent*)event           { ImGui_ImplOSX_HandleEvent(event, self.view); }
-(void)rightMouseDown:(nonnull NSEvent*)event      { ImGui_ImplOSX_HandleEvent(event, self.view); }
-(void)otherMouseDown:(nonnull NSEvent*)event      { ImGui_ImplOSX_HandleEvent(event, self.view); }
-(void)mouseUp:(nonnull NSEvent*)event             { ImGui_ImplOSX_HandleEvent(event, self.view); }
-(void)rightMouseUp:(nonnull NSEvent*)event        { ImGui_ImplOSX_HandleEvent(event, self.view); }
-(void)otherMouseUp:(nonnull NSEvent*)event        { ImGui_ImplOSX_HandleEvent(event, self.view); }
-(void)mouseMoved:(nonnull NSEvent*)event          { ImGui_ImplOSX_HandleEvent(event, self.view); }
-(void)mouseDragged:(nonnull NSEvent*)event        { ImGui_ImplOSX_HandleEvent(event, self.view); }
-(void)rightMouseMoved:(nonnull NSEvent*)event     { ImGui_ImplOSX_HandleEvent(event, self.view); }
-(void)rightMouseDragged:(nonnull NSEvent*)event   { ImGui_ImplOSX_HandleEvent(event, self.view); }
-(void)otherMouseMoved:(nonnull NSEvent*)event     { ImGui_ImplOSX_HandleEvent(event, self.view); }
-(void)otherMouseDragged:(nonnull NSEvent*)event   { ImGui_ImplOSX_HandleEvent(event, self.view); }
-(void)scrollWheel:(nonnull NSEvent*)event         { ImGui_ImplOSX_HandleEvent(event, self.view); }

@end

//-----------------------------------------------------------------------------------
// AppDelegate
//-----------------------------------------------------------------------------------

@interface AppDelegate : NSObject <NSApplicationDelegate>
@property (nonatomic, strong, nonnull) NSWindow *window;
@end

@implementation AppDelegate

-(nonnull instancetype)init
{
    if (self = [super init])
    {
        NSViewController *rootViewController = [[AppViewController alloc] initWithNibName:nil bundle:nil];
        self.window = [[NSWindow alloc] initWithContentRect:NSZeroRect
                                                  styleMask:NSWindowStyleMaskTitled | NSWindowStyleMaskClosable | NSWindowStyleMaskResizable | NSWindowStyleMaskMiniaturizable
                                                    backing:NSBackingStoreBuffered
                                                      defer:NO];
        self.window.contentViewController = rootViewController;
        [self.window orderFront:self];
        [self.window center];
        [self.window becomeKeyWindow];
    }
    return self;
}

- (void)applicationDidFinishLaunching:(nonnull NSNotification*)notification {
    (void)notification;
    NSBundle* applicaiton_bundle = [NSBundle mainBundle];
    NSDictionary* bundle_info    = [applicaiton_bundle infoDictionary];
    NSString* application_name   = bundle_info[@"CFBundleName"];
    [self initializeMainMenu:application_name];
    self.window.title = application_name;
    [self.window cascadeTopLeftFromPoint:NSMakePoint(20, 20)];
    [self.window makeKeyAndOrderFront:self];
}

-(BOOL)applicationShouldTerminateAfterLastWindowClosed:(nonnull NSApplication*)sender
{
    (void)sender;
    return YES;
}

- (void)initializeMainMenu:(nonnull NSString*)application_name {
    NSMenu* main_menu = [NSMenu new];
    [NSApp setMainMenu:main_menu];
    {
        NSMenuItem* app_item = [[NSMenuItem alloc] initWithTitle:application_name action:nil keyEquivalent:@""];
        [main_menu addItem:app_item];
        {
            NSMenu* app_menu = [NSMenu new];
            [app_item setSubmenu:app_menu];
            {
                NSMenuItem* quit_item =
                    [[NSMenuItem alloc] initWithTitle:NSLocalizedString(@"Quit", @"Quit the application.")
                                               action:@selector(terminate:)
                                        keyEquivalent:@"q"];
                [app_menu addItem:quit_item];
            }
        }
    }
}

@end
