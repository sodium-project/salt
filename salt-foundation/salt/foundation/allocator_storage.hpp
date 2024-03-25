#pragma once
#include <salt/foundation/construct_at.hpp>
#include <salt/foundation/threading.hpp>

namespace salt::fdn {

// An `allocator_storage` that stores another allocator.
// NOTE:
// - `StorageType` defines the allocator type being stored and how it is stored;
// - `Mutex` controls synchronization of the access.
template <typename StorageType, typename Mutex>
class [[nodiscard]] allocator_storage : StorageType, mutex_adapter_t<StorageType, Mutex> {
    using storage_mutex     = mutex_adapter_t<StorageType, Mutex> const;
    using storage_type      = StorageType;
    using allocator_traits  = allocator_traits<typename StorageType::allocator_type>;
    using composable_traits = composable_traits<typename StorageType::allocator_type>;

public:
    using allocator_type  = typename StorageType::allocator_type;
    using mutex_type      = Mutex;
    using size_type       = std::size_t;
    using difference_type = std::ptrdiff_t;
    using stateful        = typename allocator_traits::stateful;

    constexpr allocator_storage() noexcept = default;

    // clang-format off
    template <typename Allocator> requires(
        // use this to prevent constructor being chosen instead of move
        not meta::derived_from<meta::decay_t<Allocator>, allocator_storage>
        and meta::only_constructible<StorageType, Allocator>)
    constexpr explicit(false) allocator_storage(Allocator&& allocator) noexcept
            : storage_type{std::forward<Allocator>(allocator)} {}

    template <typename OtherStorage>
        requires requires(allocator_storage<OtherStorage, Mutex> const& other) {
            new storage_type{other.allocator()};
        }
    constexpr allocator_storage(allocator_storage<OtherStorage, Mutex> const& other) noexcept
            : storage_type{other.allocator()} {}

    constexpr allocator_storage(allocator_storage&& other) noexcept
            : storage_type {std::move(other)},
              storage_mutex{std::move(other)} {}

    constexpr allocator_storage& operator=(allocator_storage&& other) noexcept {
        storage_type ::operator=(std::move(other));
        storage_mutex::operator=(std::move(other));
        return *this;
    }
    // clang-format on

    constexpr allocator_storage(allocator_storage const&) noexcept            = default;
    constexpr allocator_storage& operator=(allocator_storage const&) noexcept = default;

    constexpr void* allocate_node(size_type size, size_type alignment) noexcept {
        lock_guard_t<storage_mutex> guard{*this};
        auto&&                      alloc = allocator();
        return allocator_traits::allocate_node(alloc, size, alignment);
    }

    constexpr void* allocate_array(size_type count, size_type size, size_type alignment) noexcept {
        lock_guard_t<storage_mutex> guard{*this};
        auto&&                      alloc = allocator();
        return allocator_traits::allocate_array(alloc, count, size, alignment);
    }

    constexpr void deallocate_node(void* ptr, size_type size, size_type alignment) noexcept {
        lock_guard_t<storage_mutex> guard{*this};
        auto&&                      alloc = allocator();
        allocator_traits::deallocate_node(alloc, ptr, size, alignment);
    }

    constexpr void deallocate_array(void* ptr, size_type count, size_type size,
                                    size_type alignment) noexcept {
        lock_guard_t<storage_mutex> guard{*this};
        auto&&                      alloc = allocator();
        allocator_traits::deallocate_array(alloc, ptr, count, size, alignment);
    }

    constexpr auto try_allocate_node(size_type size, size_type alignment) noexcept
        requires composable_allocator<allocator_type>
    {
        lock_guard_t<storage_mutex> guard{*this};
        auto&&                      alloc = allocator();
        return composable_traits::try_allocate_node(alloc, size, alignment);
    }

    constexpr auto try_allocate_array(size_type count, size_type size, size_type alignment) noexcept
        requires composable_allocator<allocator_type>
    {
        lock_guard_t<storage_mutex> guard{*this};
        auto&&                      alloc = allocator();
        return composable_traits::try_allocate_array(alloc, count, size, alignment);
    }

