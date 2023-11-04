#pragma once
#include <salt/meta.hpp>

#include <salt/foundation/memory/addressof.hpp>

namespace salt::utility {

template <meta::random_access_iterator Iterator, typename Adapter>
class iterator_adapter final {
    using iterator_type = Iterator;
    using adapter_type  = Adapter;

public:
    using reference         = meta::deduce_t<Adapter(meta::deref_t<Iterator>)>;
    using value_type        = meta::remove_cvref_t<reference>;
    using pointer           = meta::add_pointer_t<value_type>;
    using difference_type   = meta::iter_diff_t<Iterator>;
    using iterator_category = meta::random_access_iterator_tag;
    using iterator_concept  = meta::contiguous_iterator_tag;

    iterator_type it_;
    adapter_type  adapter_;

    constexpr iterator_adapter() noexcept = default;

    constexpr iterator_adapter(iterator_type const& it, adapter_type adapter = {}) noexcept
            : it_{it}, adapter_{adapter} {}

    template <meta::convertible_to<iterator_type> It, typename Fn>
    constexpr iterator_adapter(iterator_adapter<It, Fn> const& other) noexcept
            : it_{other.it_}, adapter_{other.adapter_} {}

    [[nodiscard]] constexpr reference operator*() const noexcept {
        return adapter_(*it_);
    }

    [[nodiscard]] constexpr pointer operator->() const noexcept {
        return memory::addressof(adapter_(*it_));
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

    [[nodiscard]] constexpr difference_type
    operator-(iterator_adapter const& other) const noexcept {
        return it_ - other.it_;
    }

    [[nodiscard]] constexpr std::strong_ordering
    operator<=>(iterator_adapter const& other) const noexcept {
        return it_ <=> other.it_;
    }

    [[nodiscard]] constexpr bool operator==(iterator_adapter const& other) const noexcept {
        return it_ == other.it_;
    }

    friend constexpr auto operator+(difference_type off, iterator_adapter const& it) noexcept {
        return iterator_adapter{it.it_ + off, it.adapter_};
    }
};

} // namespace salt::utility