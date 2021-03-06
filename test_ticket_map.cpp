#include "ticket_map.hpp"
#include <assert.h>
#include <type_traits>
#include <string>
#include <iostream>

void test_initially_empty() {
    jss::ticket_map<int, int> map;

    assert(map.empty());
    assert(map.size() == 0);
}

void test_inserting_a_value_gives_ticket_for_new_element() {
    jss::ticket_map<int, int> map;

    auto ticket= map.insert(42);

    assert(ticket == 0);
    assert(map[ticket] == 42);
}

void test_inserting_a_second_value_gives_new_ticket() {
    jss::ticket_map<int, int> map;

    map.insert(99);
    auto ticket= map.insert(42);

    assert(ticket == 1);
}

void test_inserting_a_bunch_of_elements_gives_iterators_and_updates_size() {
    jss::ticket_map<int, int> map;

    auto const count= 100;

    for(unsigned i= 0; i < count; ++i) {
        auto ticket= map.insert(i);
        assert(ticket == i);
        assert(map[ticket] == i);
        assert(map.size() == i + 1);
        assert(!map.empty());
    }
}

void test_value_can_be_retrieved_if_inserted() {
    jss::ticket_map<int, int> map;

    map.insert(42);
    auto iter= map.find(0);

    assert(iter->ticket == 0);
    assert(iter->value == 42);
}

void test_can_iterate_over_values() {
    jss::ticket_map<int, int> map;

    std::vector<int> const values= {2,  3,    56,          12, 99, -12,
                                    42, 1213, -1283137618, 0,  12, 12};

    for(auto &e : values) {
        map.insert(e);
    }

    std::size_t index= 0;
    for(auto &entry : map) {
        static_assert(
            std::is_same<decltype(entry.ticket), const int &>::value,
            "Ticket must be int");
        static_assert(
            std::is_same<decltype(entry.value), int &>::value,
            "Value must be ref");
        assert(entry.ticket == index);
        assert(entry.value == values[index]);
        ++index;
    }
}

void test_empty_map_has_begin_equal_end() {
    jss::ticket_map<int, std::string> map;
    assert(map.begin() == map.end());
}

void test_begin_and_end_types() {
    jss::ticket_map<int, std::string> map;

    static_assert(
        std::is_same<
            typename decltype(map)::iterator, decltype(map.begin())>::value,
        "begin must return an iterator");
    static_assert(
        std::is_same<decltype((*map.begin()).ticket), int const &>::value,
        "ticket is specified type");
    static_assert(
        std::is_same<decltype((*map.begin()).value), std::string &>::value,
        "value is ref");
}

void test_after_erasing_value_not_there() {
    jss::ticket_map<unsigned short, std::string> map;

    auto ticket= map.insert("hello");
    assert(ticket == map.begin()->ticket);

    assert(map.find(ticket) == map.begin());

    auto iter2= map.erase(ticket);

    assert(map.size() == 0);
    assert(map.empty());

    assert(map.begin() == map.end());
    assert(map.find(ticket) == map.end());
    assert(iter2 == map.end());
}

void test_after_erasing_middle_element_iteration_skips_it() {
    jss::ticket_map<unsigned short, std::string> map;

    map.insert("first");
    auto ticket= map.insert("second");
    map.insert("third");
    auto after_erased= map.erase(ticket);
    assert(after_erased != map.end());
    assert(after_erased->ticket == 2);
    assert(after_erased->value == "third");

    assert(map.size() == 2);

    unsigned index= 0;
    auto it= map.begin();
    assert(it != after_erased);
    assert(it != map.end());
    assert(it->ticket == 0);
    assert(it->value == "first");
    assert(it != after_erased);
    ++it;
    assert(it == after_erased);
    assert(it != map.end());
    assert(it->ticket == 2);
    assert(it->value == "third");
    assert((it++)->ticket == 2);
    assert(it != after_erased);
    assert(it == map.end());
}