    constexpr auto try_deallocate_node(void* ptr, size_type size, size_type alignment) noexcept
        requires composable_allocator<allocator_type>
    {
        lock_guard_t<storage_mutex> guard{*this};
        auto&&                      alloc = allocator();
        return composable_traits::try_deallocate_node(alloc, ptr, size, alignment);
    }

    constexpr auto try_deallocate_array(void* ptr, size_type count, size_type size,
                                        size_type alignment) noexcept
        requires composable_allocator<allocator_type>
    {
        lock_guard_t<storage_mutex> guard{*this};
        auto&&                      alloc = allocator();
        return composable_traits::try_deallocate_array(alloc, ptr, count, size, alignment);
    }

    constexpr size_type max_node_size() const noexcept {
        lock_guard_t<storage_mutex> guard{*this};
        auto&&                      alloc = allocator();
        return allocator_traits::max_node_size(alloc);
    }

    constexpr size_type max_array_size() const noexcept {
        lock_guard_t<storage_mutex> guard{*this};
        auto&&                      alloc = allocator();
        return allocator_traits::max_array_size(alloc);
    }

    constexpr size_type max_alignment() const noexcept {
        lock_guard_t<storage_mutex> guard{*this};
        auto&&                      alloc = allocator();
        return allocator_traits::max_alignment(alloc);
    }

    constexpr explicit operator bool() const noexcept {
        return storage_type::operator bool();
    }

    constexpr decltype(auto) allocator() noexcept {
        return storage_type::allocator();
    }
    constexpr decltype(auto) allocator() const noexcept {
        return storage_type::allocator();
    }

    constexpr auto guard() noexcept {
        return detail::lock_allocator(allocator(), static_cast<storage_mutex&>(*this));
    }
    constexpr auto guard() const noexcept {
        return detail::lock_allocator(allocator(), static_cast<storage_mutex&>(*this));
    }

    constexpr bool is_composable() const noexcept {
        return storage_type::is_composable();
    }
};

// Tag type that enables type-erasure in `reference_storage`.
// It can be used anywhere the `allocator_reference` is used internally.
struct [[nodiscard]] any_allocator final {
    using allocator_type  = any_allocator;
    using size_type       = std::size_t;
    using difference_type = std::ptrdiff_t;
};

// A `StorageType` that stores the `allocator` directly.
// It embeds the allocator inside it, i.e. moving the storage will move the allocator.
template <raw_allocator RawAllocator>
    requires(not meta::same_as<RawAllocator, any_allocator>)
struct [[nodiscard]] direct_storage : allocator_traits<RawAllocator>::allocator_type {
    using allocator_type = typename allocator_traits<RawAllocator>::allocator_type;

    constexpr direct_storage() noexcept = default;

    constexpr explicit(false) direct_storage(allocator_type&& allocator) noexcept
            : allocator_type(std::move(allocator)) {}

    constexpr direct_storage(direct_storage&& other) noexcept
            : allocator_type(std::move(other)) {}

    constexpr direct_storage& operator=(direct_storage&& other) noexcept {
        allocator_type::operator=(std::move(other));
        return *this;
    }

    constexpr allocator_type& allocator() noexcept {
        return *this;
    }

    constexpr allocator_type const& allocator() const noexcept {
        return *this;
    }

protected:
    constexpr ~direct_storage() noexcept = default;

