#!/bin/bash
JOBS=2
DEPENDENCIES_PREFIX="$PWD/dependencies"
INSTALL_PREFIX="$DEPENDENCIES_PREFIX/install"
BOOST_PREFIX="$DEPENDENCIES_PREFIX/boost"
PROTOBUF_PREFIX="$DEPENDENCIES_PREFIX/protobuf"
TMSP_PREFIX="$DEPENDENCIES_PREFIX/tmsp"

if [ "$TRAVIS_OS_NAME" = "osx" ] && [ "$CC" = "gcc" ]
then
	TOOLSET=toolset=darwin
else
	TOOLSET=toolset=$CC
fi

if ! [ -d "$INSTALL_PREFIX" ]
then
	pushd "$BOOST_PREFIX"
	./bootstrap.sh --prefix="$INSTALL_PREFIX" --with-libraries=log,system,thread
	./b2 -d0 headers
	./b2 -d0 -j$JOBS $TOOLSET cxxflags=-std=c++11 link=static install
	popd

	pushd "$PROTOBUF_PREFIX/cmake"
	cmake -DCMAKE_INSTALL_PREFIX="$INSTALL_PREFIX" -DCMAKE_BUILD_TYPE=Release -Dprotobuf_BUILD_TESTS=OFF .
	make -j $JOBS -s install
	popd

	mkdir "$INSTALL_PREFIX/include/tmsp"
	pushd "$TMSP_PREFIX/types"
	"$INSTALL_PREFIX/bin/protoc" --cpp_out="$INSTALL_PREFIX/include/tmsp" types.proto
	popd
fi

$BOOST_PREFIX/b2 -sBOOST_ROOT="$BOOST_PREFIX" -j$JOBS $TOOLSET cxxflags=-std=c++11 variant=release $@
