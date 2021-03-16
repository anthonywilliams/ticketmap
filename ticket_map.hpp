#pragma once

#include <cstdlib>
#include <vector>

namespace jss {

    template <typename Key, typename Value> class ticket_map {
        using collection_type= std::vector<std::pair<Key, Value>>;

    public:
        /// The value_type of our range is an index/value pair
        struct value_type {
            Key const &ticket;
            Value &value;
        };

        /// The iterator for our range
        class iterator {
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
            using value_type= typename ticket_map::value_type;
            /// Required iterator typedefs
            using reference= value_type;
            /// Required iterator typedefs
            using iterator_category= std::input_iterator_tag;
            /// Required iterator typedefs
            using pointer= value_type *;
            /// Required iterator typedefs
            using difference_type= std::ptrdiff_t;

            // /// Compare iterators for inequality.
            // friend bool
            // operator!=(iterator const &lhs, iterator const &rhs) noexcept {}

            // /// Equality in terms of iterators: if it's not not-equal then
            // /// it must be equal
            // friend bool
            // operator==(iterator const &lhs, iterator const &rhs) noexcept {
            //     return !(lhs != rhs);
            // }

            // /// Dereference the iterator
            // const value_type operator*() const noexcept {}

            /// Dereference for iter->m
            arrow_proxy operator->() const noexcept {
                return arrow_proxy{value_type{iter->first, iter->second}};
            }

            /// Pre-increment
            iterator &operator++() noexcept {
                ++iter;
                return *this;
            }

            /// Post-increment
            iterator operator++(int) noexcept {
                iterator temp{**this};
                ++*this;
                return temp;
            }

        private:
            friend class ticket_map;

            constexpr iterator(typename collection_type::iterator iter_) :
                iter(std::move(iter_)) {}

            /// The stored iterator
            typename collection_type::iterator iter;
        };

        constexpr ticket_map() noexcept : nextId(), filledItems(0) {}

        constexpr bool empty() const noexcept {
            return size() == 0;
        }

        constexpr std::size_t size() const noexcept {
            return filledItems;
        }

        constexpr iterator insert(Value v) {
            auto res=
                iterator(data.insert(data.end(), {nextId++, std::move(v)}));
            ++filledItems;
            return res;
        }

    private:
        Key nextId;
        collection_type data;
        std::size_t filledItems;
    };
} // namespace jss
