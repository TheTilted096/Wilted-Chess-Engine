EXE := Wilted-0-3-1-0
#no evalfile yet
ARCH := native
SOURCES := src/*.cpp

CXX := clang++

CXXFLAGS := -std=c++20 -O3 -march=$(ARCH) -static -Wall -g
#-fconstexpr-steps=...

SUFFIX :=

ifeq ($(OS), Windows_NT)
	SUFFIX := .exe
endif

OUT := $(EXE)$(SUFFIX)

# One-Shot Compilation, for OpenBench

$(EXE):
	$(CXX) $(CXXFLAGS) -o $(OUT) $(SOURCES)