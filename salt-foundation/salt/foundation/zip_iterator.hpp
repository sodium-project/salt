#pragma once
#include <salt/foundation/pair.hpp>

namespace salt {

template <std::random_access_iterator Iter0, std::random_access_iterator Iter1>
struct [[nodiscard]] Zip_iterator final {
    // clang-format off
    using value_type       = cxx23::pair<std::iter_value_t<Iter0>, std::iter_value_t<Iter1>>;
    using reference        = cxx23::pair<std::iter_reference_t<Iter0>, std::iter_reference_t<Iter1>>;
    using rvalue_reference = cxx23::pair<std::iter_rvalue_reference_t<Iter0>,
                                         std::iter_rvalue_reference_t<Iter1>>;
    using difference_type  = std::common_type_t<std::iter_difference_t<Iter0>,
                                                std::iter_difference_t<Iter1>>;
    // clang-format on

    constexpr Zip_iterator(Iter0 first = {}, Iter1 second = {}) noexcept
            : first_{first}, second_{second} {}

    constexpr reference operator*() const noexcept {
        return {*first_, *second_};
    }

    constexpr auto operator->() const noexcept {
        struct Proxy_pointer : reference {
            using reference::reference;
            const Proxy_pointer* operator->() const noexcept {
                return this;
            }
        };
        return Proxy_pointer{*first_, *second_};
    }

    constexpr Zip_iterator& operator++() noexcept {
        ++first_;
        ++second_;
        return *this;
    }

    constexpr Zip_iterator operator++(int) const noexcept {
        auto const temp = *this;
        ++(*this);
        return temp;
    }

    constexpr Zip_iterator& operator--() noexcept {
        --first_;
        --second_;
        return *this;
    }

    constexpr Zip_iterator operator--(int) const noexcept {
        auto const temp = *this;
        --(*this);
        return temp;
    }

    constexpr bool operator==(Zip_iterator const& other) const noexcept {
        bool const is_equal_first  = first_ == other.first_;
        bool const is_equal_second = second_ == other.second_;
        SALT_ASSERT(is_equal_first == is_equal_second);
        return is_equal_first;
    }

    constexpr std::weak_ordering operator<=>(Zip_iterator const& other) const noexcept {
        auto const order_first  = std::weak_order(first_, other.first_);
        auto const order_second = std::weak_order(second_, other.second_);
        SALT_ASSERT(order_first == order_second);
        return order_first;
    }

    constexpr difference_type operator-(Zip_iterator const& other) const noexcept {
        auto const distance_first  = first_ - other.first_;
        auto const distance_second = second_ - other.second_;
        SALT_ASSERT(distance_first == distance_second);
        return distance_first;
    }

    constexpr Zip_iterator operator+(difference_type d) const noexcept {
        auto temp = *this;
        return temp += d;
    }

    constexpr Zip_iterator operator-(difference_type d) const noexcept {
        auto temp = *this;
        return temp -= d;
    }

    constexpr Zip_iterator& operator+=(difference_type d) noexcept {
        first_ += d;
        second_ += d;
        return *this;
    }

    constexpr Zip_iterator& operator-=(difference_type d) noexcept {
        first_ -= d;
        second_ -= d;
        return *this;
    }

    constexpr reference operator[](difference_type idx) const noexcept {
        return {first_[idx], second_[idx]};
    }

    // clang-format off
    friend constexpr rvalue_reference iter_move(Zip_iterator it) noexcept (
        noexcept(std::ranges::iter_move(it.first_)) and
        noexcept(std::ranges::iter_move(it.second_))
    ) {
        return {std::ranges::iter_move(it.first_), std::ranges::iter_move(it.second_)};
    }

    friend constexpr void iter_swap(Zip_iterator l, Zip_iterator r) noexcept (
        noexcept(std::ranges::iter_swap(l.first_, r.first_)) and
        noexcept(std::ranges::iter_swap(l.second_, r.second_))
    ) {
        std::ranges::iter_swap(l.first_, r.first_);
        std::ranges::iter_swap(l.second_, r.second_);
    }
    // clang-format on

private:
    Iter0 first_;
    Iter1 second_;
};

template <std::forward_iterator Iter0, std::forward_iterator Iter1>
constexpr Zip_iterator<Iter0, Iter1>
operator+(typename Zip_iterator<Iter0, Iter1>::difference_type d,
          Zip_iterator<Iter0, Iter1>                           it) noexcept {
    return it + d;
}

} // namespace salt