void test_after_erasing_last_element_iteration_skips_it() {
    jss::ticket_map<unsigned short, std::string> map;

    map.insert("first");
    map.insert("second");
    auto ticket= map.insert("third");
    auto after_erased= map.erase(ticket);
    assert(after_erased == map.end());

    assert(map.size() == 2);

    unsigned index= 0;
    auto it= map.begin();
    assert(it != after_erased);
    assert(it != map.end());
    assert(it->ticket == 0);
    assert(it->value == "first");
    assert(it != after_erased);
    ++it;
    assert(it != after_erased);
    assert(it != map.end());
    assert(it->ticket == 1);
    assert(it->value == "second");
    assert((it++)->ticket == 1);
    assert(it == after_erased);
    assert(it == map.end());
}

void test_after_erasing_new_elements_added_correctly() {
    jss::ticket_map<unsigned short, int> map;

    unsigned const count= 11;
    for(unsigned i= 0; i < count; ++i) {
        map.insert(i);
    }

    for(unsigned i= 0; i < count; i+= 2) {
        map.erase(i);
    }

    assert(map.size() == count / 2);

    auto ticket= map.insert(99);
    assert(ticket == count);
    assert(map[ticket] == 99);

    assert(map.size() == count / 2 + 1);

    std::vector<int> expected;
    for(unsigned i= 1; i < count; i+= 2) {
        expected.push_back(i);
    }
    expected.push_back(99);

    assert(map.size() == expected.size());
    unsigned i= 0;
    for(auto &e : map) {
        assert(e.value == expected[i]);
        ++i;
    }
}

void test_after_erasing_lots_of_elements_still_ok() {
    jss::ticket_map<unsigned short, int> map;

    unsigned const count= 1000;
    for(unsigned i= 0; i < count; ++i) {
        map.insert(i);
    }

    auto const cutoff= (count * 9 / 10);

    for(unsigned i= 0; i < cutoff; ++i) {
        map.erase(map.begin()->ticket);
    }

    unsigned val= cutoff;
    for(auto &e : map) {
        assert(e.value == val);
        ++val;
    }
}

void test_can_erase_with_iterator() {
    jss::ticket_map<long, std::string> map;

    map.insert("first");
    auto ticket= map.insert("second");
    map.insert("third");
    auto iter= map.find(ticket);
    auto after_erased= map.erase(iter);
    assert(after_erased != map.end());
    assert(after_erased->ticket == 2);
    assert(after_erased->value == "third");

    assert(map.size() == 2);
}

void test_erase_lots_of_element_use_returned_iterator() {
    jss::ticket_map<unsigned short, int> map;

    unsigned const count= 1000;
    for(unsigned i= 0; i < count; ++i) {
        map.insert(i);
    }

    auto const cutoff= (count * 9 / 10);

    auto iter= map.begin();
    for(unsigned i= 0; i < cutoff; ++i) {
        iter= map.erase(iter);
    }

    assert(iter->value == cutoff);
    assert(iter == map.begin());

    unsigned val= cutoff;
    for(; iter != map.end(); ++iter, ++val) {
        assert(iter->value == val);
    }

    val= cutoff;
    for(auto &e : map) {
        assert(e.value == val);
        ++val;
    }
}

void test_swapping_containers_swaps_everything_including_next_ticket() {
    jss::ticket_map<int, int> a, b;

    unsigned const count1= 100;
    for(unsigned i= 0; i < count1; ++i) {
        a.insert(i);
    }

    unsigned const count2= 200;
    for(unsigned i= 0; i < count2; ++i) {
        b.insert(i + 1000);
    }

    assert(a.size() == count1);
    assert(b.size() == count2);

    a.swap(b);

    for(unsigned i= 0; i < count2; ++i) {
        assert(a.find(i)->value == i + 1000);
    }

    for(unsigned i= 0; i < count1; ++i) {
        assert(b.find(i)->value == i);
    }

    assert(b.size() == count1);
    assert(a.size() == count2);

    auto ticket1= a.insert(99);
    auto ticket2= b.insert(99);

    assert(ticket1 == count2);
    assert(ticket2 == count1);
}

