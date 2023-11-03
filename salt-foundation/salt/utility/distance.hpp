#pragma once
#include <salt/meta.hpp>

// TODO:
//  Add a more general implementation of distance.
namespace salt::utility {

template <meta::random_access_iterator Iterator>
constexpr std::size_t udistance(Iterator first, Iterator last) noexcept {
    return static_cast<std::size_t>(last - first);
}

template <typename Iterator>
constexpr std::size_t udistance(Iterator first, Iterator last) noexcept {
    std::size_t result = 0;
    while (first != last) {
        ++first;
        ++result;
    }
    return result;
}

} // namespace salt::fdn