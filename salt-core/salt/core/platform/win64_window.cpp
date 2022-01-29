#include <salt/core/platform/win64_window.hpp>

#include <salt/config.hpp>
#include <salt/utils.hpp>

namespace salt {

Win64_window::Win64_window(Size const& size) : title_{"Win64 window"}, size_{size}, position_{} {
    debug("Initializing Win64 window");

    char const* error_message = nullptr;
    if (!::glfwInit()) {
        ::glfwGetError(&error_message);
        error("Failed to initialize GLFW, error message: '{}'", error_message);
    }

    window_ = ::glfwCreateWindow(int(size_.width), int(size_.height), title_.c_str(), nullptr, nullptr);
    if (!window_) {
        ::glfwGetError(&error_message);
        error("Failed to create Window, error message: '{}'", error_message);
    } else {
        trace("Window created");
    }
    ::glfwMakeContextCurrent(window_);
    ::glfwSetWindowUserPointer(window_, static_cast<void*>(&dispatcher_));

    trace("Window size => width:{}, height:{}", size_.width, size_.height);

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

Win64_window::Win64_window(Size const& size, Position const& position)
        : title_{"Win64 window"}, size_{size}, position_{position} {
    debug("Initializing Win64 window");

    char const* error_message = nullptr;
    if (!::glfwInit()) {
        ::glfwGetError(&error_message);
        error("Failed to initialize GLFW, error message: '{}'", error_message);
    }

    window_ = ::glfwCreateWindow(int(size_.width), int(size_.height), title_.c_str(), nullptr, nullptr);
    if (!window_) {
        ::glfwGetError(&error_message);
        error("Failed to create Window, error message: '{}'", error_message);
    } else {
        trace("Window created");
    }
    ::glfwSetWindowPos(window_, position_.x, position_.y);
    ::glfwMakeContextCurrent(window_);

    ::glfwSetWindowUserPointer(window_, static_cast<void*>(&dispatcher_));

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

Win64_window::~Win64_window() {
    ::glfwDestroyWindow(window_);
}

Size Win64_window::size() const noexcept {
    return size_;
}

Point Win64_window::position() const noexcept {
    return position_;
}

void Win64_window::update() const noexcept {
    ::glfwPollEvents();
    ::glfwSwapBuffers(window_);
}

bool Win64_window::alive() const noexcept {
    return !dispatcher_.holds_state<Window_close_state>();
}

} // namespace salt