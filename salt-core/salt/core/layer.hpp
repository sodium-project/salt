#pragma once
#include <system_error>
#include <utility>

namespace salt {

class [[nodiscard]] Layer {
    struct [[nodiscard]] Base {
        virtual constexpr void  update()      = 0;
        virtual constexpr void  overlay()     = 0;
        virtual constexpr Base* clone() const = 0;
        virtual constexpr ~Base()             = default;
    };

    template <typename T> struct [[nodiscard]] Wrapper final : public Base {
        constexpr Wrapper(T&& object) noexcept : object_{std::move(object)} {}

        constexpr Base* clone() const override {
            if constexpr (std::copyable<T>)
                return new Wrapper<T>(*this);
            else
                throw std::system_error(ENOTSUP, std::generic_category());
        }

        constexpr void update() override {
            object_.update();
        }

        constexpr void overlay() override {
            object_.overlay();
        }

    private:
        T object_;
    };

public:
    // clang-format off
    template <typename T> requires(!std::derived_from<T, Layer>)
    constexpr Layer(T object)
            : ptr_{new Wrapper<T>{std::move(object)}} {}

    constexpr Layer(Layer const& other)
            : ptr_{other.ptr_->clone()} {}
    constexpr Layer(Layer&& other) noexcept
            : ptr_{std::exchange(other.ptr_, nullptr)} {}
    // clang-format on

    constexpr ~Layer() {
        delete ptr_;
    }

    constexpr Layer& operator=(Layer const& other) {
        delete ptr_;
        ptr_ = other.ptr_->clone();
        return *this;
    }
    constexpr Layer& operator=(Layer&& other) noexcept {
        if (std::addressof(other) == this)
            return *this;
        delete ptr_;
        ptr_ = std::exchange(other.ptr_, nullptr);
        return *this;
    }

    constexpr void update() {
        ptr_->update();
    }

    constexpr void overlay() {
        ptr_->overlay();
    }

private:
    Base* ptr_;
};

} // namespace salt