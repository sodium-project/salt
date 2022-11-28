#pragma once

#include <salt/meta.hpp>
#include <salt/memory/threading.hpp>

namespace salt {

// A RawAllocator that stores another allocator.
// NOTE:
// * StoragePolicy defines the allocator type being stored and how it is stored.
// * Mutex controls synchronization of the access.
template <typename StoragePolicy, typename Mutex>
class [[nodiscard]] Allocator_storage : StoragePolicy,
                                        mutex_storage_for<typename StoragePolicy::allocator_type, Mutex> {
    template <typename MutexStorage>
    using lock_guard        = lock_guard_for   <Mutex, MutexStorage>;
    using allocator_traits  = allocator_traits <typename StoragePolicy::allocator_type>;
    using composable_traits = composable_traits<typename StoragePolicy::allocator_type>;
    using mutex_storage     = mutex_storage_for<typename StoragePolicy::allocator_type, Mutex>;
    using storage_policy    = StoragePolicy;

public:
    using allocator_type  = typename StoragePolicy::allocator_type;
    using mutex_type      = mutex_storage const;
    using size_type       = std::size_t;
    using difference_type = std::ptrdiff_t;
    using is_stateful     = typename allocator_traits::is_stateful;

    Allocator_storage() = default;

    // clang-format off
    template <typename Allocator> requires(
        // use this to prevent constructor being chosen instead of move
        not base_of<Allocator_storage, std::decay_t<Allocator>>
        and only_constructible<StoragePolicy, Allocator>)
    constexpr Allocator_storage(Allocator&& allocator) noexcept
            : storage_policy{std::forward<Allocator>(allocator)} {}

    template <typename OtherPolicy> requires
        requires(Allocator_storage<OtherPolicy, Mutex> const& other) { new storage_policy{other.allocator()}; }
    constexpr Allocator_storage(Allocator_storage<OtherPolicy, Mutex> const& other) noexcept
            : storage_policy{other.allocator()} {}
    // clang-format on

    constexpr Allocator_storage(Allocator_storage&& other) noexcept
            : storage_policy{std::move(other)}, mutex_storage{std::move(other)} {}

    constexpr Allocator_storage& operator=(Allocator_storage&& other) noexcept {
        storage_policy::operator=(std::move(other));
        mutex_storage:: operator=(std::move(other));
        return *this;
    }

    Allocator_storage(Allocator_storage const&) = default;
    Allocator_storage& operator=(Allocator_storage const&) = default;

    constexpr void* allocate_node(size_type size, size_type alignment) {
        lock_guard<mutex_type> lock{*this};
        auto&&                 alloc = allocator();
        return allocator_traits::allocate_node(alloc, size, alignment);
    }

    constexpr void* allocate_array(size_type count, size_type size, size_type alignment) {
        lock_guard<mutex_type> lock{*this};
        auto&&                 alloc = allocator();
        return allocator_traits::allocate_array(alloc, count, size, alignment);
    }

    constexpr void deallocate_node(void* ptr, size_type size, size_type alignment) noexcept {
        lock_guard<mutex_type> lock{*this};
        auto&&                 alloc = allocator();
        allocator_traits::deallocate_node(alloc, ptr, size, alignment);
    }

    constexpr void deallocate_array(void* ptr, size_type count, size_type size,
                                    size_type alignment) noexcept {
        lock_guard<mutex_type> lock{*this};
        auto&&                 alloc = allocator();
        allocator_traits::deallocate_array(alloc, ptr, count, size, alignment);
    }

    constexpr size_type max_node_size() const noexcept {
        lock_guard<mutex_type> lock{*this};
        auto&&                 alloc = allocator();
        return allocator_traits::max_node_size(alloc);
    }

    constexpr size_type max_array_size() const noexcept {
        lock_guard<mutex_type> lock{*this};
        auto&&                 alloc = allocator();
        return allocator_traits::max_array_size(alloc);
    }

    constexpr size_type max_alignment() const noexcept {
        lock_guard<mutex_type> lock{*this};
        auto&&                 alloc = allocator();
        return allocator_traits::max_alignment(alloc);
    }

    constexpr auto try_allocate_node(size_type size, size_type alignment) noexcept
            requires is_composable_allocator<allocator_type> {
        lock_guard<mutex_type> lock{*this};
        auto&&                 alloc = allocator();
        return composable_traits::try_allocate_node(alloc, size, alignment);
    }

    constexpr auto try_allocate_array(size_type count, size_type size, size_type alignment) noexcept
            requires is_composable_allocator<allocator_type> {
        lock_guard<mutex_type> lock{*this};
        auto&&                 alloc = allocator();
        return composable_traits::try_allocate_array(alloc, count, size, alignment);
    }

    constexpr auto try_deallocate_node(void* ptr, size_type size, size_type alignment) noexcept
            requires is_composable_allocator<allocator_type> {
        lock_guard<mutex_type> lock{*this};
        auto&&                 alloc = allocator();
        return composable_traits::try_deallocate_node(alloc, ptr, size, alignment);
    }

    constexpr auto try_deallocate_array(void* ptr, size_type count, size_type size,
                                        size_type alignment) noexcept
            requires is_composable_allocator<allocator_type> {
        lock_guard<mutex_type> lock{*this};
        auto&&                 alloc = allocator();
        return composable_traits::try_deallocate_array(alloc, ptr, count, size, alignment);
    }

    constexpr decltype(auto) allocator() noexcept {
        return storage_policy::allocator();
    }
    constexpr decltype(auto) allocator() const noexcept {
        return storage_policy::allocator();
    }

    constexpr decltype(auto) lock() noexcept {
        return detail::lock_allocator(allocator(), static_cast<mutex_type&>(*this));
    }
    constexpr decltype(auto) lock() const noexcept {
        return detail::lock_allocator(allocator(), static_cast<mutex_type&>(*this));
    }

    bool is_composable() const noexcept {
        return storage_policy::is_composable();
    }
};

// Tag type that enables type-erasure in Reference_storage.
// It can be used anywhere the Allocator_reference is used internally.
struct [[nodiscard]] Any_allocator final {};
using any_allocator = Any_allocator;

// A StoragePolicy that stores the allocator directly.
// It embeds the allocator inside it, i.e. moving the storage policy will move the allocator.
// clang-format off
template <raw_allocator RawAllocator> requires(not std::same_as<RawAllocator, Any_allocator>)
struct [[nodiscard]] Direct_storage : allocator_traits<RawAllocator>::allocator_type {
    using allocator_type = typename allocator_traits<RawAllocator>::allocator_type;

