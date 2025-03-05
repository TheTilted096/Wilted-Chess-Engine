# makefile

debug:
	clang++ -O2 -Wall -static -g -march=native -o wtest WiltedDriver.cpp

auto:
	clang++ -O2 -Wall -static -g -march=native -o wauto WiltedAutoplay.cpp

latest:
	clang++ -O2 -Wall -static -g -march=native -o cwilted-pre-pre7 WiltedDriver.cpp

# debug:
#clang++ -O2 -Wall -static -g -march=native -o stilted-28-test STiltedDriverV3.cpp STiltedMoveGenV5.cpp STiltedSearchEvalV4.cpp
