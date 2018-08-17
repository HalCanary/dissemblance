.PHONY: test clean

test: bin/dissemblance
	./test_dissemblance.sh

CXXFLAGS := $(CXXFLAGS) --std=c++11

HEADERS := $(wildcard src/*.h)

bin/%.o : src/%.cpp $(HEADERS)
	mkdir -p bin
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

bin/dissemblance: bin/dissemblance.o bin/main.o
	$(CXX) $(LDFLAGS) $^ -o $@

clean:
	rm -rf bin