void test_can_swap_with_std_swap() {
    jss::ticket_map<int, int> a, b;

    unsigned const count1= 100;
    for(unsigned i= 0; i < count1; ++i) {
        a.insert(i);
    }

    unsigned const count2= 200;
    for(unsigned i= 0; i < count2; ++i) {
        b.insert(i + 1000);
    }

    assert(a.size() == count1);
    assert(b.size() == count2);

    std::swap(a, b);

    for(unsigned i= 0; i < count2; ++i) {
        assert(a.find(i)->value == i + 1000);
    }

    for(unsigned i= 0; i < count1; ++i) {
        assert(b.find(i)->value == i);
    }

    assert(b.size() == count1);
    assert(a.size() == count2);

    auto ticket1= a.insert(99);
    auto ticket2= b.insert(99);

    assert(ticket1 == count2);
    assert(ticket2 == count1);
}

void test_clear_removes_elements_but_does_not_reset_ticket() {
    jss::ticket_map<unsigned short, int> map;

    unsigned const count= 100;
    for(unsigned i= 0; i < count; ++i) {
        map.insert(i);
    }

    map.clear();

    assert(map.size() == 0);
    assert(map.empty());

    auto ticket= map.insert(42);
    assert(ticket == count);
}

void test_copies_preserve_elements() {
    jss::ticket_map<unsigned short, int> map;

    unsigned const count= 100;
    for(unsigned i= 0; i < count; ++i) {
        map.insert(i + 1000);
    }

    jss::ticket_map<unsigned short, int> map2(map);

    assert(map2.size() == map.size());

    for(auto &e : map) {
        auto found= map2.find(e.ticket);
        assert(found != map2.end());
        assert(found->ticket == e.ticket);
        assert(found->value == e.value);
        assert(found != map.find(e.ticket));
    }

    assert(map2.insert(-1) == count);

    jss::ticket_map<unsigned short, int> map3;

    map3= map;

    assert(map3.size() == map.size());

    for(auto &e : map) {
        auto found= map3.find(e.ticket);
        assert(found != map3.end());
        assert(found->ticket == e.ticket);
        assert(found->value == e.value);
        assert(found != map.find(e.ticket));
    }

    assert(map3.insert(-1) == count);
}

void test_move_transfers_elements() {
    jss::ticket_map<unsigned short, int> map;

    unsigned const count= 100;
    for(unsigned i= 0; i < count; ++i) {
        map.insert(i + 1000);
    }

    static_assert(
        std::is_nothrow_move_constructible<jss::ticket_map<int, int>>::value);
    static_assert(
        std::is_nothrow_move_assignable<jss::ticket_map<int, int>>::value);

    jss::ticket_map<unsigned short, int> map2(std::move(map));

    assert(map2.size() == count);
    assert(map.size() == 0);
    assert(map.empty());

    for(auto &e : map2) {
        assert(e.value == e.ticket + 1000);
    }

    assert(map2.insert(-1) == count);

    jss::ticket_map<unsigned short, int> map3;

    map3= std::move(map2);

    assert(map3.size() == count + 1);
    assert(map2.size() == 0);
    assert(map2.empty());
    assert(map3.insert(-1) == count + 1);
    assert(map2.insert(-1) == count + 1);
}

void test_bulk_insert() {
    jss::ticket_map<unsigned short, int> map;

    unsigned const count= 100;
    for(unsigned i= 0; i < count; ++i) {
        map.insert(i + 1000);
    }

    std::vector<int> const extras= {1, 2, 42, 59, 66, 78, 99};

    auto iter= map.insert(extras.begin(), extras.end());

    assert(iter != map.end());
    assert(iter->ticket == count);
    assert(map.size() == count + extras.size());
    for(auto it= extras.begin(); it != extras.end(); ++it, ++iter) {
        assert(iter != map.end());
        assert(iter->value == *it);
    }
    assert(iter == map.end());
}