    constexpr bool is_composable() const noexcept {
        return is_composable_allocator<allocator_type>;
    }
};

// An alias template for `allocator_storage` using the `direct_storage` policy without a `mutex`.
// It has the effect of giving any `concept raw_allocator` the interface with all member functions,
// avoiding the need to wrap it inside the `allocator_traits`.
template <raw_allocator RawAllocator>
using allocator_adapter = allocator_storage<direct_storage<RawAllocator>, no_mutex>;

namespace detail {

struct [[nodiscard]] reference_stateful  final {};
struct [[nodiscard]] reference_stateless final {};
struct [[nodiscard]] reference_shared    final {};

// clang-format off
reference_stateful  reference_type(meta::true_type  stateful, meta::false_type shared);
reference_stateless reference_type(meta::false_type stateful, meta::true_type  shared);
reference_stateless reference_type(meta::false_type stateful, meta::false_type shared);
reference_shared    reference_type(meta::true_type  stateful, meta::true_type  shared);

template <raw_allocator RawAllocator>
struct [[nodiscard]] allocator_reference final {
    using type = decltype(reference_type(meta::declval<is_stateful_allocator<RawAllocator>>(),
                                         meta::declval<  is_shared_allocator<RawAllocator>>()));
};
template <raw_allocator RawAllocator>
using allocator_reference_t = typename allocator_reference<RawAllocator>::type;

template <raw_allocator RawAllocator, typename Tag>
struct [[nodiscard]] reference_storage_base;
// clang-format on

// Reference to stateful allocator: stores a pointer to `RawAllocator`.
template <raw_allocator RawAllocator>
struct [[nodiscard]] reference_storage_base<RawAllocator, reference_stateful> {
protected:
    constexpr reference_storage_base() noexcept : allocator_{nullptr} {}

    constexpr explicit(false) reference_storage_base(RawAllocator& allocator) noexcept
            : allocator_{&allocator} {}

    constexpr bool is_valid() const noexcept {
        return allocator_ != nullptr;
    }

    constexpr RawAllocator& allocator() const noexcept {
        SALT_ASSERT(is_valid());
        return *allocator_;
    }

private:
    RawAllocator* allocator_;
};

// Reference to stateless allocator: stores `RawAllocator` statically.
template <raw_allocator RawAllocator>
struct [[nodiscard]] reference_storage_base<RawAllocator, reference_stateless> {
protected:
    constexpr reference_storage_base() noexcept = default;

    constexpr explicit(false) reference_storage_base(RawAllocator const&) noexcept {}

    constexpr bool is_valid() const noexcept {
        return true;
    }

    /* constexpr */ RawAllocator& allocator() const noexcept {
        static RawAllocator alloc;
        return alloc;
    }
};

// Reference to shared allocator: stores `RawAllocator` directly.
template <raw_allocator RawAllocator>
struct [[nodiscard]] reference_storage_base<RawAllocator, reference_shared> {
protected:
    constexpr reference_storage_base() noexcept : allocator_{} {}

    constexpr explicit(false) reference_storage_base(RawAllocator const& allocator) noexcept
            : allocator_{allocator} {}

    constexpr bool is_valid() const noexcept {
        return true;
    }

    constexpr RawAllocator& allocator() const noexcept {
        return allocator_;
    }

private:
    mutable RawAllocator allocator_;
};

} // namespace detail

template <typename RawAllocator>
using reference_storage_base =
        detail::reference_storage_base<typename allocator_traits<RawAllocator>::allocator_type,
                                       detail::allocator_reference_t<RawAllocator>>;

// A `StorageType` that stores a reference to an allocator.
// - For `stateful` allocators it only stores a pointer to an allocator.
// - For `stateless` allocators it does not store anything.
// - For `shared` allocators it will store the allocator type directly.
// NOTE:
//  It does not take ownership over the allocator in the stateful case, the user has to ensure
//  that the allocator object stays valid. In the other cases the lifetime does not matter.
template <raw_allocator RawAllocator>
class [[nodiscard]] reference_storage : reference_storage_base<RawAllocator> {
    using base = reference_storage_base<RawAllocator>;

public:
    using allocator_type = typename allocator_traits<RawAllocator>::allocator_type;

    constexpr reference_storage() noexcept = default;

    constexpr explicit(false) reference_storage(allocator_type& allocator) noexcept
            : base{allocator} {}

    constexpr explicit(false) reference_storage(allocator_type const& allocator) noexcept
            : base{allocator} {}

    constexpr reference_storage(reference_storage const&) noexcept = default;

    constexpr reference_storage& operator=(reference_storage const&) noexcept = default;

