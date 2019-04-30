CCFLAGS = -std=c++11 -pedantic -Wall -g #-o3 -march=native
huffman: huffman.o Tree.o Coder.o
	g++ $(CCFLAGS) -o $@ $^

huffman.o: huffman.cpp huffman.hpp Coder.hpp
	g++ $(CCFLAGS) -c $< -o $@

Tree.o: Tree.cpp Tree.hpp
	g++ $(CCFLAGS) -c $< -o $@

Coder.o: Coder.cpp Coder.hpp Tree.hpp
	g++ $(CCFLAGS) -c $< -o $@

.PHONY: clean quickTest zip

zip:
	zip kko_xmatej52.zip Tree.cpp Tree.hpp Coder.cpp Coder.hpp huffman.cpp huffman.hpp Makefile

clean:
	rm -f huffman.o Tree.o Coder.o huffman .code.compress .code.decompress .diffIn .diffOut test.raw test.huffman

quickTest:
	make clean
	make
	./huffman -i data/hd07.raw -o test.huffman -c -h adaptive >.code.compress
	./huffman -i test.huffman -o test.raw -d -h adaptive >.code.decompress
	xxd -b data/hd07.raw > .diffIn
	xxd -b test.raw > .diffOut
	#diff .diffIn .diffOut