void test_construct_from_range() {
    std::vector<int> const entries= {1, 2, 42, 59, 66, 78, 99};
    jss::ticket_map<unsigned short, int> map(entries.begin(), entries.end());
    assert(map.size() == entries.size());
    assert(!map.empty());
    auto iter= map.begin();
    for(auto it= entries.begin(); it != entries.end(); ++it, ++iter) {
        assert(iter != map.end());
        assert(iter->value == *it);
    }
    assert(iter == map.end());
    assert(map.size() == entries.size());

    assert(map.insert(99) == entries.size());
}

void test_emplace() {
    struct X {
        int val;
        std::string str;

        X(int i, std::string const &s) : val(i + 100), str(s + " world") {}
    };

    jss::ticket_map<int, X> map;

    auto ticket= map.emplace(42, "hello");
    assert(ticket == 0);
    assert(map[ticket].val == 142);
    assert(map[ticket].str == "hello world");
    assert(map.size() == 1);
}

void test_ticket_must_be_incrementable() {
    // jss::ticket_map<std::string, int> map; // should fail to compile
}

void test_direct_lookup() {
    jss::ticket_map<int, std::string> map;

    auto ticket= map.insert("hello");

    std::string &s= map[ticket];
    assert(&s == &map.begin()->value);
    assert(s == "hello");

    try {
        map[ticket + 1];
        assert(!"Should throw");
    } catch(std::out_of_range &) {
        assert(true);
    } catch(...) {
        assert(!"Should throw out-of-range");
    }

    auto const &cmap= map;

    static_assert(std::is_same_v<decltype(cmap[ticket]), const std::string &>);
    assert(&map[ticket] == &s);
}

void test_iteration_over_const() {
    std::vector<int> const entries= {1, 2, 42, 59, 66, 78, 99};
    jss::ticket_map<int, int> map(entries.begin(), entries.end());

    jss::ticket_map<int, int> const &cmap= map;
    jss::ticket_map<int, int>::const_iterator iter= cmap.begin();
    for(auto it= entries.begin(); it != entries.end(); ++it, ++iter) {
        assert(iter != cmap.end());
        assert(iter->value == *it);
    }

    static_assert(
        std::is_same_v<
            decltype(map.cbegin()), jss::ticket_map<int, int>::const_iterator>);

    iter= map.cbegin();
    for(auto it= entries.begin(); it != entries.end(); ++it, ++iter) {
        assert(iter != map.cend());
        assert(iter->value == *it);
    }
}

void test_can_reserve() {
    struct MoveCounter {
        unsigned count;

        MoveCounter() : count(0) {}
        MoveCounter(MoveCounter &&other) : count(other.count + 1) {}
        MoveCounter &operator=(MoveCounter &&other) {
            count= other.count + 1;
            return *this;
        }
    };

    jss::ticket_map<int, MoveCounter> map;

    unsigned const count= 45;
    map.reserve(count);
    assert(map.insert_capacity() >= count);

    auto const initial_capacity= map.insert_capacity();

    for(unsigned i= 0; i < count; ++i) {
        map.emplace();
    }

    assert(map.size() == count);
    assert(map[0].count == 0);

    assert(map.insert_capacity() == initial_capacity - count);
}

struct MyTicket {
    int i;
    MyTicket() : i(100) {}

    MyTicket operator++(int) {
        MyTicket res(*this);
        i+= 10;
        return res;
    }
    friend bool operator==(MyTicket const &lhs, MyTicket const &rhs) noexcept {
        return lhs.i == rhs.i;
    }
    friend bool operator!=(MyTicket const &lhs, MyTicket const &rhs) noexcept {
        return lhs.i != rhs.i;
    }
    friend bool operator<(MyTicket const &lhs, MyTicket const &rhs) noexcept {
        return lhs.i < rhs.i;
    }
};

