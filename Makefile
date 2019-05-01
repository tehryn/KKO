CCFLAGS = -std=c++11 -o3 -march=native
all: huff_codec

debug : CCFLAGS = -std=c++11 -g
debug : all

analyze: CCFLAGS += -pg
analyze: all

huff_codec: huffman.o Tree.o Coder.o
	g++ $(CCFLAGS) -o $@ $^

huffman.o: huffman.cpp huffman.hpp Coder.hpp
	g++ $(CCFLAGS) -c $< -o $@

Tree.o: Tree.cpp Tree.hpp
	g++ $(CCFLAGS) -c $< -o $@

Coder.o: Coder.cpp Coder.hpp Tree.hpp
	g++ $(CCFLAGS) -c $< -o $@

.PHONY: clean quickTest zip debug all

zip:
	zip kko_xmatej52.zip Tree.cpp Tree.hpp Coder.cpp Coder.hpp huffman.cpp huffman.hpp Makefile

clean:
	rm -f huffman.o Tree.o Coder.o huff_codec .code.compress .code.decompress .diffIn .diffOut test.raw test.huffman

quickTest:
	make clean
	make debug
	./huff_codec -i data/hd07.raw -o test.huffman -c -h static -m 2>.code.compress
	./huff_codec -i test.huffman -o test.raw -d -h static -m 2>.code.decompress
	xxd -b data/hd07.raw > .diffIn
	xxd -b test.raw > .diffOut
	#diff .diffIn .diffOut
