#include <salt/platform/glfw_window.hpp>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <imgui_stdlib.h>

#include <salt/config.hpp>
#include <salt/utils.hpp>

namespace salt {

static void framebuffer_size_callback(::GLFWwindow* const, int const width, int const height) {
    ::glViewport(0, 0, width, height);
}

Glfw_window::Glfw_window(Size const& size, Position const& position) noexcept
        : title_{"Win64 window"}, size_{size}, position_{position}, dispatcher_{} {
    debug("Initializing Win64 window");

    char const* error_message = nullptr;
    if (!::glfwInit()) {
        ::glfwGetError(&error_message);
        error("Failed to initialize GLFW, error message: '{}'", error_message);
    }

    // GL 4.3 + GLSL 430
    char const* glsl_version = "#version 430";
    ::glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    ::glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    ::glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window_ = ::glfwCreateWindow(int(size_.width), int(size_.height), title_.data(), nullptr, nullptr);
    if (!window_) {
        ::glfwGetError(&error_message);
        error("Failed to create Window, error message: '{}'", error_message);
    } else {
        trace("Window created");
    }
    ::glfwSetWindowPos(window_, position_.x, position_.y);
    ::glfwMakeContextCurrent(window_);
    ::glfwSetFramebufferSizeCallback(window_, framebuffer_size_callback);

    ::glfwSetWindowUserPointer(window_, static_cast<void*>(&dispatcher_));

    // glad: load all OpenGL function pointers
    if (!::gladLoadGLLoader(reinterpret_cast<::GLADloadproc>(::glfwGetProcAddress))) {
        trace("Failed to initialize GLAD");
    }

    trace("Window size => width:{}, height:{}", size_.width, size_.height);
    trace("Window position => x:{}, y:{}", position_.x, position_.y);

    // Set GLFW callbacks
    ::glfwSetWindowCloseCallback(window_, [](::GLFWwindow* const window) {
        auto const& dispatcher = *static_cast<Event_dispatcher const*>(::glfwGetWindowUserPointer(window));
        dispatcher.dispatch(Window_close_event{});
    });

    ::glfwSetWindowSizeCallback(window_, [](::GLFWwindow* const window, int const width, int const height) {
        auto const& dispatcher = *static_cast<Event_dispatcher const*>(::glfwGetWindowUserPointer(window));
        dispatcher.dispatch(Window_resize_event{.size = {std::size_t(width), std::size_t(height)}});
    });

    // clang-format off
    ::glfwSetKeyCallback(window_, [](::GLFWwindow* const window, int const key, int const scancode, int const action,
                                     int const mods) {
        (void)scancode;
        (void)mods;
        auto const& dispatcher = *static_cast<Event_dispatcher const*>(::glfwGetWindowUserPointer(window));

        switch (action) {
            break; case GLFW_PRESS  : dispatcher.dispatch(Key_pressed_event{.key = key});
            break; case GLFW_RELEASE: dispatcher.dispatch(Key_released_event{.key = key});
            break; case GLFW_REPEAT : dispatcher.dispatch(Key_pressed_event{.key = key});
        }
    });

    ::glfwSetMouseButtonCallback(window_, [](::GLFWwindow* const window, int const button, int const action,
                                             int const mods) {
        (void)mods;
        auto const& dispatcher = *static_cast<Event_dispatcher const*>(::glfwGetWindowUserPointer(window));

        switch (action) {
            break; case GLFW_PRESS  : dispatcher.dispatch(Mouse_pressed_event{.button = button});
            break; case GLFW_RELEASE: dispatcher.dispatch(Mouse_released_event{.button = button});
        }
    });
    // clang-format on

    // Setup ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    ::ImGui::StyleColorsDark();

    auto& colors              = ImGui::GetStyle().Colors;
    colors[ImGuiCol_WindowBg] = ImVec4{0.1f, 0.105f, 0.11f, 1.0f};

    // Headers
    colors[ImGuiCol_Header]        = ImVec4{0.2f, 0.205f, 0.21f, 1.0f};
    colors[ImGuiCol_HeaderHovered] = ImVec4{0.3f, 0.305f, 0.31f, 1.0f};
    colors[ImGuiCol_HeaderActive]  = ImVec4{0.15f, 0.1505f, 0.151f, 1.0f};

    // Buttons
    colors[ImGuiCol_Button]        = ImVec4{0.2f, 0.205f, 0.21f, 1.0f};
    colors[ImGuiCol_ButtonHovered] = ImVec4{0.3f, 0.305f, 0.31f, 1.0f};
    colors[ImGuiCol_ButtonActive]  = ImVec4{0.15f, 0.1505f, 0.151f, 1.0f};

    // Frame BG
    colors[ImGuiCol_FrameBg]        = ImVec4{0.2f, 0.205f, 0.21f, 1.0f};
    colors[ImGuiCol_FrameBgHovered] = ImVec4{0.3f, 0.305f, 0.31f, 1.0f};
    colors[ImGuiCol_FrameBgActive]  = ImVec4{0.15f, 0.1505f, 0.151f, 1.0f};

    // Tabs
    colors[ImGuiCol_Tab]                = ImVec4{0.15f, 0.1505f, 0.151f, 1.0f};
    colors[ImGuiCol_TabHovered]         = ImVec4{0.38f, 0.3805f, 0.381f, 1.0f};
    colors[ImGuiCol_TabActive]          = ImVec4{0.28f, 0.2805f, 0.281f, 1.0f};
    colors[ImGuiCol_TabUnfocused]       = ImVec4{0.15f, 0.1505f, 0.151f, 1.0f};
    colors[ImGuiCol_TabUnfocusedActive] = ImVec4{0.2f, 0.205f, 0.21f, 1.0f};

    // Title
    colors[ImGuiCol_TitleBg]          = ImVec4{0.15f, 0.1505f, 0.151f, 1.0f};
    colors[ImGuiCol_TitleBgActive]    = ImVec4{0.15f, 0.1505f, 0.151f, 1.0f};
    colors[ImGuiCol_TitleBgCollapsed] = ImVec4{0.15f, 0.1505f, 0.151f, 1.0f};

    ImGui_ImplGlfw_InitForOpenGL(window_, true);
    ImGui_ImplOpenGL3_Init(glsl_version);
}

Glfw_window::~Glfw_window() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    ::glfwDestroyWindow(window_);
}

