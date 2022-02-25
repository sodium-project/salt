#include <salt/platform/glfw_window.hpp>

// clang-format off
#include <glad/glad.h>
#include <GLFW/glfw3.h>
// clang-format on

#include <salt/config.hpp>
#include <salt/utils.hpp>

namespace salt {

static void framebuffer_size_callback(::GLFWwindow* const, int const width, int const height) {
    ::glViewport(0, 0, width, height);
}

Glfw_window::Glfw_window() noexcept : Glfw_window(Size{.width = 1280, .height = 720}) {}

Glfw_window::Glfw_window(Size const& size, Position const& position) noexcept
        : window_{nullptr}, title_{"Win64 window"}, size_{size}, position_{position}, dispatcher_{} {
    debug("Initializing Win64 window");

    char const* error_message = nullptr;
    if (!::glfwInit()) {
        ::glfwGetError(&error_message);
        error("Failed to initialize GLFW, error message: '{}'", error_message);
    }

    // GL 4.3 + GLSL 430
    [[maybe_unused]] char const* glsl_version = "#version 430";
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
}

Glfw_window::~Glfw_window() {
    ::glfwDestroyWindow(window_);
}

Size Glfw_window::size() const noexcept {
    return size_;
}

void Glfw_window::update() const noexcept {
    ::glfwPollEvents();
    ::glfwSwapBuffers(window_);
}

bool Glfw_window::alive() const noexcept {
    return !dispatcher_.holds_state<Window_close_state>();
}

} // namespace salt