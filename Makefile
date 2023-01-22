.PHONY: build clean gcovclean

build:
	cmake -B build -S .
	cmake --build build

clean:
	rm -rf build/
	rm -rf test/build

gcovclean:
	rm test/*.gcov