Size Glfw_window::size() const noexcept {
    return size_;
}

void Glfw_window::update() const noexcept {
    ::glfwPollEvents();

    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    // Docking
    static bool               fullscreen      = true;
    static bool               open            = true;
    static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

    // We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
    // because it would be confusing to have two docking targets within each others.
    ImGuiWindowFlags     window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
    const ImGuiViewport* viewport     = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);
    ImGui::SetNextWindowViewport(viewport->ID);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize |
                    ImGuiWindowFlags_NoMove;
    window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

    // When using ImGuiDockNodeFlags_PassthruCentralNode, DockSpace() will render our background
    // and handle the pass-thru hole, so we ask Begin() to not render a background.
    if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
        window_flags |= ImGuiWindowFlags_NoBackground;

    // Important: note that we proceed even if Begin() returns false (aka window is collapsed).
    // This is because we want to keep our DockSpace() active. If a DockSpace() is inactive,
    // all active windows docked into it will lose their parent and become undocked.
    // We cannot preserve the docking relationship between an active window and an inactive docking, otherwise
    // any change of dockspace/settings would lead to windows being stuck in limbo and never being visible.
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
            // Disabling fullscreen would allow the window to be moved to the front of other windows,
            // which we can't undo at the moment without finer window depth/z control.
            ImGui::MenuItem("Fullscreen", NULL, &fullscreen);
            ImGui::Separator();

            if (ImGui::MenuItem("Flag: NoSplit", "", (dockspace_flags & ImGuiDockNodeFlags_NoSplit) != 0)) {
                dockspace_flags ^= ImGuiDockNodeFlags_NoSplit;
            }
            if (ImGui::MenuItem("Flag: NoResize", "", (dockspace_flags & ImGuiDockNodeFlags_NoResize) != 0)) {
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
    std::string xxx = fmt::to_string(buffer);
    ImGui::InputTextMultiline("##text1", &xxx, ImGui::GetWindowSize(), ImGuiInputTextFlags_ReadOnly);
    ImGui::PopStyleColor();
    ImGui::End();
    ImGui::PopStyleVar();

    ImGui::Begin("Settings");
    ImGui::Text("Some additional info");
    ImGui::End();

    ImGui::End();

    // Rendering
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    ::glfwSwapBuffers(window_);
}

bool Glfw_window::alive() const noexcept {
    return !dispatcher_.holds_state<Window_close_state>();
}

} // namespace salt