    Direct_storage() = default;

    constexpr Direct_storage(allocator_type&& allocator) noexcept
            : allocator_type(std::move(allocator)) {}

    constexpr Direct_storage(Direct_storage&& other) noexcept : allocator_type(std::move(other)) {}

    constexpr Direct_storage& operator=(Direct_storage&& other) noexcept {
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
    ~Direct_storage() noexcept = default;

    constexpr bool is_composable() const noexcept {
        return is_composable_allocator<allocator_type>;
    }
};
// clang-format on

template <raw_allocator RawAllocator>
using Allocator_adapter = Allocator_storage<Direct_storage<RawAllocator>, No_mutex>;

template <raw_allocator RawAllocator, typename Mutex = std::mutex>
using Thread_safe_allocator = Allocator_storage<Direct_storage<RawAllocator>, Mutex>;

// TODO:
//  Try to rewrite it using the concept.
// clang-format off
template <raw_allocator RawAllocator>
struct [[nodiscard]] is_shared_allocator;
// clang-format on

namespace detail {

struct [[nodiscard]] Reference_stateful  final {};
struct [[nodiscard]] Reference_stateless final {};
struct [[nodiscard]] Reference_shared    final {};

// clang-format off
Reference_stateful  reference_type(std::true_type  stateful, std::false_type shared);
Reference_stateless reference_type(std::false_type stateful, std::true_type  shared);
Reference_stateless reference_type(std::false_type stateful, std::false_type shared);
Reference_shared    reference_type(std::true_type  stateful, std::true_type  shared);
// clang-format on

template <raw_allocator RawAllocator> struct [[nodiscard]] allocator_reference final {
    using is_stateful = typename allocator_traits<RawAllocator>::is_stateful;
    using is_shared   = is_shared_allocator<RawAllocator>;
    using type        = decltype(reference_type(is_stateful{}, is_shared{}));
};
template <raw_allocator RawAllocator>
using allocator_reference_t = typename allocator_reference<RawAllocator>::type;

// clang-format off
template <raw_allocator RawAllocator, typename Tag>
struct [[nodiscard]] Reference_storage_impl;
// clang-format on

// Stores a pointer to an allocator
template <raw_allocator RawAllocator>
struct [[nodiscard]] Reference_storage_impl<RawAllocator, Reference_stateful> {
protected:
    constexpr Reference_storage_impl() noexcept : allocator_{nullptr} {}

