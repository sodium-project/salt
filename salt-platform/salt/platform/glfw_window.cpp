#include <salt/platform/glfw_window.hpp>

// clang-format off
#include <glad/glad.h>
#include <GLFW/glfw3.h>
// clang-format on

#include <salt/config.hpp>
#include <salt/foundation.hpp>

namespace salt {

static void framebuffer_size_callback(::GLFWwindow* const, int const width, int const height) {
    ::glViewport(0, 0, width, height);
}

Glfw_window::Glfw_window() noexcept : Glfw_window(Size{.width = 1280, .height = 720}) {}

Glfw_window::Glfw_window(Size size, Position position) noexcept
        : window_{nullptr}, title_{"Win64 window"}, size_{std::move(size)},
          position_{std::move(position)}, vsync_{false}, bus_{} {
    debug("Initializing Win64 window");

    // set callbacks
    // clang-format off
    bus_.attach<Window_close_event  >([&](auto& event) { on(event); });
    bus_.attach<Window_resize_event >([&](auto& event) { on(event); });
    bus_.attach<Key_pressed_event   >([&](auto& event) { on(event); });
    bus_.attach<Key_released_event  >([&](auto& event) { on(event); });
    bus_.attach<Mouse_pressed_event >([&](auto& event) { on(event); });
    bus_.attach<Mouse_released_event>([&](auto& event) { on(event); });
    // clang-format on

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

    window_ = ::glfwCreateWindow(int(size_.width), int(size_.height), title_.data(), nullptr,
                                 nullptr);
    if (!window_) {
        ::glfwGetError(&error_message);
        error("Failed to create Window, error message: '{}'", error_message);
    } else {
        trace("Window created");
    }
    ::glfwSetWindowPos(window_, position_.x, position_.y);
    ::glfwMakeContextCurrent(window_);
    ::glfwSetFramebufferSizeCallback(window_, framebuffer_size_callback);

    ::glfwSetWindowUserPointer(window_, static_cast<void*>(&bus_));

    // glad: load all OpenGL function pointers
    if (!::gladLoadGLLoader(reinterpret_cast<::GLADloadproc>(::glfwGetProcAddress))) {
        trace("Failed to initialize GLAD");
    }

    trace("Window size => width:{}, height:{}", size_.width, size_.height);
    trace("Window position => x:{}, y:{}", position_.x, position_.y);

    // Set GLFW callbacks
    ::glfwSetWindowCloseCallback(window_, [](::GLFWwindow* const window) {
        auto& bus = *static_cast<event_bus*>(::glfwGetWindowUserPointer(window));
        bus.dispatch<Window_close_event>();
    });

    // clang-format off
    ::glfwSetWindowSizeCallback(window_, [](::GLFWwindow* const window, int const width, int const height) {
        auto& bus = *static_cast<event_bus*>(::glfwGetWindowUserPointer(window));
        bus.dispatch<Window_resize_event>(Size{std::size_t(width), std::size_t(height)});
    });

    ::glfwSetKeyCallback(window_, [](::GLFWwindow* const window, int const key, int const scancode, int const action,
                                     int const mods) {
        (void)scancode;
        (void)mods;
        auto& bus = *static_cast<event_bus*>(::glfwGetWindowUserPointer(window));

        switch (action) {
            break; case GLFW_PRESS  : bus.dispatch<Key_pressed_event >(key);
            break; case GLFW_RELEASE: bus.dispatch<Key_released_event>(key);
            break; case GLFW_REPEAT : bus.dispatch<Key_pressed_event >(key);
        }
    });

    ::glfwSetMouseButtonCallback(window_, [](::GLFWwindow* const window, int const button, int const action,
                                             int const mods) {
        (void)mods;
        auto& bus = *static_cast<event_bus*>(::glfwGetWindowUserPointer(window));

        switch (action) {
            break; case GLFW_PRESS  : bus.dispatch<Mouse_pressed_event >(button);
            break; case GLFW_RELEASE: bus.dispatch<Mouse_released_event>(button);
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
    return true;
}

bool Glfw_window::vsync() const noexcept {
    return vsync_;
}

void Glfw_window::vsync(bool flag) noexcept {
    vsync_ = flag;
}

Position Glfw_window::position() const noexcept {
    return position_;
}

void Glfw_window::on(Window_close_event& event) noexcept {
    (void)event;
}

void Glfw_window::on(Window_resize_event& event) noexcept {
    auto const& new_size = event->size;
    size_                = new_size;
    trace("Window resized, width: {}, height: {}", new_size.width, new_size.height);
}

void Glfw_window::on(Key_pressed_event& event) noexcept {
    trace("Key pressed, key code: {}", event->key);
}

void Glfw_window::on(Key_released_event& event) noexcept {
    trace("Key released, key code: {}", event->key);
}

void Glfw_window::on(Mouse_pressed_event& event) noexcept {
    trace("Mouse pressed, button code: {}", event->button);
}

void Glfw_window::on(Mouse_released_event& event) noexcept {
    trace("Mouse released, button code: {}", event->button);
}

} // namespace salt