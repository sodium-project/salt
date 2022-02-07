#include <cstdio>
#include <salt/core.hpp>

// NOTE(Andrii):
//  Get rid of this hack in the future.
// #define RUN_IMGUI_DEMO (1)

#ifdef RUN_IMGUI_DEMO
#    include "imgui-demo.inl"
#endif

void salt::application_main() {
#ifdef RUN_IMGUI_DEMO
    show_imgui_demo();
#endif
}