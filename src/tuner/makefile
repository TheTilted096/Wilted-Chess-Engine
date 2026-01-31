EXE := WiltedTuna-1-1-0

ARCH := native

SOURCES := Guesser.cpp Tuner.cpp

CXX := clang++

CXXFLAGS := -std=c++20 -O3 -march=$(ARCH) -pthread -static -Wall -g

ifeq ($(OS), Windows_NT)
	SUFFIX := .exe
endif

OUT := $(EXE)$(SUFFIX)

$(EXE):
	$(CXX) $(CXXFLAGS) -o $(OUT) $(SOURCES)

clean: # Works on Windows
	del *.exe;