CCFLAGS = -std=c++11 -pedantic -Wall -g
huffman: huffman.o
	g++ $(CCFLAGS) -o $@ $<

huffman.o: huffman.cpp huffman.hpp
	g++ $(CCFLAGS) -c $< -o $@

.PHONY: clean

clean:
	rm -f huffman.o huffman