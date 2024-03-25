#include <salt/core.hpp>

// NOTE(Andrii):
//  Get rid of this hack in the future.
// #define RUN_IMGUI_DEMO (1)

#ifdef RUN_IMGUI_DEMO
#    include <imgui.h>
#    include "imgui-demo.inl"
#endif

void salt::main(application& app) {
#ifdef RUN_IMGUI_DEMO
    show_imgui_demo();
#endif
    (void)app;
}