CXX = clang++
CXXFLAGS += -Wall -Wextra -std=c++17 -O2
LDFLAGS += -lpthread

OBJS = main.o solver.o parser.o

all: solver

clean:
	rm -f *.o
	rm -f solver

%.o: %.cc
	$(CXX) $(CXXFLAGS) -c $< -o $@

solver: $(OBJS)
	$(CXX) -o solver $(OBJS) $(LDFLAGS)

.PHONY: all

