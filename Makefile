CXX = g++
CXXFLAGS = -std=c++11 -Wall -Wextra -g

all: project1 project2 project3

project1: project1.cpp
	$(CXX) $(CXXFLAGS) -o project1 project1.cpp

project2: project2.cpp
	$(CXX) $(CXXFLAGS) -o project2 project2.cpp

project3: project3.cpp
	$(CXX) $(CXXFLAGS) -o project3 project3.cpp

clean:
	rm -f project1 project2 project3