    constexpr explicit Reference_storage_impl(RawAllocator& allocator) noexcept
            : allocator_{&allocator} {}

    constexpr bool is_valid() const noexcept {
        return allocator_ != nullptr;
    }

    constexpr RawAllocator& allocator() const noexcept {
        SALT_ASSERT(allocator_ != nullptr);
        return *allocator_;
    }

private:
    RawAllocator* allocator_;
};

// Stores RawAllocator in static storage
template <raw_allocator RawAllocator>
struct [[nodiscard]] Reference_storage_impl<RawAllocator, Reference_stateless> {
protected:
    constexpr Reference_storage_impl() noexcept = default;

    constexpr explicit Reference_storage_impl(RawAllocator const&) noexcept {}

    constexpr bool is_valid() const noexcept {
        return true;
    }

    /* constexpr */ RawAllocator& allocator() const noexcept {
        static RawAllocator alloc;
        return alloc;
    }
};

// Stores RawAllocator directly
template <raw_allocator RawAllocator>
struct [[nodiscard]] Reference_storage_impl<RawAllocator, Reference_shared> {
protected:
    constexpr Reference_storage_impl() noexcept = default;

    constexpr explicit Reference_storage_impl(RawAllocator const& allocator) noexcept
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

// Specifies whether or not a RawAllocator has shared semantics. It is shared, if
// multiple objects refer to the same internal allocator and if it can be copied.
// TODO:
//  Try to use the concepts
// clang-format off
template <raw_allocator RawAllocator>
struct [[nodiscard]] is_shared_allocator : std::false_type {};
// clang-format on

template <typename RawAllocator>
using Reference_storage_impl =
        detail::Reference_storage_impl<typename allocator_traits<RawAllocator>::allocator_type,
                                       detail::allocator_reference_t<RawAllocator>>;

// A StoragePolicy that stores a reference to an allocator.
// For stateful allocators it only stores a pointer to an allocator.
// For stateless allocators it does not store anything.
// For allocators that are already shared it will store the allocator type directly.
// NOTE:
//  It does not take ownership over the allocator in the stateful case, the user has to ensure
//  that the allocator object stays valid. In the other cases the lifetime does not matter.
template <raw_allocator RawAllocator>
class [[nodiscard]] Reference_storage : Reference_storage_impl<RawAllocator> {
    using storage = Reference_storage_impl<RawAllocator>;

public:
    using allocator_type = typename allocator_traits<RawAllocator>::allocator_type;

    constexpr Reference_storage() noexcept = default;

    constexpr explicit Reference_storage(allocator_type& allocator) noexcept : storage{allocator} {}

    constexpr explicit Reference_storage(allocator_type const& allocator) noexcept
            : storage{allocator} {}

    constexpr Reference_storage(Reference_storage const&) noexcept = default;

    constexpr Reference_storage& operator=(Reference_storage const&) noexcept = default;

    constexpr explicit operator bool() const noexcept {
        return storage::is_valid();
    }

    constexpr allocator_type& allocator() const noexcept {
        return storage::allocator();
    }

protected:
    constexpr ~Reference_storage() = default;

