.PHONY: clean gcovclean

clean:
	rm -rf build/
	rm -rf test/build

gcovclean:
	rm test/*.gcov
