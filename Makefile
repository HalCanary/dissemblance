all: bin/dissemblance
	./test_dissemblance.sh
bin/dissemblance: dissemblance.cpp number.h
	c++ -fno-exceptions -g --std=c++11 dissemblance.cpp -o bin/dissemblance
