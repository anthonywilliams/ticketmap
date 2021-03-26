// This code is released under the Boost Software License
// https://www.boost.org/LICENSE_1_0.txt
// (C) Copyright 2021 Anthony Williams

#pragma once

#include <cstdlib>
#include <vector>
#include <algorithm>
#include <type_traits>
#include <optional>
#include <stdexcept>

namespace jss {

    /// A map between from Ticket values to Value values.
    ///
    /// Ticket must be default-constructible, incrementable, less-than
    /// comparable and equality-comparable. Value must be move-constructible
    ///
    /// When new values are inserted they are assigned new Ticket values
    /// automatically. If the Ticket value overflows then no more values can be
    /// inserted.
    template <typename Ticket, typename Value> class ticket_map {

        static_assert(
            std::is_default_constructible<Ticket>(),
            "Ticket must be default constructible");
        static_assert(
            std::is_same_v<decltype(std::declval<Ticket &>()++), Ticket>,
            "Ticket must be post incrementable");
        static_assert(
            std::is_same_v<
                decltype(std::declval<Ticket &>() < std::declval<Ticket &>()),
                bool>,
            "Ticket must be less-than-comparable");
        static_assert(
            std::is_same_v<
                decltype(std::declval<Ticket &>() == std::declval<Ticket &>()),
                bool>,
            "Ticket must be equality-comparable");
        static_assert(
            std::is_same_v<
                decltype(std::declval<Ticket &>() != std::declval<Ticket &>()),
                bool>,
            "Ticket must be inequality-comparable");

        /// The type of the actual storage
        using collection_type=
            std::vector<std::pair<Ticket, std::optional<Value>>>;

        /// The iterator for our map
        template <bool is_const> class iterator_impl {
            using dereference_type=
                std::conditional_t<is_const, Value const &, Value &>;

        public:
            /// The value_type of our iterator is a ticket/value pair. We use
            /// references, since the underlying storage doesn't hold the same
            /// member types
            struct value_type {
                /// A reference to the ticket value for this element
                Ticket const &ticket;
                /// A reference to the data value for this element
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
            using difference_type= void;

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

            /// Allow constructing a const_iterator from a non-const iterator,
            /// but not vice-versa
            template <
                typename Other,
                typename= std::enable_if_t<
                    std::is_same<Other, iterator_impl<false>>::value &&
                    is_const>>
            constexpr iterator_impl(Other const &other) noexcept :
                iter(other.iter), map(other.map) {}

            /// A default-constructed iterator is a sentinel value
            constexpr iterator_impl() noexcept= default;

        private:
            friend class ticket_map;
            friend class iterator_impl<!is_const>;

            using underlying_iterator= std::conditional_t<
                is_const, typename collection_type::const_iterator,
                typename collection_type::iterator>;

            using map_ptr=
                std::conditional_t<is_const, ticket_map const *, ticket_map *>;

            /// Construct from an iterator into a map
            constexpr iterator_impl(underlying_iterator iter_, map_ptr map_) :
                iter(std::move(iter_)), map(map_) {}

            /// The stored iterator
            underlying_iterator iter;
            /// The map
            map_ptr map;
        };

    public:
        /// Standard iterator typedef
        using iterator= iterator_impl<false>;
        /// Standard const_iterator typedef
        using const_iterator= iterator_impl<true>;

        /// Construct an empty map
        constexpr ticket_map() noexcept : nextId(), filledItems(0) {}

        /// Construct a map from a range of elements
        template <typename Iter>
        constexpr ticket_map(Iter first, Iter last) : ticket_map() {
            insert(first, last);
        }

        /// Move-construct from other. The elements of other are transferred to
        /// *this; other is left empty
        constexpr ticket_map(ticket_map &&other) noexcept :
            nextId(std::move(other.nextId)), data(std::move(other.data)),
            filledItems(std::move(other.filledItems)) {
            other.filledItems= 0;
        }
        /// Copy-construct from other. *this will have the same elements and
        /// next ticket value as other.
        constexpr ticket_map(ticket_map const &other)= default;
        /// Copy-assign from other
        constexpr ticket_map &operator=(ticket_map const &other) {
            ticket_map temp(other);
            swap(temp);
            return *this;
        }
        /// Move-assign from other
        constexpr ticket_map &operator=(ticket_map &&other) noexcept {
            ticket_map temp(std::move(other));
            swap(temp);
            return *this;
        }

        /// Returns true if there are no elements currently in the map, false
        /// otherwise
        constexpr bool empty() const noexcept {
            return size() == 0;
        }

        /// Returns the number of elements currently in the map
        constexpr std::size_t size() const noexcept {
            return filledItems;
        }

        /// Insert a new value into the map. It is assigned a new ticket value.
        /// Returns the ticket for the new entry.
        /// Invalidates any existing iterators into the map.
        /// Throws overflow_error if the Ticket values have overflowed.
        constexpr Ticket insert(Value v) {
            return emplace(std::move(v));
        }

        /// Insert a set of new values into the map. Each is assigned a new
        /// ticket value. Returns an iterator that references the first new
        /// entry, or end() if no values were inserted.
        /// Invalidates any existing iterators into the map.
        /// Throws overflow_error if the Ticket values have overflowed.
        template <typename Iter>
        constexpr iterator insert(Iter first, Iter last) {
            auto const index= data.size();
            for(; first != last; ++first) {
                emplace(*first);
            }
            return {data.begin() + index, this};
        }

        /// Insert a new value into the map, directly constructing in place. It
        /// is assigned a new ticket value. Returns the ticket for the new
        /// entry. Invalidates any existing iterators into the map.
        /// Throws overflow_error if the Ticket values have overflowed.
        template <typename... Args> constexpr Ticket emplace(Args &&... args) {
            if(overflow)
                throw std::overflow_error(
                    "Ticket values overflowed; cannot insert");
            auto id= increment_with_overflow_check(nextId, overflow);

            if(!insert_capacity()) {
                reserve(size() * 2);
            }
            auto baseIter= data.insert(data.end(), {id, std::nullopt});
            baseIter->second.emplace(std::forward<Args>(args)...);
            ++filledItems;
            return id;
        }

        /// Find a value in the map by its ticket. Returns an iterator referring
        /// to the found element, or end() if no element could be found
        constexpr const_iterator find(const Ticket &ticket) const noexcept {
            return {lookup(data, ticket), this};
        }

        /// Find a value in the map by its ticket. Returns an iterator referring
        /// to the found element, or end() if no element could be found
        constexpr iterator find(const Ticket &ticket) noexcept {
            return {lookup(data, ticket), this};
        }

        /// Find a value in the map by its ticket. Returns a reference to the
        /// found element. Throws std:out_of_range if the value was not present.
        constexpr Value &operator[](const Ticket &ticket) {
            return index(data, ticket);
        }

        /// Find a value in the map by its ticket. Returns a reference to the
        /// found element. Throws std:out_of_range if the value was not present.
        constexpr const Value &operator[](const Ticket &ticket) const {
            return index(data, ticket);
        }

        /// Returns an iterator to the first element, or end() if the container
        /// is empty
        constexpr iterator begin() noexcept {
            return {next_valid(data.begin()), this};
        }

        /// Returns an iterator one-past-the-end of the container
        constexpr iterator end() noexcept {
            return {data.end(), this};
        }

        /// Returns a const_iterator to the first element, or end() if the
        /// container is empty
        constexpr const_iterator begin() const noexcept {
            return {next_valid(data.begin()), this};
        }

        /// Returns a const_iterator one-past-the-end of the container
        constexpr const_iterator end() const noexcept {
            return {data.end(), this};
        }

        /// Returns a const_iterator to the first element, or cend() if the
        /// container is empty
        constexpr const_iterator cbegin() const noexcept {
            return {next_valid(data.begin()), this};
        }

        /// Returns a const_iterator one-past-the-end of the container
        constexpr const_iterator cend() const noexcept {
            return {data.end(), this};
        }

        /// Remove an element with the specified ticket. Returns an iterator to
        /// the next element if there is one, or end() otherwise. Returns end()
        /// if there was no element with the specified ticket.
        /// Invalidates any existing iterators into the map.
        /// Compacts the data if there are too many empty slots.
        constexpr iterator erase(const Ticket &ticket) noexcept {
            return {erase_entry(lookup(data, ticket)), this};
        }

        /// Remove the element referenced by the provided iterator.
        /// Returns an iterator to the next element if there is one, or end()
        /// otherwise.
        /// Invalidates any existing iterators into the map.
        /// Compacts the data if there are too many empty slots.
        constexpr iterator erase(const_iterator pos) noexcept {
            return {erase_entry(data.begin() + (pos.iter - data.begin())),
                    this};
        }

        /// Swap the contents with other. Afterwards, other has the contents and
        /// next ticket value of *this prior to the call, and *this has the
        /// contents and next ticket value of other prior to the call.
        constexpr void swap(ticket_map &other) noexcept {
            data.swap(other.data);
            std::swap(filledItems, other.filledItems);
            std::swap(nextId, other.nextId);
        }

        /// Remove all elements from *this. Invalidates all iterators into the
        /// map.
        constexpr void clear() noexcept {
            data.clear();
            filledItems= 0;
        }

        /// Ensure the map has room for at least count items
        constexpr void reserve(std::size_t count) {
            if(count > size()) {
                collection_type new_data;
                new_data.reserve(count);
                for(auto &[ticket, value] : data) {
                    if(value) {
                        new_data.emplace_back(
                            std::move(ticket), std::move(value));
                    }
                }
                data.swap(new_data);
            } else {
                compact();
            }
        }

        /// Return the maximum number of items that can be inserted without
        /// reallocating
        constexpr std::size_t insert_capacity() const noexcept {
            return data.capacity() - data.size();
        }

        /// Return the number of entries for a ticket in the container. The
        /// return value is 1 if the ticket is in the container, 0 otherwise.
        constexpr std::size_t count(Ticket const &ticket) const noexcept {
            return (lookup(data, ticket) == data.end()) ? 0 : 1;
        }

    private:
        /// Find the next valid iterator into the map
        template <typename Iter>
        constexpr Iter next_valid(Iter iter) const noexcept {
            for(; iter != data.end() && !iter->second; ++iter)
                ;
            return iter;
        }

        /// Erase an entry referenced by an iterator into the internal vector
        constexpr typename collection_type::iterator
        erase_entry(typename collection_type::iterator iter) {
            if(iter != data.end()) {
                iter->second.reset();
                iter= next_valid(iter);
                --filledItems;
                if(needs_compaction()) {
                    auto ticket= iter != data.end() ? iter->first :
                                                      std::optional<Ticket>();
                    compact();
                    iter= ticket ? lookup(data, *ticket) : data.end();
                }
            }
            return iter;
        }

        /// Find an element based on a ticket value
        template <typename Collection>
        static constexpr std::conditional_t<
            std::is_const_v<std::remove_reference_t<Collection>>,
            typename collection_type::const_iterator,
            typename collection_type::iterator>
        lookup(Collection &data, Ticket const &ticket) noexcept {
            auto pos= std::lower_bound(
                data.begin(), data.end(), ticket,
                [](auto &value, const Ticket &ticket) {
                    return value.first < ticket;
                });

            if(pos == data.end() || pos->first != ticket || !pos->second)
                return data.end();
            return pos;
        }

        /// Get a reference to a value based on a ticket value
        template <typename Collection>
        static constexpr std::conditional_t<
            std::is_const_v<std::remove_reference_t<Collection>>, const Value &,
            Value &>
        index(Collection &data, Ticket const &ticket) {
            auto iter= lookup(data, ticket);
            if(iter == data.end())
                throw std::out_of_range("No entry for specified ticket");
            return *iter->second;
        }

        /// Returns true if the container has too many empty slot, false
        /// otherwise
        bool needs_compaction() const noexcept {
            return filledItems < (data.size() / 2);
        }

        /// Compact the container to remove all empty slots.
        void compact() {
            data.erase(
                std::remove_if(
                    data.begin(), data.end(),
                    [](auto &entry) { return !entry.second; }),
                data.end());
        }

        /// Increment a ticket and check for overflow (generic)
        template <typename T>
        static std::enable_if_t<!std::is_integral_v<T>, T>
        increment_with_overflow_check(T &value, bool &overflow) {
            auto id= value++;
            if(value < id || value == id)
                overflow= true;
            return id;
        }

        /// Increment a ticket and check for overflow (integral)
        template <typename T>
        static std::enable_if_t<std::is_integral_v<T>, T>
        increment_with_overflow_check(T &value, bool &overflow) {
            auto id= value;
            if(value == std::numeric_limits<T>::max())
                overflow= true;
            else
                ++value;
            return id;
        }

        bool overflow= false;
        Ticket nextId;
        collection_type data;
        std::size_t filledItems;
    };
} // namespace jss

namespace std {

    template <typename Ticket, typename Value>
    void swap(
        jss::ticket_map<Ticket, Value> &lhs,
        jss::ticket_map<Ticket, Value> &rhs) noexcept {
        lhs.swap(rhs);
    }
} // namespace std
