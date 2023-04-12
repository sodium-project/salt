#include <salt/platform/opengl_window.hpp>

// clang-format off
#include <glad/glad.h>
#include <GLFW/glfw3.h>
// clang-format on

#include <salt/config.hpp>
#include <salt/foundation.hpp>

namespace salt {

static void framebuffer_size_callback(::GLFWwindow*, int width, int height) noexcept {
    ::glViewport(0, 0, width, height);
}

Opengl_window::Opengl_window() noexcept : Opengl_window(Size{.width = 1280, .height = 720}) {}

Opengl_window::Opengl_window(Size size, Position position) noexcept
        : native_window_{nullptr}, title_{"OpenGL Window"}, size_{std::move(size)},
          position_{std::move(position)}, vsync_{true}, bus_{} {
    debug("Initializing OpenGL Window");

    char const* error_message = nullptr;
    if (!::glfwInit()) {
        ::glfwGetError(&error_message);
        error("Failed to initialize GLFW, error message: '", std::string_view{error_message}, "'");
    }

    ::glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, SALT_OPENGL_VERSION_MAJOR);
    ::glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, SALT_OPENGL_VERSION_MINOR);
    ::glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    int const width  = static_cast<int>(size_.width);
    int const height = static_cast<int>(size_.height);
    native_window_   = ::glfwCreateWindow(width, height, title_.data(), nullptr, nullptr);
    if (!native_window_) {
        ::glfwGetError(&error_message);
        error("Failed to create Window, error message: '", std::string_view{error_message}, "'");
    }

    trace("Window created");

    ::glfwSetWindowPos(native_window_, position_.x, position_.y);
    ::glfwMakeContextCurrent(native_window_);
    ::glfwSetFramebufferSizeCallback(native_window_, framebuffer_size_callback);
    ::glfwSwapInterval(vsync_);

    ::glfwSetWindowUserPointer(native_window_, static_cast<void*>(&bus_));

    // glad: load all OpenGL function pointers
    if (!::gladLoadGLLoader(reinterpret_cast<::GLADloadproc>(::glfwGetProcAddress))) {
        trace("Failed to initialize GLAD");
    }

    trace("Window size => {width:", width, ", height:", height, "}");
    trace("Window position => {x:", position_.x, ", y:", position_.y, "}");

    // Set GLFW callbacks
    ::glfwSetWindowCloseCallback(native_window_, [](::GLFWwindow* window) {
        auto& bus = *static_cast<event_bus*>(::glfwGetWindowUserPointer(window));
        bus.dispatch<Window_close_event>();
    });

    // clang-format off
    ::glfwSetWindowSizeCallback(native_window_, [](::GLFWwindow* window, int width, int height) {
        auto& bus  = *static_cast<event_bus*>(::glfwGetWindowUserPointer(window));
        bus.dispatch<Window_resize_event>(Size{.width  = static_cast<std::size_t>(width),
                                               .height = static_cast<std::size_t>(height)});
    });

    ::glfwSetWindowPosCallback(native_window_, [](GLFWwindow* window, int xpos, int ypos) {
        auto& bus      = *static_cast<event_bus*>(::glfwGetWindowUserPointer(window));
        bus.dispatch<Window_move_event>(Position{.x = xpos, .y = ypos});
    });

    ::glfwSetKeyCallback(
            native_window_, [](::GLFWwindow* window, int key, int scancode, int action, int mods) {
        (void)scancode;
        (void)mods;
        auto& bus = *static_cast<event_bus*>(::glfwGetWindowUserPointer(window));

        switch (action) {
            break; case GLFW_PRESS  : bus.dispatch<Key_pressed_event >(key);
            break; case GLFW_RELEASE: bus.dispatch<Key_released_event>(key);
            break; case GLFW_REPEAT : bus.dispatch<Key_pressed_event >(key);
        }
    });

    ::glfwSetMouseButtonCallback(
            native_window_, [](::GLFWwindow* window, int button, int action, int mods) {
        (void)mods;
        auto& bus = *static_cast<event_bus*>(::glfwGetWindowUserPointer(window));

        switch (action) {
            break; case GLFW_PRESS  : bus.dispatch<Mouse_pressed_event >(button);
            break; case GLFW_RELEASE: bus.dispatch<Mouse_released_event>(button);
        }
    });
    // clang-format on
}

Opengl_window::~Opengl_window() {
    ::glfwDestroyWindow(native_window_);
}

Size Opengl_window::size() const noexcept {
    return size_;
}

Position Opengl_window::position() const noexcept {
    return position_;
}

bool Opengl_window::vsync() const noexcept {
    return vsync_;
}

void Opengl_window::vsync(bool enable) noexcept {
    if (vsync_ != enable) {
        vsync_ = enable;
        ::glfwSwapInterval(vsync_);
    }
}

void Opengl_window::update() const noexcept {
    ::glfwPollEvents();
    ::glfwSwapBuffers(native_window_);
}

} // namespace salt