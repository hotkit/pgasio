
CXX=g++
CXXFLAGS=-Iinclude

test: tests/compile/include/memory.cpp
	$(CXX) $(CXXFLAGS) -c tests/compile/include/memory.cpp -o /dev/null

