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

void test_inserting_a_value_gives_iterator_to_new_element() {
    jss::ticket_map<int, int> map;

    auto iter= map.insert(42);

    assert(iter->ticket == 0);
    assert(iter->value == 42);
}

void test_inserting_a_second_value_gives_new_ticket() {
    jss::ticket_map<int, int> map;

    map.insert(99);
    auto iter= map.insert(42);

    assert(iter->ticket == 1);
    assert(iter->value == 42);
}

void test_inserting_a_bunch_of_elements_gives_iterators_and_updates_size() {
    jss::ticket_map<int, int> map;

    auto const count= 100;

    for(unsigned i= 0; i < count; ++i) {
        auto iter= map.insert(i);
        assert(iter->ticket == i);
        assert(iter->value == i);
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
        "key is specified type");
    static_assert(
        std::is_same<decltype((*map.begin()).value), std::string &>::value,
        "value is ref");
}

void test_after_erasing_value_not_there() {
    jss::ticket_map<unsigned short, std::string> map;

    auto iter= map.insert("hello");
    assert(iter == map.begin());
    assert(iter != map.end());

    auto const ticket= iter->ticket;
    assert(map.find(ticket) == iter);

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
    auto ticket= map.insert("second")->ticket;
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
    auto ticket= map.insert("third")->ticket;
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

    auto iter= map.insert(99);
    assert(iter->ticket == count);
    assert(iter->value == 99);

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
    auto ticket= map.insert("second")->ticket;
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

int main() {
    test_initially_empty();
    test_inserting_a_value_gives_iterator_to_new_element();
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
}
