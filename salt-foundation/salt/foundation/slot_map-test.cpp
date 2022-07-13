#include <catch2/catch.hpp>

#include <array>

#include <salt/foundation.hpp>

template <typename T> using Vector   = std::vector<T>;
template <typename T> using Slot_map = salt::Slot_map<T, std::size_t, Vector, Vector>;

namespace Catch {
template <> struct StringMaker<Slot_map<int>> {
    static std::string convert(Slot_map<int> const& map) {
        std::string result;
        for (auto [key, value] : map) {
            result += "{idx: ";
            result += std::to_string(key.idx);
            result += ", value: ";
            result += std::to_string(value);
            result += "}, ";
        }
        return result;
    }
};
} // namespace Catch

using Test_iterator = Slot_map<int>::iterator;
static_assert(std::input_iterator<Test_iterator>);
static_assert(std::forward_iterator<Test_iterator>);
static_assert(std::bidirectional_iterator<Test_iterator>);
static_assert(std::random_access_iterator<Test_iterator>);

TEST_CASE("salt::Slot_map", "[salt-utils/slot_map.hpp]") {
    SECTION("test if empty") {
        Slot_map<int> map;
        REQUIRE(map.empty());
    }

    SECTION("test if non-empty") {
        Slot_map<int>         map;
        [[maybe_unused]] auto k = map.insert(1);
        REQUIRE_FALSE(map.empty());
    }

    SECTION("test insert and delete") {
        Slot_map<int>                        map;
        std::vector<decltype(map)::key_type> keys;

        std::size_t i = 0;
        for (; i < 16; i++) {
            REQUIRE(map.size() == i);
            [[maybe_unused]] auto k = map.insert(2022);
            keys.push_back(k);
        }

        for (; i > 0; i--) {
            REQUIRE(map.size() == i);
            auto k = keys.back();
            keys.pop_back();
            auto it = map.find(k);
            REQUIRE(it != map.end());
            (void)map.erase(it);
        }
        REQUIRE(map.size() == i);
    }

    SECTION("test capacity when map is empty") {
        Slot_map<int> map;
        REQUIRE(map.capacity() == 0);
    }

    SECTION("test capacity after reserve") {
        Slot_map<int> map;
        map.reserve(10);
        REQUIRE(map.capacity() >= 10);
    }

    SECTION("test capacity after shrink_to_fit") {
        Slot_map<int> map;
        map.reserve(10);
        auto cnt = map.size();
        map.shrink_to_fit();
        REQUIRE(map.capacity() <= cnt);
    }

    SECTION("test clear") {
        Slot_map<int> map;
        for (size_t i = 0; i < 16; i++) {
            [[maybe_unused]] auto k = map.insert(int(i));
        }
        map.clear();
        REQUIRE(map.empty());
    }

    SECTION("test clear after insert and clear") {
        Slot_map<int> map;
        for (size_t i = 0; i < 3; i++) {
            [[maybe_unused]] auto k = map.insert(int(i));
        }
        map.clear();
        REQUIRE(map.empty());
        std::vector values = {0, 1, 2, 3, 4};
        for (auto v : values) {
            auto k  = map.insert(v);
            auto it = map.find(k);
            REQUIRE(it != map.end());
            REQUIRE(it->first == k);
            REQUIRE(it->second == v);
        }
    }

    SECTION("test clear after insert") {
        Slot_map<int> map;
        std::vector   values = {0, 1, 2, 3, 4};
        for (auto v : values) {
            auto k  = map.insert(v);
            auto it = map.find(k);
            REQUIRE(it != map.end());
            REQUIRE(it->first == k);
            REQUIRE(it->second == v);
        }
    }

    SECTION("test insert") {
        Slot_map<int>                        map;
        std::vector                          values = {0, 1, 2, 3, 4};
        std::vector<decltype(map)::key_type> keys;
        for (auto v : values) {
            keys.push_back(map.insert(v));
        }
        for (size_t i = 0; i < values.size(); i++) {
            auto k  = keys[i];
            auto v  = values[i];
            auto it = map.find(k);
            REQUIRE(it != map.end());
            REQUIRE(it->first == k);
            REQUIRE(it->second == v);
        }
    }

    SECTION("test insert after erase") {
        Slot_map<int> map;

        auto base_k = map.insert(0);
        auto old_k  = map.insert(1);
        (void)map.erase(map.find(old_k));
        auto new_k = map.insert(2);

        auto base_it = map.find(base_k);
        auto old_it  = map.find(old_k);
        auto new_it  = map.find(new_k);

        REQUIRE(base_it != map.end());
        REQUIRE(new_it != map.end());
        REQUIRE(old_it == new_it);

        REQUIRE(base_it->second == 0);
        REQUIRE(old_it->second == 2);
        REQUIRE(new_it->second == 2);
    }

    SECTION("test emplace") {
        Slot_map<int> map;
        std::vector   values = {0, 1, 2, 3, 4};
        for (auto v : values) {
            auto&& [k, ref] = map.emplace(v);
            REQUIRE(ref == v);
        }
    }

    SECTION("test emplace after erase") {
        Slot_map<int> map;

        auto base_k = map.emplace(0).key;
        auto old_k  = map.emplace(1).key;
        map.erase(old_k);
        auto new_k = map.emplace(2).key;

        auto base_it = map.find(base_k);
        auto old_it  = map.find(old_k);
        auto new_it  = map.find(new_k);

        REQUIRE(base_it != map.end());
        REQUIRE(new_it != map.end());
        REQUIRE(old_it == new_it);

        REQUIRE(base_it->second == 0);
        REQUIRE(new_it->second == 2);
    }

    SECTION("test erase") {
        Slot_map<int> map;

        auto k  = map.insert(2022);
        auto it = map.find(k);
        REQUIRE(it != map.end());

        (void)map.erase(it);
        REQUIRE(map.size() == 0);
        REQUIRE(map.find(k) == map.end());
    }

    SECTION("test erase all iterator") {
        Slot_map<int> map;
        std::vector   values = {0, 1, 2, 3, 4};
        for (auto v : values) {
            [[maybe_unused]] auto k = map.insert(v);
        }
        for (auto it = map.begin(); it != map.end();) {
            it = map.erase(it);
        }
        REQUIRE(map.empty());
    }

    SECTION("test erase all iterator backward") {
        Slot_map<int> map;
        std::vector   values = {0, 1, 2, 3, 4};
        for (auto v : values) {
            [[maybe_unused]] auto k = map.insert(v);
        }
        while (not map.empty()) {
            (void)map.erase(map.end() - 1);
        }
        REQUIRE(map.empty());
    }

    SECTION("test erase a key") {
        Slot_map<int> map;

        auto k = map.insert(0);

        map.erase(k);
        REQUIRE(map.size() == 0);
        REQUIRE(map.find(k) == map.end());
    }

    SECTION("test swap") {
        Slot_map<int> s1, s2;
        using key_type = decltype(s1)::key_type;
        std::vector<key_type> keys1;
        std::vector<key_type> keys2;
        std::array<int, 6>    vals = {0, 1, 2, 3, 4, 5};

        std::size_t i = 0;
        for (; i < 4; i++) {
            keys1.push_back(s1.insert(vals[i]));
        }
        for (; i < vals.size(); i++) {
            keys2.push_back(s2.insert(vals[i]));
        }

        swap(s1, s2);

        i = 0;
        for (; i < 4; i++) {
            auto k = keys1[i];
            REQUIRE(s2[k] == vals[i]);
        }
        for (; i < vals.size(); i++) {
            auto k = keys2[i - 4];
            REQUIRE(s1[k] == vals[i]);
        }
    }

    SECTION("test find") {
        Slot_map<int> map;
        auto          val = 0xffdead;
        auto          k   = map.insert(val);
        auto          it  = map.find(k);
        REQUIRE(it != map.end());
        REQUIRE(it->second == val);
    }

    SECTION("test find after erase") {
        Slot_map<int> map;
        auto          val = 0xffdead;
        auto          k   = map.insert(val);
        map.erase(k);
        auto it = map.find(k);
        REQUIRE(it == map.end());
    }

    SECTION("test find after clear") {
        Slot_map<int> map;
        auto          val = 0xffdead;
        auto          k   = map.insert(val);
        map.clear();
        auto it = map.find(k);
        REQUIRE(it == map.end());
    }

    SECTION("test access") {
        Slot_map<int> map;
        auto          val = 0xffdead;
        auto          k   = map.insert(val);
        REQUIRE(map[k] == val);
    }

    SECTION("test contains") {
        Slot_map<int> map;
        auto          val = 0xffdead;
        auto          k   = map.insert(val);
        REQUIRE(map.contains(k));
    }

    SECTION("test contains after erase") {
        Slot_map<int> map;
        auto          val = 0xffdead;
        auto          k   = map.insert(val);
        map.erase(k);
        REQUIRE_FALSE(map.contains(k));
    }

    SECTION("test contains after clear") {
        Slot_map<int> map;
        auto          val = 0xffdead;
        auto          k   = map.insert(val);
        map.clear();
        REQUIRE_FALSE(map.contains(k));
    }

    SECTION("test get data") {
        Slot_map<int> map;
        auto          val = 0xffdead;
        auto          k1  = map.insert(val);
        REQUIRE(&map[k1] == map.data());
    }

    SECTION("test compare with self") {
        Slot_map<int> s1;
        REQUIRE(s1 == s1);
    }

    SECTION("test compare with copy") {
        Slot_map<int> s1;
        auto          s2 = s1;
        REQUIRE(s1 == s2);
    }

    SECTION("test compare of two empty map") {
        Slot_map<int> s1;
        Slot_map<int> s2;
        REQUIRE(s1 == s2);
    }

    SECTION("test compare of two not empty map") {
        Slot_map<int>         s1;
        [[maybe_unused]] auto k = s1.insert(0);
        Slot_map<int>         s2;
        REQUIRE(s1 != s2);
    }

    SECTION("test compare of two maps with same values") {
        Slot_map<int> s1;
        for (int i = 0; i < 16; i++) {
            [[maybe_unused]] auto k = s1.insert(i);
        }
        Slot_map<int> s2;
        for (int i = 0; i < 16; i++) {
            REQUIRE(s1 != s2);
            [[maybe_unused]] auto k = s2.insert(i);
        }
        REQUIRE(s1 == s2);
    }

    SECTION("test pop") {
        Slot_map<int> map;

        auto k = map.insert(0);

        auto v = map.pop(k);
        REQUIRE(map.size() == 0);
        REQUIRE(map.find(k) == map.end());
        REQUIRE(v == 0);
    }

    SECTION("test pop all") {
        Slot_map<int> map;
        using Key = decltype(map)::key_type;

        std::vector      values = {0, 1, 2, 3, 4};
        std::vector<Key> keys;

        for (auto v : values) {
            auto k = map.insert(v);
            keys.push_back(k);
        }
        for (size_t i = 0; i < values.size(); i++) {
            auto k = keys[i];
            REQUIRE(map.size() == values.size() - i);
            auto v = map.pop(k);
            REQUIRE(v == values[i]);
        }
        REQUIRE(map.empty());
    }

    SECTION("test keys-values") {
        Slot_map<int>                        map;
        std::vector                          values = {0, 1, 2, 3, 4};
        std::vector<decltype(map)::key_type> keys;
        for (auto v : values) {
            keys.push_back(map.insert(v));
        }
        REQUIRE(salt::cxx20::is_permutation(map.keys(), keys));
        REQUIRE(salt::cxx20::is_permutation(map.values(), values));
    }
}