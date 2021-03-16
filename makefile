.PHONY: test

test: test_ticket_map
	./test_ticket_map

test_ticket_map: test_ticket_map.cpp ticket_map.hpp
	g++ -std=c++2a -g -o $@ $<
