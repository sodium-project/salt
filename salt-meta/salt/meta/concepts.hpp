#pragma once

namespace salt {

template <std::integral T> constexpr bool is_pow2(T const value) noexcept {
    return value && !((value) & (value - 1));
}

template <typename T, std::size_t Size>
concept sized = requires {
    requires sizeof(T) == Size;
};

template <typename T, std::size_t Alignment>
concept aligned = requires {
    requires alignof(T) == Alignment;
};

template <typename T>
concept aligned_as_pow2 = requires {
    requires is_pow2(alignof(T));
};

// clang-format off
template <typename T>
concept storable = (std::movable<T> || std::copyable<T>) && std::destructible<T>;
// clang-format on

template <typename Container>
concept has_reserve = requires(Container c) {
    c.reserve(std::declval<typename Container::size_type>());
};

template <typename Container>
concept has_capacity = requires(Container c) {
    c.capacity();
};

template <typename Container>
concept has_shrink_to_fit = requires(Container c) {
    c.shrink_to_fit();
};

template <typename Container>
concept has_data = requires(Container c0, const Container c1) {
    c0.data();
    c1.data();
};

struct Size;

// clang-format off
template <typename Window>
concept windowable = requires(Window window) {
    requires std::is_class_v<Window>;
    requires std::is_nothrow_constructible_v<Window, Size>
          && std::is_nothrow_destructible_v<Window>;
    { window.size()   } -> std::same_as<Size>;
    { window.update() } -> std::same_as<void>;
    { window.alive()  } -> std::same_as<bool>;
};
// clang-format on

template <windowable Window> using is_windowable = Window;

// clang-format off
template <typename Overlay, typename Window>
concept overliable = requires(Overlay overlay, Window window) {
    requires std::is_class_v<Overlay>;
    requires std::is_nothrow_constructible_v<Overlay>
          && std::is_nothrow_destructible_v<Overlay>;
    { overlay.attach(window) } -> std::same_as<void>;
    { overlay.detach() } -> std::same_as<void>;
    { overlay.render() } -> std::same_as<void>;
};
// clang-format on

template <windowable Window, overliable<Window> Overlay> using is_overliable = Overlay;

} // namespace salt