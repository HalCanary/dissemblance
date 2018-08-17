.PHONY: test clean

test: bin/dissemblance
	./test_dissemblance.sh

CXXFLAGS := $(CXXFLAGS) --std=c++11

bin/dissemblance: dissemblance.cpp number.h
	mkdir -p bin
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $< -o $@

clean:
	rm -rf bin
