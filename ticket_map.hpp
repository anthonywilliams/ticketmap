#pragma once

#include <cstdlib>
#include <vector>
#include <algorithm>
#include <type_traits>
#include <optional>

namespace jss {

    template <typename Key, typename Value> class ticket_map {
        using collection_type=
            std::vector<std::pair<Key, std::optional<Value>>>;

        /// The iterator for our range
        template <bool is_const> class iterator_impl {
            using dereference_type=
                std::conditional_t<is_const, Value const &, Value &>;

        public:
            /// The value_type of our range is an key/value pair
            struct value_type {
                Key const &ticket;
                dereference_type value;
            };

        private:
            /// It's an input iterator, so we need a proxy for ->
            struct arrow_proxy {
                /// Our proxy operator->
                value_type *operator->() noexcept {
                    return &value;
                }

                /// The pointed-to value
                value_type value;
            };

        public:
            /// Required iterator typedefs
            using reference= value_type;
            /// Required iterator typedefs
            using iterator_category= std::input_iterator_tag;
            /// Required iterator typedefs
            using pointer= value_type *;
            /// Required iterator typedefs
            using difference_type= std::ptrdiff_t;

            /// Compare iterators for inequality.
            friend bool operator!=(
                iterator_impl const &lhs, iterator_impl const &rhs) noexcept {
                return lhs.iter != rhs.iter;
            }

            /// Equality in terms of iterator_impls: if it's not not-equal then
            /// it must be equal
            friend bool operator==(
                iterator_impl const &lhs, iterator_impl const &rhs) noexcept {
                return !(lhs != rhs);
            }

            /// Dereference the iterator
            const value_type operator*() const noexcept {
                return value_type{iter->first, *iter->second};
            }

            /// Dereference for iter->m
            arrow_proxy operator->() const noexcept {
                return arrow_proxy{value_type{iter->first, *iter->second}};
            }

            /// Pre-increment
            iterator_impl &operator++() noexcept {
                iter= map->next_valid(++iter);
                return *this;
            }

            /// Post-increment
            iterator_impl operator++(int) noexcept {
                iterator_impl temp{*this};
                ++*this;
                return temp;
            }

            template <
                typename Other,
                typename= std::enable_if_t<
                    std::is_same<Other, iterator_impl<false>>::value &&
                    is_const>>
            constexpr iterator_impl(Other const &other) noexcept :
                iter(other.iter), map(other.map) {}

            constexpr iterator_impl() noexcept= default;

        private:
            friend class ticket_map;
            friend class iterator_impl<!is_const>;

            using underlying_iterator= std::conditional_t<
                is_const, typename collection_type::const_iterator,
                typename collection_type::iterator>;

            using map_ptr=
                std::conditional_t<is_const, ticket_map const *, ticket_map *>;

            constexpr iterator_impl(underlying_iterator iter_, map_ptr map_) :
                iter(std::move(iter_)), map(map_) {}

            /// The stored iterator
            underlying_iterator iter;
            /// The map
            map_ptr map;
        };

    public:
        using iterator= iterator_impl<false>;
        using const_iterator= iterator_impl<true>;

        constexpr ticket_map() noexcept : nextId(), filledItems(0) {}

        constexpr ticket_map(ticket_map &&other) noexcept :
            nextId(std::move(other.nextId)), data(std::move(other.data)),
            filledItems(std::move(other.filledItems)) {
            other.filledItems= 0;
        }
        constexpr ticket_map(ticket_map const &other)= default;
        constexpr ticket_map &operator=(ticket_map const &other)= default;
        constexpr ticket_map &operator=(ticket_map &&other) noexcept {
            ticket_map temp(std::move(other));
            swap(temp);
            return *this;
        }

        constexpr bool empty() const noexcept {
            return size() == 0;
        }

        constexpr std::size_t size() const noexcept {
            return filledItems;
        }

        constexpr iterator insert(Value v) {
            auto res= iterator(
                data.insert(data.end(), {nextId++, std::move(v)}), this);
            ++filledItems;
            return res;
        }

        constexpr const_iterator find(const Key &key) const noexcept {
            return {lookup<true>(key), this};
        }

        constexpr iterator find(const Key &key) noexcept {
            return {lookup<false>(key), this};
        }

        constexpr iterator begin() noexcept {
            return {next_valid(data.begin()), this};
        }

        constexpr iterator end() noexcept {
            return {data.end(), this};
        }

        constexpr iterator erase(const Key &key) noexcept {
            return {erase_entry(lookup<false>(key)), this};
        }

        constexpr iterator erase(const_iterator pos) noexcept {
            return {erase_entry(data.begin() + (pos.iter - data.begin())),
                    this};
        }

        constexpr void swap(ticket_map &other) noexcept {
            data.swap(other.data);
            std::swap(filledItems, other.filledItems);
            std::swap(nextId, other.nextId);
        }

        constexpr void clear() noexcept {
            data.clear();
            filledItems= 0;
        }

    private:
        template <typename Iter>
        constexpr Iter next_valid(Iter iter) const noexcept {
            for(; iter != data.end() && !iter->second; ++iter)
                ;
            return iter;
        }

        constexpr typename collection_type::iterator
        erase_entry(typename collection_type::iterator iter) {
            if(iter != data.end()) {
                iter->second.reset();
                iter= next_valid(iter);
                --filledItems;
                if(needs_compaction()) {
                    auto key=
                        iter != data.end() ? iter->first : std::optional<Key>();
                    compact();
                    iter= key ? lookup<false>(*key) : data.end();
                }
            }
            return iter;
        }

        template <bool is_const>
        std::conditional_t<
            is_const, typename collection_type::const_iterator,
            typename collection_type::iterator>
        lookup(Key const &key) noexcept {
            auto pos= std::lower_bound(
                data.begin(), data.end(), key,
                [](auto &value, const Key &key) { return value.first < key; });

            if(pos == data.end() || pos->first != key || !pos->second)
                return data.end();
            return pos;
        }

        bool needs_compaction() const noexcept {
            return filledItems < (data.size() / 2);
        }

        void compact() {
            data.erase(
                std::remove_if(
                    data.begin(), data.end(),
                    [](auto &entry) { return !entry.second; }),
                data.end());
        }

        Key nextId;
        collection_type data;
        std::size_t filledItems;
    };
} // namespace jss

namespace std {

    template <typename Key, typename Value>
    void swap(
        jss::ticket_map<Key, Value> &lhs,
        jss::ticket_map<Key, Value> &rhs) noexcept {
        lhs.swap(rhs);
    }
} // namespace std
