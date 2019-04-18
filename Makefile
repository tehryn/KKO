CCFLAGS = -std=c++11 -pedantic -Wall -g #-o3 -march=native
huffman: huffman.o Tree.o
	g++ $(CCFLAGS) -o $@ $^

huffman.o: huffman.cpp huffman.hpp
	g++ $(CCFLAGS) -c $< -o $@

Tree.o: Tree.cpp Tree.hpp
	g++ $(CCFLAGS) -c $< -o $@

.PHONY: clean quickTest

clean:
	rm -f huffman.o Tree.o huffman

quickTest:
	make clean
	make
	./huffman -i data/hd01.raw -o test.huffman -c -h static >.code.compress
	./huffman -i test.huffman -o test.raw -d -h static >.code.decompress
	xxd -b data/hd01.raw > .diffIn
	xxd -b test.raw > .diffOut
	diff .diffIn .diffOut
