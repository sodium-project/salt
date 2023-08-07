#pragma once
#include <salt/meta.hpp>

namespace salt::fdn::detail {

template <meta::random_access_iterator Iterator, typename Adapter>
class [[nodiscard]] iterator_adapter final {
    using iterator_traits = std::iterator_traits<Iterator>;

public:
    using reference         = meta::deduce_t<Adapter(meta::dereference_t<Iterator>)>;
    using value_type        = meta::remove_cvref_t<reference>;
    using pointer           = meta::add_pointer_t<value_type>;
    using difference_type   = typename iterator_traits::difference_type;
    using iterator_category = typename iterator_traits::iterator_category;
    using iterator_concept  = typename iterator_traits::iterator_concept;

    constexpr iterator_adapter() noexcept = default;

    constexpr iterator_adapter(Iterator const& it, Adapter adapter) noexcept
            : it_{it}, adapter_{adapter} {}

    [[nodiscard]] constexpr reference operator*() const noexcept {
        return adapter_(*it_);
    }

    [[nodiscard]] constexpr pointer operator->() const noexcept {
        return std::addressof(adapter_(*it_));
    }

    [[nodiscard]] constexpr reference operator[](difference_type index) const noexcept {
        return adapter_(it_[index]);
    }

    constexpr iterator_adapter& operator++() noexcept {
        ++it_;
        return *this;
    }

    constexpr iterator_adapter operator++(int) noexcept {
        return {it_++, adapter_};
    }

    constexpr iterator_adapter& operator--() noexcept {
        --it_;
        return *this;
    }

    constexpr iterator_adapter operator--(int) noexcept {
        return {it_--, adapter_};
    }

    constexpr iterator_adapter& operator+=(difference_type off) noexcept {
        it_ += off;
        return *this;
    }

    constexpr iterator_adapter& operator-=(difference_type off) noexcept {
        it_ -= off;
        return *this;
    }

    constexpr iterator_adapter operator+(difference_type off) const noexcept {
        return {it_ + off, adapter_};
    }

    constexpr iterator_adapter operator-(difference_type off) const noexcept {
        return {it_ - off, adapter_};
    }

    // clang-format off
    [[nodiscard]]
    constexpr difference_type operator-(iterator_adapter const& other) const noexcept {
        return it_ - other.it_;
    }

    [[nodiscard]]
    constexpr std::strong_ordering operator<=>(iterator_adapter const& other) const noexcept {
        return it_ <=> other.it_;
    }

    [[nodiscard]]
    constexpr bool operator==(iterator_adapter const& other) const noexcept {
        return it_ == other.it_;
    }
    // clang-format on

    friend constexpr auto operator+(difference_type off, iterator_adapter const& it) noexcept {
        return iterator_adapter{it.it_ + off, it.adapter_};
    }

private:
    Iterator it_;
    Adapter  adapter_;
};

} // namespace salt::fdn::detail