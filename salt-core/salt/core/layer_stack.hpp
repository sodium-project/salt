#pragma once

#include <vector>

#include <salt/core/layer.hpp>

namespace salt {

struct [[nodiscard]] Layer_stack final {

    template <typename Layer> constexpr void push() {
        layers_.push_back(Layer{});
    }

    template <typename... Args> constexpr void emplace(Args&&... args) {
        layers_.emplace_back(std::forward<Args>(args)...);
    }

    constexpr void pop() noexcept {
        layers_.pop_back();
    }

    constexpr auto& back() noexcept {
        return layers_.back();
    }

    constexpr auto const& back() const noexcept {
        return layers_.back();
    }

    constexpr auto& front() noexcept {
        return layers_.front();
    }

    constexpr auto const& front() const noexcept {
        return layers_.front();
    }

    // clang-format off
    auto begin() noexcept { return layers_.begin(); }
    auto end  () noexcept { return layers_.end();   }

    auto begin() const noexcept { return layers_.begin(); }
    auto end  () const noexcept { return layers_.end();   }
    // clang-format on

private:
    std::vector<Layer> layers_;
};

} // namespace salt