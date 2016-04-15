#!/bin/bash
JOBS=2
DEPENDENCIES_PREFIX="$PWD/dependencies"
BOOST_PREFIX="$DEPENDENCIES_PREFIX/boost"
PROTOBUF_PREFIX="$DEPENDENCIES_PREFIX/protobuf"
TMSP_PREFIX="$DEPENDENCIES_PREFIX/tmsp"
INSTALL_PREFIX="$DEPENDENCIES_PREFIX/install"

if ! [ -d "$INSTALL_PREFIX" ]
then
        pushd "$BOOST_PREFIX"
        ./bootstrap.sh --prefix="$INSTALL_PREFIX"
		./b2 headers
        ./b2 -j$JOBS cxxflags=-std=c++11 link=static define=NO_COMPRESSION install
        popd

        pushd "$PROTOBUF_PREFIX/cmake"
        cmake -DCMAKE_INSTALL_PREFIX="$INSTALL_PREFIX" -DCMAKE_BUILD_TYPE=Release -Dprotobuf_BUILD_TESTS=OFF .
        make install
        popd

        mkdir "$INSTALL_PREFIX/include/tmsp"
        pushd "$TMSP_PREFIX/types"
        "$INSTALL_PREFIX/bin/protoc" --cpp_out="$INSTALL_PREFIX/include/tmsp" types.proto
        popd
fi

$BOOST_PREFIX/b2 -sBOOST_ROOT="$BOOST_PREFIX" -j$JOBS variant=release $@