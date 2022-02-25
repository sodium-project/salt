#pragma once

#include <vector>

#include <entt/core/type_traits.hpp>
#include <entt/poly/poly.hpp>

namespace salt {

struct [[nodiscard]] Layer final : entt::type_list<> {
    template <typename Base> struct type : Base {
        void attach() const {
            entt::poly_call<0>(*this);
        }
    };

    template <typename Type> using impl = entt::value_list<&Type::attach>;
};

using Poly_layer = entt::poly<Layer>;

struct [[nodiscard]] Layer_stack final {

    template <typename Layer> constexpr void push() noexcept {
        layers_.emplace_back(Layer{});
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

private:
    std::vector<Poly_layer> layers_;
};

} // namespace salt