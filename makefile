.PHONY: test

test: test_ticket_map
	./test_ticket_map

test_ticket_map: test_ticket_map.cpp ticket_map.hpp
	g++-10 -std=c++17 -g -o $@ $<
