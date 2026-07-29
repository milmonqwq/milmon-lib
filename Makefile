CXX ?= g++
CXXFLAGS ?= -std=c++17 -O2 -Wall -Wextra

.PHONY: test clean example bundle-example

test: build/test_library
	./build/test_library
	python3 -m unittest discover -s tests -p 'test_*.py' -v

LIBRARY_HEADERS := $(shell find include/milmon -name '*.hpp' -type f)

build/test_library: tests/test_library.cpp $(LIBRARY_HEADERS)
	mkdir -p build
	$(CXX) $(CXXFLAGS) -Iinclude tests/test_library.cpp -o $@

example: build/basic

build/basic: examples/basic.cpp $(LIBRARY_HEADERS)
	mkdir -p build
	$(CXX) $(CXXFLAGS) -Iinclude examples/basic.cpp -o $@

bundle-example:
	mkdir -p build
	python3 tools/bundle.py examples/basic.cpp -o build/submission.cpp --explain

clean:
	rm -rf build