    constexpr explicit operator bool() const noexcept {
        return base::is_valid();
    }

    constexpr allocator_type& allocator() const noexcept {
        return base::allocator();
    }

protected:
    constexpr ~reference_storage() = default;

    constexpr bool is_composable() const noexcept {
        return is_composable_allocator<allocator_type>;
    }
};

// Specialization of the class template `reference_storage` that is type-erased.
// It can store a reference to _any_ allocator type.
template <>
class [[nodiscard]] reference_storage<any_allocator> {
    // clang-format off
    struct [[nodiscard]] allocator_concept {
        using size_type       = std::size_t;
        using difference_type = std::ptrdiff_t;
        using stateful        = meta::true_type;

        virtual constexpr void clone(void* const) const noexcept = 0;

        virtual constexpr void*     allocate_node(size_type, size_type) noexcept = 0;
        virtual constexpr void* try_allocate_node(size_type, size_type) noexcept = 0;

        virtual constexpr void*     allocate_array(size_type, size_type, size_type) noexcept = 0;
        virtual constexpr void* try_allocate_array(size_type, size_type, size_type) noexcept = 0;

        virtual constexpr void     deallocate_node(void*, size_type, size_type) noexcept = 0;
        virtual constexpr bool try_deallocate_node(void*, size_type, size_type) noexcept = 0;

        virtual constexpr void     deallocate_array(void*, size_type, size_type, size_type) noexcept = 0;
        virtual constexpr bool try_deallocate_array(void*, size_type, size_type, size_type) noexcept = 0;

        virtual constexpr bool is_composable() const noexcept = 0;

        virtual constexpr ~allocator_concept() = default;

        constexpr size_type max_node_size() const noexcept {
            return max_size(query_type::node);
        }

        constexpr size_type max_array_size() const noexcept {
            return max_size(query_type::array);
        }

        constexpr size_type max_alignment() const noexcept {
            return max_size(query_type::alignment);
        }

    protected:
        enum class query_type : std::uint8_t { node, array, alignment };

        virtual constexpr size_type max_size(query_type) const noexcept = 0;
    };
    // clang-format on

    // clang-format off
    template <raw_allocator RawAllocator>
    struct [[nodiscard]] wrapper final
            : public  allocator_concept,
              private reference_storage_base<RawAllocator>
    // clang-format on
    {
        using allocator_traits  = allocator_traits<RawAllocator>;
        using composable_traits = composable_traits<RawAllocator>;
        using allocator_type    = typename allocator_traits::allocator_type;
        using storage           = reference_storage_base<RawAllocator>;

        constexpr explicit wrapper(RawAllocator& allocator) noexcept : storage{allocator} {}

        constexpr explicit wrapper(RawAllocator const& allocator) noexcept : storage{allocator} {}

        constexpr void clone(void* storage) const noexcept override {
            fdn::construct_at(static_cast<wrapper*>(storage), allocator());
        }

        constexpr void* allocate_node(size_type size, size_type alignment) noexcept override {
            return allocator_traits::allocate_node(allocator(), size, alignment);
        }

        constexpr void* allocate_array(size_type count, size_type size,
                                       size_type alignment) noexcept override {
            return allocator_traits::allocate_array(allocator(), count, size, alignment);
        }

        constexpr void deallocate_node(void* node, size_type size,
                                       size_type alignment) noexcept override {
            allocator_traits::deallocate_node(allocator(), node, size, alignment);
        }

        constexpr void deallocate_array(void* array, size_type count, size_type size,
                                        size_type alignment) noexcept override {
            allocator_traits::deallocate_array(allocator(), array, count, size, alignment);
        }

        constexpr void* try_allocate_node(size_type size, size_type alignment) noexcept override {
            if constexpr (is_composable_allocator<allocator_type>)
                return composable_traits::try_allocate_node(allocator(), size, alignment);
            else
                return nullptr;
        }

        constexpr void* try_allocate_array(size_type count, size_type size,
                                           size_type alignment) noexcept override {
            if constexpr (is_composable_allocator<allocator_type>)
                return composable_traits::try_allocate_array(allocator(), count, size, alignment);
            else
                return nullptr;
        }

        constexpr bool try_deallocate_node(void* node, size_type size,
                                           size_type alignment) noexcept override {
            if constexpr (is_composable_allocator<allocator_type>)
                return composable_traits::try_deallocate_node(allocator(), node, size, alignment);
            else
                return false;
        }

        constexpr bool try_deallocate_array(void* array, size_type count, size_type size,
                                            size_type alignment) noexcept override {
            if constexpr (is_composable_allocator<allocator_type>)
                return composable_traits::try_deallocate_array(allocator(), array, count, size,
                                                               alignment);
            else
                return false;
        }

        constexpr bool is_composable() const noexcept override {
            return is_composable_allocator<allocator_type>;
        }

        constexpr size_type max_size(query_type type) const noexcept override {
            auto&& alloc = allocator();
            if (query_type::node == type)
                return allocator_traits::max_node_size(alloc);
            else if (query_type::array == type)
                return allocator_traits::max_array_size(alloc);
            return allocator_traits::max_alignment(alloc);
        }

        constexpr allocator_type& allocator() const noexcept {
            return storage::allocator();
        }
    };

public:
    using allocator_type  = allocator_concept;
    using size_type       = typename allocator_type::size_type;
    using difference_type = typename allocator_type::difference_type;

