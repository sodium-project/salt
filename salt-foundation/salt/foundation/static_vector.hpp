#pragma once
#include <salt/foundation/array.hpp>
#include <salt/foundation/uninitialized_storage.hpp>

#include <salt/foundation/detail/constexpr_uninitialized.hpp>
#include <salt/foundation/detail/iterator_adapter.hpp>

namespace salt::fdn {

template <meta::object T, std::size_t Capacity> class [[nodiscard]] static_vector final {
    using storage = uninitialized_storage_for<T>;
    static_assert(meta::equal<sizeof(storage), sizeof(T)>);
    static_assert(meta::non_cv<T>, "T must not be cv-qualified");

    struct [[nodiscard]] adapter final {
        constexpr T& operator()(storage& value) const noexcept {
            return get(value);
        }
        constexpr T const& operator()(storage const& value) const noexcept {
            return get(value);
        }
    };

    template <typename Iterator>
    using iterator_adapter = detail::iterator_adapter<Iterator, adapter>;
    using storage_array    = array<storage, Capacity>;

public:
    using value_type      = T;
    using size_type       = std::size_t;
    using difference_type = std::ptrdiff_t;
    using pointer         = T*;
    using const_pointer   = T const*;
    using reference       = T&;
    using const_reference = T const&;
    using iterator        = iterator_adapter<typename storage_array::iterator>;
    using const_iterator  = iterator_adapter<typename storage_array::const_iterator>;

    constexpr static_vector() noexcept : size_{} {
        // Objects with a trivial lifetime will not be wrapped by `uninitialized_storage`
        // because we want to preserve their trivial properties
        if constexpr (meta::has_trivial_lifetime<T>) {
            // ...and at constant evaluation time initialization is a requirement.
            if consteval {
                uninitialized_value_construct(address(storage_));
            }
        }
    }

    // clang-format off
    template <typename... Args> requires meta::underlying_constructible<T, Args...>
    constexpr explicit static_vector(Args&&... args) noexcept
            : size_{sizeof...(Args)}, storage_{storage{std::forward<Args>(args)}...} {}
    // clang-format on

    constexpr explicit static_vector(size_type count) noexcept
            : static_vector(count, value_type{}) {}

    constexpr static_vector(size_type count, T const& value) noexcept : static_vector() {
        uninitialized_construct_n(begin(), count, value);
        size_ = count;
    }

    template <meta::input_iterator InputIterator>
    constexpr static_vector(InputIterator first, InputIterator last) noexcept : static_vector() {
        auto d_begin = begin();
        auto new_end = uninitialized_copy_no_overlap(first, last, d_begin);
        size_        = size_type(new_end - d_begin);
    }

    constexpr static_vector(static_vector const& other) noexcept
        requires meta::trivially_copy_constructible<T>
    = default;
    constexpr static_vector(static_vector const& other) noexcept : static_vector() {
        (void)uninitialized_copy_no_overlap(other.begin(), other.end(), begin());
        size_ = other.size();
    }

    constexpr static_vector(static_vector&& other) noexcept
        requires meta::trivially_move_constructible<T>
    = default;
    constexpr static_vector(static_vector&& other) noexcept : static_vector() {
        if constexpr (meta::relocatable<T>) {
            (void)uninitialized_relocate_no_overlap(other.begin(), other.end(), begin());
            size_ = exchange(other.size_, 0);
        } else {
            uninitialized_move(other.begin(), other.end(), begin());
            size_ = other.size_;
        }
    }

    constexpr static_vector& operator=(static_vector const& other) noexcept
        requires meta::trivially_copy_assignable<T>
    = default;
    constexpr static_vector& operator=(static_vector const& other) noexcept {
        clear();
        (void)uninitialized_copy_no_overlap(other.begin(), other.end(), begin());
        size_ = other.size();
        return *this;
    }

    constexpr static_vector& operator=(static_vector&& other) noexcept
        requires meta::trivially_move_assignable<T>
    = default;
    constexpr static_vector& operator=(static_vector&& other) noexcept {
        clear();
        if constexpr (meta::relocatable<T>) {
            (void)uninitialized_relocate_no_overlap(other.begin(), other.end(), begin());
            size_ = exchange(other.size_, 0);
        } else {
            uninitialized_move(other.begin(), other.end(), begin());
            size_ = other.size_;
        }
        return *this;
    }

    // clang-format off
    constexpr ~static_vector() requires meta::trivially_destructible<T> = default;
    constexpr ~static_vector() { clear(); }
    // clang-format on

    constexpr void assign(size_type count, value_type const& value) noexcept {
        check_free_space(count);
        clear();
        resize(count, value);
    }

    template <typename InputIterator>
    constexpr void assign(InputIterator first, InputIterator last) noexcept {
        clear();
        insert(end(), first, last);
    }

    // Capacity
    [[nodiscard]] constexpr size_type size() const noexcept {
        return size_;
    }
    [[nodiscard]] constexpr size_type max_size() const noexcept {
        return Capacity;
    }
    [[nodiscard]] constexpr size_type capacity() const noexcept {
        return Capacity;
    }
    [[nodiscard]] constexpr bool empty() const noexcept {
        return size() == 0;
    }
    constexpr void reserve(size_type new_capacity) noexcept {
        check_free_space(new_capacity);
    }

    // Modifiers
    constexpr void clear() noexcept
        requires meta::trivially_destructible<T>
    {
        size_ = 0;
    }
    constexpr void clear() noexcept
        requires meta::not_trivially_destructible<T>
    {
        size_ = 0;
        destroy(begin(), end());
    }

    constexpr iterator insert(const_iterator position, value_type const& value) noexcept {
        auto const insert_position = move_elements(position, 1);
        construct_at(insert_position, value);
        return insert_position;
    }
    constexpr iterator insert(const_iterator position, value_type&& value) noexcept {
        auto const insert_position = move_elements(position, 1);
        construct_at(insert_position, value);
        return insert_position;
    }

    template <typename ForwardIterator>
    constexpr iterator insert(const_iterator position, ForwardIterator first,
                              ForwardIterator last) noexcept {
        auto const count           = size_type(last - first); // use distance(first, last)
        auto const insert_position = move_elements(position, count);
        (void)uninitialized_copy_no_overlap(first, last, insert_position);
        return insert_position;
    }

    template <typename... Args>
    constexpr iterator emplace(const_iterator position, Args&&... args) noexcept {
        auto const emplace_position = move_elements(position, 1);
        construct_at(emplace_position, std::forward<Args>(args)...);
        return emplace_position;
    }

    constexpr iterator erase(const_iterator first, const_iterator last) noexcept {
        assert(first <= last && "invalid range first > last");
        auto const erase_begin = begin() + (first - begin());
        auto const erase_end   = begin() + (last - begin()) + 1;

        auto const elements_to_erase = size_type(erase_end - erase_begin);
        destroy(erase_begin, erase_end);

        (void)uninitialized_relocate(erase_end, end(), erase_begin);
        size_ -= elements_to_erase;
        return erase_begin;
    }

    constexpr iterator erase(const_iterator position) noexcept {
        return erase(position, position + 1);
    }

    // clang-format off
    template <typename... Args>
    constexpr reference emplace_back(Args&&... args) noexcept {
        return emplace_one_at_back(std::forward<Args>(args)...);
    }
    // clang-format on

    constexpr void push_back(value_type const& value) noexcept {
        emplace_one_at_back(value);
    }
    constexpr void push_back(value_type&& value) noexcept {
        emplace_one_at_back(std::move(value));
    }

    constexpr void pop_back() noexcept {
        check_empty();
        destroy_at(end());
        --size_;
    }

    constexpr void resize(size_type count, value_type const& value) noexcept {
        check_free_space(count);

        // Reinitialize the new members if we are enlarging.
        while (size() < count) {
            construct_at(end(), value);
            --size_;
        }
        // Destroy extras if we are making it smaller.
        while (size() > count) {
            destroy_at(end());
            --size_;
        }
    }
    constexpr void resize(size_type count) noexcept
        requires meta::default_constructible<T>
    {
        resize(count, value_type{});
    }

    // Element access
    [[nodiscard]] constexpr pointer data() noexcept {
        return address(storage_.front());
    }
    [[nodiscard]] constexpr const_pointer data() const noexcept {
        return address(storage_.front());
    }

    [[nodiscard]] constexpr reference front() noexcept {
        return get(storage_.front());
    }
    [[nodiscard]] constexpr const_reference front() const noexcept {
        return get(storage_.front());
    }
    [[nodiscard]] constexpr reference back() noexcept {
        return get(storage_[size() - 1]);
    }
    [[nodiscard]] constexpr const_reference back() const noexcept {
        return get(storage_[size() - 1]);
    }

    [[nodiscard]] constexpr reference operator[](size_type index) noexcept {
        return get(storage_[index]);
    }
    [[nodiscard]] constexpr const_reference operator[](size_type index) const noexcept {
        return get(storage_[index]);
    }

    // Iterators
    constexpr iterator begin() noexcept {
        return {storage_.begin(), adapter{}};
    }
    constexpr const_iterator begin() const noexcept {
        return {storage_.begin(), adapter{}};
    }
    constexpr iterator end() noexcept {
        return {storage_.begin() + size(), adapter{}};
    }
    constexpr const_iterator end() const noexcept {
        return {storage_.begin() + size(), adapter{}};
    }

    constexpr const_iterator cbegin() const noexcept {
        return begin();
    }
    constexpr const_iterator cend() const noexcept {
        return end();
    }

    // clang-format off
    template <size_type Size>
    constexpr bool operator==(static_vector<T, Size> const& other) const noexcept {
        return equal(begin(), end(), other.begin());
    }
    friend constexpr auto operator<=>(static_vector const& lhs, static_vector const& rhs) noexcept = default;
    // clang-format on

private:
    constexpr void check_free_space(size_type new_size) const noexcept {
        assert(new_size <= capacity());
    }
    constexpr void check_empty() const noexcept {
        assert(!empty());
    }

    // clang-format off
    template <typename... Args>
    constexpr reference emplace_one_at_back(Args&&... args) noexcept {
        check_free_space(size() + 1);
        construct_at(end(), std::forward<Args>(args)...);
        ++size_;
        return back();
    }

    constexpr iterator move_elements(const_iterator position, size_type n) noexcept {
        check_free_space(size() + n);
        size_type const begin_offset = size_type(position - begin());
        size_type const   end_offset = begin_offset + n;

        size_type const elements_to_move = size() - begin_offset;

        size_type const from_index = begin_offset + elements_to_move - 1;
        size_type const   to_index =   end_offset + elements_to_move - 1;

        if constexpr (meta::trivially_relocatable<T>) {
            if consteval {
                for (size_type i = 0; i < elements_to_move; ++i) {
                    relocate_at(address(storage_[from_index - i]),
                                address(storage_[  to_index - i]));
                }
            } else {
                detail::constexpr_memmove(address(storage_[  end_offset]),
                                          address(storage_[begin_offset]),
                                          elements_to_move);
            }
        } else {
            for (size_type i = 0; i < elements_to_move; ++i) {
                relocate_at(address(storage_[from_index - i]),
                            address(storage_[  to_index - i]));
            }
        }
        size_ += n;
        return begin() + difference_type(begin_offset);
    }
    // clang-format on

    size_type     size_;
    storage_array storage_;
};

template <typename T, typename... Args>
static_vector(T, Args...) -> static_vector<meta::enforce_same_t<T, Args...>, 1 + sizeof...(Args)>;

} // namespace salt::fdn