CXX			:= g++
CXXFLAGS 	:= -std=c++17 -I include

all: build

obj/%.o: src/%.cpp
	$(MAKE) dirs
	$(CXX) $(CXXFLAGS) -c $^ -o $@

.PHONY: dirs clean lint

dirs:
	mkdir -p obj bin

build: $(patsubst src/%.cpp, obj/%.o ,$(wildcard src/*.cpp))
	$(CXX) $(CXXFLAGS) $^ -o bin/exec

clean:
	rm -rf obj bin

lint:
	clang-tidy src/*.cpp -- -I./include