#include "ticket_map.hpp"
#include <assert.h>

void test_initially_empty() {
    jss::ticket_map<int, int> map;

    assert(map.empty());
    assert(map.size() == 0);
}

int main() {
    test_initially_empty();
}
