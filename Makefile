bin/dissemblance:
	c++ -ferror-limit=1 -fno-exceptions -g --std=c++11 dissemblance.cpp -o bin/dissemblance
	./test_dissemblance.sh