void test_custom_ticket_type() {
    jss::ticket_map<MyTicket, int> map;

    auto ticket= map.insert(42);

    static_assert(std::is_same_v<decltype(ticket), MyTicket>);
    assert(ticket.i == 100);

    auto ticket2= map.insert(99);
    assert(ticket2.i == 110);

    assert(map[ticket] == 42);
    assert(map[ticket2] == 99);
}

void test_cannot_overflow() {
    jss::ticket_map<unsigned char, int> map;

    for(unsigned i= 0; i < 256; ++i) {
        map.insert(i);
    }

    assert(map.size() == 256);

    try {
        map.insert(-1);
        assert(!"Should not be able to insert if ticket overflows");
    } catch(std::overflow_error &) {
        assert(true);
    } catch(...) {
        assert(!"Wrong type of exception thrown");
    }
}

void test_cannot_overflow_signed() {
    jss::ticket_map<signed char, int> map;

    for(unsigned i= 0; i < 128; ++i) {
        map.insert(i);
    }

    assert(map.size() == 128);

    try {
        map.insert(-1);
        assert(!"Should not be able to insert if ticket overflows");
    } catch(std::overflow_error &) {
        assert(true);
    } catch(...) {
        assert(!"Wrong type of exception thrown");
    }
}

struct SmallTicket {
    unsigned char i;
    SmallTicket() : i(0) {}

    SmallTicket operator++(int) {
        SmallTicket res(*this);
        i+= 1;
        return res;
    }
    friend bool
    operator==(SmallTicket const &lhs, SmallTicket const &rhs) noexcept {
        return lhs.i == rhs.i;
    }
    friend bool
    operator!=(SmallTicket const &lhs, SmallTicket const &rhs) noexcept {
        return lhs.i != rhs.i;
    }
    friend bool
    operator<(SmallTicket const &lhs, SmallTicket const &rhs) noexcept {
        return lhs.i < rhs.i;
    }
};

void test_cannot_overflow_custom() {
    jss::ticket_map<SmallTicket, int> map;

    for(unsigned i= 0; i < 256; ++i) {
        map.insert(i);
    }

    assert(map.size() == 256);

    try {
        map.insert(-1);
        assert(!"Should not be able to insert if ticket overflows");
    } catch(std::overflow_error &) {
        assert(true);
    } catch(...) {
        assert(!"Wrong type of exception thrown");
    }
}

void test_count() {
    jss::ticket_map<int, int> map;
    assert(!map.count(0));
    assert(!map.count(1));
    map.insert(42);
    assert(map.count(0) == 1);
    assert(!map.count(1));

    map.erase(0);
    assert(!map.count(0));
    assert(!map.count(1));
}

int main() {
    test_initially_empty();
    test_inserting_a_value_gives_ticket_for_new_element();
    test_inserting_a_second_value_gives_new_ticket();
    test_inserting_a_bunch_of_elements_gives_iterators_and_updates_size();
    test_can_iterate_over_values();
    test_empty_map_has_begin_equal_end();
    test_begin_and_end_types();
    test_after_erasing_value_not_there();
    test_after_erasing_middle_element_iteration_skips_it();
    test_after_erasing_last_element_iteration_skips_it();
    test_after_erasing_new_elements_added_correctly();
    test_after_erasing_lots_of_elements_still_ok();
    test_can_erase_with_iterator();
    test_erase_lots_of_element_use_returned_iterator();
    test_swapping_containers_swaps_everything_including_next_ticket();
    test_can_swap_with_std_swap();
    test_clear_removes_elements_but_does_not_reset_ticket();
    test_copies_preserve_elements();
    test_move_transfers_elements();
    test_bulk_insert();
    test_construct_from_range();
    test_emplace();
    test_direct_lookup();
    test_iteration_over_const();
    test_can_reserve();
    test_custom_ticket_type();
    test_cannot_overflow();
    test_count();
    test_cannot_overflow_signed();
    test_cannot_overflow_custom();
}
