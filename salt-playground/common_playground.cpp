#include <cstdio>
#include <imgui.h>
#include <salt/core.hpp>

// NOTE(Andrii):
//  Get rid of this hack in the future.
// #define RUN_IMGUI_DEMO (1)

#ifdef RUN_IMGUI_DEMO
#    include "imgui-demo.inl"
#endif

struct [[nodiscard]] Playground_layer final {
    void update() {}

    void on_overlay_render() {
        // Docking
        static bool               fullscreen      = true;
        static bool               open            = true;
        static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

        // We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable
        // into, because it would be confusing to have two docking targets within each others.
        ImGuiWindowFlags     window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
        ImGuiViewport const* viewport     = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->WorkPos);
        ImGui::SetNextWindowSize(viewport->WorkSize);
        ImGui::SetNextWindowViewport(viewport->ID);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse |
                        ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
        window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

        // When using ImGuiDockNodeFlags_PassthruCentralNode, DockSpace() will render our background
        // and handle the pass-thru hole, so we ask Begin() to not render a background.
        if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
            window_flags |= ImGuiWindowFlags_NoBackground;

        // Important: note that we proceed even if Begin() returns false (aka window is collapsed).
        // This is because we want to keep our DockSpace() active. If a DockSpace() is inactive,
        // all active windows docked into it will lose their parent and become undocked.
        // We cannot preserve the docking relationship between an active window and an inactive
        // docking, otherwise any change of dockspace/settings would lead to windows being stuck in
        // limbo and never being visible.
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{0, 0});
        ImGui::Begin("DockSpace Demo", &open, window_flags);
        ImGui::PopStyleVar();

        ImGui::PopStyleVar(2);

        // Submit the DockSpace
        ImGuiIO& io = ImGui::GetIO();
        if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable) {
            ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
            ImGui::DockSpace(dockspace_id, ImVec2{0, 0}, dockspace_flags);
        }

        if (ImGui::BeginMenuBar()) {
            if (ImGui::BeginMenu("Options")) {
                // Disabling fullscreen would allow the window to be moved to the front of other
                // windows, which we can't undo at the moment without finer window depth/z control.
                ImGui::MenuItem("Fullscreen", NULL, &fullscreen);
                ImGui::Separator();

                if (ImGui::MenuItem("Flag: NoSplit", "",
                                    (dockspace_flags & ImGuiDockNodeFlags_NoSplit) != 0)) {
                    dockspace_flags ^= ImGuiDockNodeFlags_NoSplit;
                }
                if (ImGui::MenuItem("Flag: NoResize", "",
                                    (dockspace_flags & ImGuiDockNodeFlags_NoResize) != 0)) {
                    dockspace_flags ^= ImGuiDockNodeFlags_NoResize;
                }
                ImGui::EndMenu();
            }

            ImGui::EndMenuBar();
        }

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{0, 0});
        ImGui::Begin("Viewport");
        [[maybe_unused]] ImVec2 viewport_panel_size = ImGui::GetContentRegionAvail();
        ImGui::Text("Hello from another window!");
        ImGui::End();
        ImGui::PopStyleVar();

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{0, 0});
        ImGui::Begin("Output");
        ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(106, 90, 205, 255));
        ImGui::Text("Some text!");
        ImGui::PopStyleColor();
        ImGui::End();
        ImGui::PopStyleVar();

        ImGui::Begin("Settings");
        ImGui::Text("Some additional info");
        ImGui::End();

        ImGui::End();
    }
};

void salt::main(Application& app) {
#ifdef RUN_IMGUI_DEMO
    show_imgui_demo();
#endif
    (void)app;
    app.push<Playground_layer>();
}