    // clang-format off
    template <raw_allocator RawAllocator>
        requires(not meta::derived_from<meta::decay_t<RawAllocator>, reference_storage>)
    constexpr explicit(false) reference_storage(RawAllocator&& allocator) noexcept {
        static_assert(sizeof(wrapper<meta::remove_cvref_t<RawAllocator>>) <=
                              sizeof(wrapper<default_instantiation>),
                      "requires all instantiations to have certain maximum size");
        fdn::construct_at(
                reinterpret_cast<wrapper<meta::remove_cvref_t<RawAllocator>>*>(storage_),
                            std::forward<meta::remove_cvref_t<RawAllocator>>  (allocator));
    }

    template <raw_allocator RawAllocator>
        requires thread_safe_allocator<RawAllocator>
    constexpr explicit(false) reference_storage(RawAllocator const& allocator) noexcept {
        static_assert(sizeof(wrapper<meta::remove_cvref_t<RawAllocator>>) <=
                              sizeof(wrapper<default_instantiation>),
                      "requires all instantiations to have certain maximum size");
        fdn::construct_at(reinterpret_cast<wrapper<RawAllocator>*>(storage_), allocator);
    }
    // clang-format on

    constexpr explicit(false) reference_storage(allocator_concept& allocator) noexcept
            : reference_storage{static_cast<allocator_concept const&>(allocator)} {}

    constexpr explicit(false) reference_storage(allocator_concept const& allocator) noexcept {
        allocator.clone(&storage_);
    }

    constexpr ~reference_storage() {
        allocator().~allocator_type();
    }

    constexpr reference_storage(reference_storage const& other) noexcept {
        other.allocator().clone(&storage_);
    }

    constexpr reference_storage& operator=(reference_storage const& other) noexcept {
        allocator().~allocator_type();
        other.allocator().clone(&storage_);
        return *this;
    }

    constexpr bool is_composable() const noexcept {
        return allocator().is_composable();
    }

    constexpr explicit operator bool() const noexcept {
        return true;
    }

    constexpr allocator_type& allocator() const noexcept {
        auto memory = static_cast<void*>(&storage_);
        return *static_cast<allocator_type*>(memory);
    }

private:
    using default_instantiation = wrapper<allocator_concept>;
    alignas(default_instantiation) mutable std::byte storage_[sizeof(default_instantiation)];
};

template <raw_allocator RawAllocator>
using allocator_reference     = allocator_storage<reference_storage<RawAllocator>, no_mutex>;
using any_reference_storage   = reference_storage<any_allocator>;
using any_allocator_reference = allocator_storage<any_reference_storage, no_mutex>;

} // namespace salt::fdn