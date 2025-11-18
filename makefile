EXE := Wilted-0-9-3-1
#no evalfile yet
ARCH := native
SOURCES := src/*.cpp

CXX := clang++

CXXFLAGS := -std=c++20 -O3 -march=$(ARCH) -Wall -g
#-fconstexpr-steps=...

SUFFIX :=

ifeq ($(OS), Windows_NT)
	SUFFIX := .exe
endif

OUT := $(EXE)$(SUFFIX)

# One-Shot Compilation, for OpenBench

$(EXE):
	$(CXX) $(CXXFLAGS) -o $(OUT) $(SOURCES)