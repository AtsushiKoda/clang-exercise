.PHONY: build clean gcovclean test

build:
	mkdir -p build
	cmake -S . -B build
	make -C build

test:
	mkdir -p test/build
	cmake -S test -B test/build
	make -C test/build
	test/build/webclienttest

clean:
	rm -rf build/
	rm -rf test/build

gcovclean:
	rm test/*.gcov
