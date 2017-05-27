BUILDTYPE ?= Release

all: bench test

build/Makefile:
	deps/run_gyp shelf-pack.gyp --depth=. -Goutput_dir=. --generator-output=./build -f make

xcode:
	deps/run_gyp shelf-pack.gyp --depth=. -Goutput_dir=. --generator-output=./build -f xcode
	open build/shelf-pack.xcodeproj

bench: build/Makefile
	BUILDTYPE=Debug make -C build bench

test: build/Makefile
	BUILDTYPE=Debug make -C build test
	build/Debug/test

runbench:
	build/Debug/bench

clean:
	-rm -rf build
