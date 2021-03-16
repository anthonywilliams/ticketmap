#include "ticket_map.hpp"
#include <assert.h>

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

int main() {
    test_initially_empty();
    test_inserting_a_value_gives_iterator_to_new_element();
}
