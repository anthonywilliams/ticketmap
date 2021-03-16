#include "ticket_map.hpp"
#include <assert.h>
#include <type_traits>

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
        static_assert(std::is_same_v<decltype(entry.ticket), const int &>);
        static_assert(std::is_same_v<decltype(entry.value), int &>);
        assert(entry.ticket == index);
        assert(entry.value == values[index]);
        ++index;
    }
}

int main() {
    test_initially_empty();
    test_inserting_a_value_gives_iterator_to_new_element();
    test_inserting_a_second_value_gives_new_ticket();
    test_inserting_a_bunch_of_elements_gives_iterators_and_updates_size();
}