    constexpr bool is_composable() const noexcept {
        return is_composable_allocator<allocator_type>;
    }
};

// Specialization of the class template Reference_storage that is type-erased. It is triggered by
// the tag type Any_allocator. The specialization can store a reference to any allocator type.
#if 0
template <> class [[nodiscard]] Reference_storage<Any_allocator> {
    struct [[nodiscard]] Base_allocator {
        using size_type       = std::size_t;
        using difference_type = std::ptrdiff_t;
        using is_stateful     = std::true_type;

        virtual constexpr void* allocate(size_type count, size_type size,
                                         size_type alignment) noexcept       = 0;
        virtual constexpr void  deallocate(void* ptr, size_type count, size_type size,
                                           size_type alignment) noexcept     = 0;
        virtual constexpr void* try_allocate(size_type count, size_type size,
                                             size_type alignment) noexcept   = 0;
        virtual constexpr bool  try_deallocate(void* ptr, size_type count, size_type size,
                                               size_type alignment) noexcept = 0;

        virtual constexpr bool is_composable() const noexcept      = 0;
        virtual constexpr void clone(void* storage) const noexcept = 0;

        virtual constexpr ~Base_allocator() = default;

        constexpr void* allocate_node(size_type size, size_type alignment) {
            return allocate(1, size, alignment);
        }

        constexpr void* allocate_array(size_type count, size_type size, size_type alignment) {
            return allocate(count, size, alignment);
        }

        constexpr void deallocate_node(void* node, size_type size, size_type alignment) noexcept {
            deallocate(node, 1, size, alignment);
        }

        constexpr void deallocate_array(void* array, size_type count, size_type size,
                                        size_type alignment) noexcept {
            deallocate(array, count, size, alignment);
        }

        constexpr void* try_allocate_node(size_type size, size_type alignment) noexcept {
            return try_allocate(1, size, alignment);
        }

        constexpr void* try_allocate_array(size_type count, size_type size,
                                           size_type alignment) noexcept {
            return try_allocate(count, size, alignment);
        }

        constexpr bool try_deallocate_node(void* node, size_type size,
                                           size_type alignment) noexcept {
            return try_deallocate(node, 1, size, alignment);
        }

        constexpr bool try_deallocate_array(void* array, size_type count, size_type size,
                                            size_type alignment) noexcept {
            return try_deallocate(array, count, size, alignment);
        }

    protected:
        enum class allocation_type : std::uint8_t { node_size, array_size, alignment };

        virtual constexpr size_type max_size(allocation_type) const noexcept = 0;
    };

    // clang-format off
    template <raw_allocator RawAllocator>
    class [[nodiscard]] Wrap_allocator final
            : public  Base_allocator,
              private detail::Reference_storage_impl<
                        typename allocator_traits<RawAllocator>::allocator_type,
                        detail::allocator_reference_t<RawAllocator>
                      >
    // clang-format on
    {
        using allocator_traits    = allocator_traits<RawAllocator>;
        using composable_traits   = composable_traits<RawAllocator>;
        using allocator_type      = typename allocator_traits::allocator_type;
        using allocator_reference = detail::allocator_reference_t<RawAllocator>;
        using reference_storage   =
                detail::Reference_storage_impl<allocator_type, allocator_reference>;

    public:
        constexpr Wrap_allocator(RawAllocator& allocator) noexcept : reference_storage{allocator} {}

        constexpr Wrap_allocator(RawAllocator const& allocator) noexcept
                : reference_storage{allocator} {}

        constexpr allocator_type& self() const noexcept {
            return reference_storage::allocator();
        }

        constexpr void clone(void* storage) const noexcept override {
            ::new (storage) Wrap_allocator(self());
        }

        constexpr void* allocate(size_type count, size_type size,
                                 size_type alignment) noexcept override {
            auto&& alloc = self();
            if (1u == count)
                return allocator_traits::allocate_node(alloc, size, alignment);
            else
                return allocator_traits::allocate_array(alloc, count, size, alignment);
        }

        constexpr void deallocate(void* ptr, size_type count, size_type size,
                                  size_type alignment) noexcept override {
            auto&& alloc = self();
            if (1u == count)
                allocator_traits::deallocate_node(alloc, ptr, size, alignment);
            else
                allocator_traits::deallocate_array(alloc, ptr, count, size, alignment);
        }

        constexpr void* try_allocate(size_type count, size_type size,
                                     size_type alignment) noexcept override {
            if constexpr (is_composable_allocator<allocator_type>) {
                auto&& alloc = self();
                if (1u == count)
                    return composable_traits::try_allocate_node(alloc, size, alignment);
                else
                    return composable_traits::try_allocate_array(alloc, count, size, alignment);
            } else {
                return nullptr;
            }
        }

        constexpr bool try_deallocate(void* ptr, size_type count, size_type size,
                                      size_type alignment) noexcept override {
            if constexpr (is_composable_allocator<allocator_type>) {
                auto&& alloc = self();
                if (1u == count)
                    return composable_traits::try_deallocate_node(alloc, ptr, size, alignment);
                else
                    return composable_traits::try_deallocate_array(alloc, ptr, count, size,
                                                                   alignment);
            } else {
                return false;
            }
        }

        constexpr bool is_composable() const noexcept override {
            return is_composable_allocator<allocator_type>;
        }

        constexpr size_type max_size(allocation_type type) const noexcept override {
            auto&& alloc = self();
            if (type == allocation_type::node_size)
                return allocator_traits::max_node_size(alloc);
            else if (type == allocation_type::array_size)
                return allocator_traits::max_array_size(alloc);
            return allocator_traits::max_alignment(alloc);
        }
    };

public:
    using allocator_type  = Base_allocator;
    using size_type       = typename allocator_type::size_type;
    using difference_type = typename allocator_type::difference_type;

    // clang-format off
    template <raw_allocator RawAllocator> requires(
        not std::derived_from<Reference_storage, std::decay_t<RawAllocator>>)
    constexpr Reference_storage(RawAllocator&& allocator) noexcept {
        static_assert(sizeof(Wrap_allocator<RawAllocator>) <=
                              sizeof(Wrap_allocator<default_instantiation>),
                      "requires all instantiations to have certain maximum size");
        ::new (static_cast<void*>(storage_)) Wrap_allocator<std::remove_cvref_t<RawAllocator>>{
                std::forward<std::remove_cvref_t<RawAllocator>>(allocator)};
    }

    template <raw_allocator RawAllocator> requires(
        not allocator_traits<RawAllocator>::is_stateful::value)
    constexpr Reference_storage(RawAllocator const& allocator) noexcept {
        static_assert(sizeof(Wrap_allocator<RawAllocator>) <=
                              sizeof(Wrap_allocator<default_instantiation>),
                      "requires all instantiations to have certain maximum size");
        ::new (static_cast<void*>(storage_)) Wrap_allocator<RawAllocator>{allocator};
    }
    // clang-format on

    constexpr Reference_storage(Base_allocator& allocator) noexcept
            : Reference_storage{static_cast<Base_allocator const&>(allocator)} {}

    constexpr Reference_storage(Base_allocator const& allocator) noexcept {
        allocator.clone(&storage_);
    }

    constexpr Reference_storage(Reference_storage const& other) noexcept {
        other.allocator().clone(&storage_);
    }

    constexpr ~Reference_storage() {
        allocator().~allocator_type();
    }

    constexpr Reference_storage& operator=(Reference_storage const& other) {
        allocator().~allocator_type();
        other.allocator().clone(&storage_);
        return *this;
    }

    constexpr bool is_composable() const noexcept {
        return allocator().is_composable();
    }

    constexpr allocator_type& allocator() const noexcept {
        auto memory = static_cast<void*>(&storage_);
        return *static_cast<allocator_type*>(memory);
    }

private:
    using default_instantiation = Wrap_allocator<Base_allocator>;
    alignas(default_instantiation) mutable std::byte storage_[sizeof(default_instantiation)];
};
#else
template <> class [[nodiscard]] Reference_storage<Any_allocator> {
    // clang-format off
    struct [[nodiscard]] Composable_allocator {
        using size_type       = std::size_t;
        using difference_type = std::ptrdiff_t;

        virtual constexpr bool is_composable() const noexcept = 0;

        virtual constexpr void* try_allocate_node (size_type, size_type)            noexcept = 0;
        virtual constexpr void* try_allocate_array(size_type, size_type, size_type) noexcept = 0;

        virtual constexpr bool try_deallocate_node (void*, size_type, size_type)            noexcept = 0;
        virtual constexpr bool try_deallocate_array(void*, size_type, size_type, size_type) noexcept = 0;

        virtual constexpr ~Composable_allocator() = default;
    };

    struct [[nodiscard]] Base_allocator : Composable_allocator {
        using size_type       = typename Composable_allocator::size_type;
        using difference_type = typename Composable_allocator::difference_type;
        using is_stateful     = std::true_type;

        virtual constexpr void clone(void* storage) const noexcept = 0;

        virtual constexpr void* allocate_node (size_type, size_type)            = 0;
        virtual constexpr void* allocate_array(size_type, size_type, size_type) = 0;

        virtual constexpr void deallocate_node (void*, size_type, size_type)            noexcept = 0;
        virtual constexpr void deallocate_array(void*, size_type, size_type, size_type) noexcept = 0;

        virtual constexpr ~Base_allocator() = default;

    protected:
        enum class allocation_type : std::uint8_t { node, array };

        virtual constexpr size_type max_size(allocation_type) const noexcept = 0;
    };
    // clang-format on

    // clang-format off
    template <raw_allocator RawAllocator>
    struct [[nodiscard]] Wrapper final
            : public  Base_allocator,
              private Reference_storage_impl<RawAllocator>
    // clang-format on
    {
        using allocator_traits  = allocator_traits<RawAllocator>;
        using composable_traits = composable_traits<RawAllocator>;
        using allocator_type    = typename allocator_traits::allocator_type;
        using storage           = Reference_storage_impl<RawAllocator>;

        constexpr explicit Wrapper(RawAllocator& allocator) noexcept : storage{allocator} {}

        constexpr explicit Wrapper(RawAllocator const& allocator) noexcept : storage{allocator} {}

        constexpr void clone(void* storage) const noexcept override {
            ::new (storage) Wrapper{allocator()};
        }

        constexpr void* allocate_node(size_type size, size_type alignment) override {
            return allocator_traits::allocate_node(allocator(), size, alignment);
        }

        constexpr void* allocate_array(size_type count, size_type size,
                                       size_type alignment) override {
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
            // clang-format off
            if constexpr (is_composable_allocator<allocator_type>)
                return composable_traits::try_deallocate_array(allocator(), array, count, size, alignment);
            else
                return false;
            // clang-format on
        }

        constexpr bool is_composable() const noexcept override {
            return is_composable_allocator<allocator_type>;
        }

        constexpr size_type max_size(allocation_type type) const noexcept override {
            auto&& alloc = allocator();
            if (type == allocation_type::node)
                return allocator_traits::max_node_size(alloc);
            else if (type == allocation_type::array)
                return allocator_traits::max_array_size(alloc);
            return allocator_traits::max_alignment(alloc);
        }

        constexpr allocator_type& allocator() const noexcept {
            return storage::allocator();
        }
    };

public:
    using allocator_type  = Base_allocator;
    using size_type       = typename allocator_type::size_type;
    using difference_type = typename allocator_type::difference_type;

    // clang-format off
    template <raw_allocator RawAllocator> requires(
        not std::derived_from<Reference_storage, std::decay_t<RawAllocator>>)
    constexpr Reference_storage(RawAllocator&& allocator) noexcept {
        static_assert(sizeof(Wrapper<RawAllocator>) <=
                              sizeof(Wrapper<Default_instantiation>),
                      "requires all instantiations to have certain maximum size");
        ::new (static_cast<void*>(storage_)) Wrapper<std::remove_cvref_t<RawAllocator>>{
                std::forward<std::remove_cvref_t<RawAllocator>>(allocator)};
    }

    template <raw_allocator RawAllocator> requires(
        not allocator_traits<RawAllocator>::is_stateful::value)
    constexpr Reference_storage(RawAllocator const& allocator) noexcept {
        static_assert(sizeof(Wrapper<RawAllocator>) <=
                              sizeof(Wrapper<Default_instantiation>),
                      "requires all instantiations to have certain maximum size");
        ::new (static_cast<void*>(storage_)) Wrapper<RawAllocator>{allocator};
    }
    // clang-format on

    constexpr explicit Reference_storage(Base_allocator& allocator) noexcept
            : Reference_storage{static_cast<Base_allocator const&>(allocator)} {}

    constexpr explicit Reference_storage(Base_allocator const& allocator) noexcept {
        allocator.clone(&storage_);
    }

    constexpr Reference_storage(Reference_storage const& other) noexcept {
        other.allocator().clone(&storage_);
    }

    constexpr ~Reference_storage() {
        allocator().~allocator_type();
    }

    constexpr Reference_storage& operator=(Reference_storage const& other) {
        allocator().~allocator_type();
        other.allocator().clone(&storage_);
        return *this;
    }

    constexpr bool is_composable() const noexcept {
        return allocator().is_composable();
    }

    constexpr allocator_type& allocator() const noexcept {
        auto memory = static_cast<void*>(&storage_);
        return *static_cast<allocator_type*>(memory);
    }

private:
    using Default_instantiation = Wrapper<Base_allocator>;
    alignas(Default_instantiation) mutable std::byte storage_[sizeof(Default_instantiation)];
};
#endif

template <raw_allocator RawAllocator>
using Allocator_reference = Allocator_storage<Reference_storage<RawAllocator>, No_mutex>;

using Any_reference_storage = Reference_storage<Any_allocator>;

using Any_allocator_reference = Allocator_storage<Any_reference_storage, No_mutex>;

} // namespace salt
