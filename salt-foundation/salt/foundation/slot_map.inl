SALT_DISABLE_WARNING_PUSH
SALT_DISABLE_WARNING_MICROSOFT_TEMPLATE

// clang-format off
#define SALT_SLOT_MAP_TEMPLATE                                                                     \
    template <typename T,                                                                          \
              std::unsigned_integral KeyType,                                                      \
              template <typename...> typename ValueContainer,                                      \
              template <typename...> typename KeyContainer>                                        \
    requires slot_map_requires<T, KeyType, ValueContainer, KeyContainer>
// clang-format on
#define SALT_SLOT_MAP Slot_map<T, KeyType, ValueContainer, KeyContainer>

#ifdef CLANG_ERROR_OUT_OF_LINE_DEFINITION
SALT_SLOT_MAP_TEMPLATE
constexpr void SALT_SLOT_MAP::reserve_impl(size_type size) {
    values_.reserve(size);
    indices_.reserve(size);
    keys_.reserve(size);
}

SALT_SLOT_MAP_TEMPLATE
constexpr auto SALT_SLOT_MAP::capacity_impl() const noexcept -> size_type {
    auto capacity = max_size();

    if (auto value_capacity = values_.capacity(); value_capacity < capacity) {
        capacity = value_capacity;
    }
    if (auto idx_capacity = indices_.capacity(); idx_capacity < capacity) {
        capacity = idx_capacity;
    }
    if (auto key_capacity = keys_.capacity(); key_capacity < capacity) {
        capacity = key_capacity;
    }

    return capacity;
}

SALT_SLOT_MAP_TEMPLATE
constexpr void SALT_SLOT_MAP::shrink_to_fit_impl() {
    values_.shrink_to_fit();
    indices_.shrink_to_fit();
    keys_.shrink_to_fit();
}
#else

SALT_SLOT_MAP_TEMPLATE
constexpr void SALT_SLOT_MAP::reserve(size_type size) REQUIRES_HAS_RESERVE(SALT_SLOT_MAP) {
    values_.reserve(size);
    indices_.reserve(size);
    keys_.reserve(size);
}

SALT_SLOT_MAP_TEMPLATE
constexpr auto SALT_SLOT_MAP::capacity() const noexcept -> size_type
        REQUIRES_HAS_CAPACITY(SALT_SLOT_MAP) {
    auto capacity = max_size();

    if (auto value_capacity = values_.capacity(); value_capacity < capacity) {
        capacity = value_capacity;
    }
    if (auto idx_capacity = indices_.capacity(); idx_capacity < capacity) {
        capacity = idx_capacity;
    }
    if (auto key_capacity = keys_.capacity(); key_capacity < capacity) {
        capacity = key_capacity;
    }

    return capacity;
}

SALT_SLOT_MAP_TEMPLATE
constexpr void SALT_SLOT_MAP::shrink_to_fit() REQUIRES_HAS_SHRINK_TO_FIT(SALT_SLOT_MAP) {
    values_.shrink_to_fit();
    indices_.shrink_to_fit();
    keys_.shrink_to_fit();
}
#endif

SALT_SLOT_MAP_TEMPLATE
constexpr void SALT_SLOT_MAP::clear() noexcept {
    values_.clear();
    // Push all objects into free indices list
    for (auto key : keys_) {
        indices_[key.idx] = std::exchange(free_idx_, key.idx);
    }
    keys_.clear();
}

// clang-format off
SALT_SLOT_MAP_TEMPLATE
template <typename... Args> requires std::constructible_from<T, Args&&...>
constexpr auto SALT_SLOT_MAP::emplace(Args&&... args) -> emplace_result {
    index_type value_idx = values_.size();

    if (free_idx_ == free_idx_null) {
        index_type free_idx_next = indices_.size();
        indices_.emplace_back()  = std::exchange(free_idx_, free_idx_next);
    }

    keys_.resize(value_idx + 1);

    auto free_idx      = free_idx_;
    auto free_idx_next = indices_[free_idx];

    auto& ref = values_.emplace_back(std::forward<Args>(args)...);
    auto  key = keys_.back() = {.idx = free_idx};
    indices_[free_idx]       = value_idx;
    free_idx_                = free_idx_next;

    return {
            .key = key,
            .ref = ref,
    };
}
// clang-format on

SALT_SLOT_MAP_TEMPLATE
constexpr auto SALT_SLOT_MAP::erase(iterator it) noexcept -> iterator {
    auto value_idx = cxx20::ranges::distance(begin(), it);
    erase_impl(static_cast<index_type>(value_idx));
    return std::ranges::next(begin(), value_idx);
}

SALT_SLOT_MAP_TEMPLATE
constexpr auto SALT_SLOT_MAP::pop(key_type key) noexcept -> value_type {
    auto value_idx = index(key);
    auto temp      = std::exchange(values_[value_idx], values_.back());

    values_.pop_back();
    erase_index_and_key(value_idx);
    return temp;
}

SALT_SLOT_MAP_TEMPLATE
constexpr void SALT_SLOT_MAP::erase_impl(index_type value_idx) noexcept {
    std::ranges::swap(values_[value_idx], values_.back());
    values_.pop_back();
    erase_index_and_key(value_idx);
}

SALT_SLOT_MAP_TEMPLATE
constexpr void SALT_SLOT_MAP::erase_index_and_key(index_type value_idx) noexcept {
    auto back_key  = keys_.back();
    auto erase_key = std::exchange(keys_[value_idx], back_key);
    keys_.pop_back();

    indices_[back_key.idx]  = value_idx;
    indices_[erase_key.idx] = std::exchange(free_idx_, erase_key.idx);
}

SALT_SLOT_MAP_TEMPLATE
constexpr void SALT_SLOT_MAP::swap(SALT_SLOT_MAP& other) noexcept {
    std::ranges::swap(values_, other.values_);
    std::ranges::swap(indices_, other.indices_);
    std::ranges::swap(keys_, other.keys_);
    std::ranges::swap(free_idx_, other.free_idx_);
}

#define SALT_SLOT_MAP_FIND(key)                                                                    \
    if (key.idx < indices_.size()) {                                                               \
        auto value_idx = indices_[key.idx];                                                        \
        auto it        = std::ranges::next(begin(), difference_type(value_idx), end());            \
        if (it < end() and it->first == key) {                                                     \
            return it;                                                                             \
        }                                                                                          \
    }                                                                                              \
    return end();

SALT_SLOT_MAP_TEMPLATE
constexpr auto SALT_SLOT_MAP::find(key_type key) const noexcept -> const_iterator {
    SALT_SLOT_MAP_FIND(key);
}

SALT_SLOT_MAP_TEMPLATE
constexpr auto SALT_SLOT_MAP::find(key_type key) noexcept -> iterator {
    SALT_SLOT_MAP_FIND(key);
}

#undef SALT_SLOT_MAP_FIND

SALT_SLOT_MAP_TEMPLATE
constexpr bool SALT_SLOT_MAP::operator==(Slot_map const& other) const noexcept {
    return cxx20::ranges::is_permutation(*this, other);
}

SALT_SLOT_MAP_TEMPLATE
constexpr void swap(SALT_SLOT_MAP& l, SALT_SLOT_MAP& r) noexcept {
    l.swap(r);
}

SALT_DISABLE_WARNING_POP

#undef SALT_SLOT_MAP_TEMPLATE
#undef SALT_SLOT_MAP