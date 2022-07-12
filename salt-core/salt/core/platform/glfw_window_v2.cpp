#include <salt/core/platform/glfw_window_v2.hpp>
#include <salt/core/window.hpp>

// clang-format off
#include <glad/glad.h>
#include <GLFW/glfw3.h>
// clang-format on

SALT_DEFINE_OPAQUE_TYPE(Window)

//**************************************************************************************************
namespace salt::detail {

GLFW_window::~GLFW_window() {
    ::glfwDestroyWindow(window);
}

} // namespace salt::detail

//**************************************************************************************************
namespace salt {

Size Window::size() const noexcept {
    return guts_->size;
}

Position Window::position() const noexcept {
    return guts_->position;
}

void Window::update() const noexcept {
    ::glfwPollEvents();
    ::glfwSwapBuffers(guts_->window);
}

} // namespace salt

//**************************************************************************************************
namespace salt {

static void framebuffer_size_callback(::GLFWwindow* const, int const width, int const height) {
    ::glViewport(0, 0, width, height);
}

Window make_default_window() noexcept {
    debug("Initializing Win64 window");

    char const* error_message = nullptr;
    if (!::glfwInit()) {
        ::glfwGetError(&error_message);
        error("Failed to initialize GLFW, error message: '{}'", error_message);
    }

    // GL 4.3 + GLSL 430
    // [[maybe_unused]] char const* glsl_version = "#version 430";
    // ::glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    // ::glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    // ::glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    auto const    size   = Size{.width = 1280, .height = 720};
    auto const    title  = "Win64 Window";
    ::GLFWwindow* window = ::glfwCreateWindow(size.width, size.height, title, nullptr, nullptr);
    if (!window) {
        ::glfwGetError(&error_message);
        error("Failed to create Window, error message: '{}'", error_message);
    }
    trace("Window created");

    auto const position = Position{.x = 500, .y = 500};
    ::glfwSetWindowPos(window, position.x, position.y);
    ::glfwMakeContextCurrent(window);
    ::glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // glad: load all OpenGL function pointers
    // if (!::gladLoadGLLoader(reinterpret_cast<::GLADloadproc>(::glfwGetProcAddress))) {
    //     trace("Failed to initialize GLAD");
    // }

    trace("Window size => width:{}, height:{}", size.width, size.height);
    trace("Window position => x:{}, y:{}", position.x, position.y);

    auto guts = Static_storage_for<detail::GLFW_window>{in_place};
    {
        guts->window   = window;
        guts->size     = size;
        guts->position = position;
    }

    return Window{std::move(guts)};
}

} // namespace salt