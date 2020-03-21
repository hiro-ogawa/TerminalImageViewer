all: build/tiv

build/tiv: build/Makefile
	make -C build

build/Makefile: CMakeLists.txt build
	cd build; cmake ..

build:
	mkdir -p build

clean:
	rm -rf build

.PHONY: all clean
