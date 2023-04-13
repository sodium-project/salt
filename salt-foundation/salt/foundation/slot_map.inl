SALT_DISABLE_WARNING_PUSH
SALT_DISABLE_WARNING_MICROSOFT_TEMPLATE

// clang-format off
#define SLOT_MAP_TEMPLATE                                                                     \
    template <typename T,                                                                          \
              std::unsigned_integral KeyType,                                                      \
              template <typename...> typename ValueContainer,                                      \
              template <typename...> typename KeyContainer>                                        \
    requires slottable<T, KeyType, ValueContainer, KeyContainer>
// clang-format on
#define SLOT_MAP Slot_map<T, KeyType, ValueContainer, KeyContainer>

SLOT_MAP_TEMPLATE
constexpr void SLOT_MAP::reserve(size_type size)
    requires detail::has_reserve<Slot_map>
{
    values_.reserve(size);
    indices_.reserve(size);
    keys_.reserve(size);
}

SLOT_MAP_TEMPLATE
constexpr auto SLOT_MAP::capacity() const noexcept -> size_type
    requires detail::has_capacity<Slot_map>
{
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

SLOT_MAP_TEMPLATE
constexpr void SLOT_MAP::shrink_to_fit()
    requires detail::has_shrink_to_fit<Slot_map>
{
    values_.shrink_to_fit();
    indices_.shrink_to_fit();
    keys_.shrink_to_fit();
}

SLOT_MAP_TEMPLATE
constexpr auto SLOT_MAP::begin() noexcept -> iterator {
    return {keys().begin(), values().begin()};
}

SLOT_MAP_TEMPLATE
constexpr auto SLOT_MAP::end() noexcept -> iterator {
    return {keys().end(), values().end()};
}

SLOT_MAP_TEMPLATE
constexpr auto SLOT_MAP::begin() const noexcept -> const_iterator {
    return {keys().begin(), values().begin()};
}

SLOT_MAP_TEMPLATE
constexpr auto SLOT_MAP::end() const noexcept -> const_iterator {
    return {keys().end(), values().end()};
}

SLOT_MAP_TEMPLATE
constexpr auto SLOT_MAP::cbegin() const noexcept -> const_iterator {
    return begin();
}

SLOT_MAP_TEMPLATE
constexpr auto SLOT_MAP::cend() const noexcept -> const_iterator {
    return end();
}

SLOT_MAP_TEMPLATE
constexpr bool SLOT_MAP::empty() const noexcept {
    return begin() == end();
}

SLOT_MAP_TEMPLATE
constexpr auto SLOT_MAP::size() const noexcept -> size_type {
    return static_cast<size_type>(ranges::distance(begin(), end()));
}

SLOT_MAP_TEMPLATE
constexpr void SLOT_MAP::clear() noexcept {
    values_.clear();
    // Push all objects into free indices list
    for (auto key : keys_) {
        indices_[key.idx] = std::exchange(free_idx_, key.idx);
    }
    keys_.clear();
}

SLOT_MAP_TEMPLATE
constexpr auto SLOT_MAP::insert(value_type const& value) -> key_type
    requires std::copy_constructible<value_type>
{
    return emplace(value).key;
}

SLOT_MAP_TEMPLATE
constexpr auto SLOT_MAP::insert(value_type&& value) -> key_type
    requires std::move_constructible<value_type>
{
    return emplace(std::move(value)).key;
}

// clang-format off
SLOT_MAP_TEMPLATE
template <typename... Args> requires std::constructible_from<T, Args&&...>
constexpr auto SLOT_MAP::emplace(Args&&... args) -> emplace_result {
    index_type value_idx = static_cast<index_type>(values_.size());

    if (free_idx_ == free_idx_null) {
        index_type free_idx_next = static_cast<index_type>(indices_.size());
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

SLOT_MAP_TEMPLATE
constexpr auto SLOT_MAP::erase(iterator it) noexcept -> iterator {
    auto value_idx = ranges::distance(begin(), it);
    erase_impl(static_cast<index_type>(value_idx));
    return std::ranges::next(begin(), value_idx);
}

SLOT_MAP_TEMPLATE
constexpr void SLOT_MAP::erase(key_type key) noexcept {
    erase_impl(index(key));
}

SLOT_MAP_TEMPLATE
constexpr auto SLOT_MAP::access(key_type key) noexcept -> iterator {
    return std::ranges::next(begin(), index(key));
};

SLOT_MAP_TEMPLATE
constexpr auto SLOT_MAP::access(key_type key) const noexcept -> const_iterator {
    return std::ranges::next(begin(), index(key));
}

SLOT_MAP_TEMPLATE
constexpr auto SLOT_MAP::pop(key_type key) noexcept -> value_type {
    auto value_idx = index(key);
    auto temp      = std::exchange(values_[value_idx], values_.back());

    values_.pop_back();
    erase_index_and_key(value_idx);
    return temp;
}

SLOT_MAP_TEMPLATE
constexpr T& SLOT_MAP::operator[](key_type key) noexcept {
    return values_[index(key)];
}

SLOT_MAP_TEMPLATE
constexpr T const& SLOT_MAP::operator[](key_type key) const noexcept {
    return values_[index(key)];
}

SLOT_MAP_TEMPLATE
constexpr auto SLOT_MAP::data() noexcept -> pointer
    requires detail::has_data<Slot_map>
{
    return values_.data();
}

SLOT_MAP_TEMPLATE
constexpr auto SLOT_MAP::data() const noexcept -> const_pointer
    requires detail::has_data<Slot_map>
{
    return values_.data();
}

SLOT_MAP_TEMPLATE
constexpr bool SLOT_MAP::contains(key_type key) const noexcept {
    return find(key) != end();
};

SLOT_MAP_TEMPLATE
constexpr void SLOT_MAP::erase_impl(index_type value_idx) noexcept {
    std::ranges::swap(values_[value_idx], values_.back());
    values_.pop_back();
    erase_index_and_key(value_idx);
}

SLOT_MAP_TEMPLATE
constexpr void SLOT_MAP::erase_index_and_key(index_type value_idx) noexcept {
    auto back_key  = keys_.back();
    auto erase_key = std::exchange(keys_[value_idx], back_key);
    keys_.pop_back();

    indices_[back_key.idx]  = value_idx;
    indices_[erase_key.idx] = std::exchange(free_idx_, erase_key.idx);
}

SLOT_MAP_TEMPLATE
constexpr void SLOT_MAP::swap(SLOT_MAP& other) noexcept {
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

SLOT_MAP_TEMPLATE
constexpr auto SLOT_MAP::find(key_type key) const noexcept -> const_iterator {
    SALT_SLOT_MAP_FIND(key);
}

SLOT_MAP_TEMPLATE
constexpr auto SLOT_MAP::find(key_type key) noexcept -> iterator {
    SALT_SLOT_MAP_FIND(key);
}

#undef SALT_SLOT_MAP_FIND

SLOT_MAP_TEMPLATE
constexpr bool SLOT_MAP::operator==(Slot_map const& other) const noexcept {
#ifdef SALT_LIBCPP_HAS_NO_RANGES
    return ranges::is_permutation(std::begin(*this), std::end(*this), std::begin(other),
                                  std::end(other));
#else
    return ranges::is_permutation(*this, other);
#endif
}

SLOT_MAP_TEMPLATE
constexpr void swap(SLOT_MAP& l, SLOT_MAP& r) noexcept {
    l.swap(r);
}

SALT_DISABLE_WARNING_POP

#undef SLOT_MAP_TEMPLATE
#undef SLOT_MAP