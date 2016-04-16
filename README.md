# cpp-tmsp
C++ library for Tendermint Socket Protocol (TMSP) of [Tendermint](http://www.tendermint.com) fame.

Look here for an overview of TMSP along with some tutorials: http://tendermint.com/posts/tendermint-socket-protocol/


## Getting started

1. Do a `git clone --recursive https://github.com/mdyring/cpp-tmsp.git`
2. Run `./build.sh` or `build.cmd` on Windows
3. Try the freshly baked counter demo application from the deployment folder: `./app-counter 127.0.0.1 46658`
4. Tinker with the application using `tmsp-cli` - see [this tutorial](http://tendermint.com/tutorials/run-your-first-tmsp-application/) for inspiration


## Dependencies
cpp-tmsp has a couple of dependencies:

* [The Boost C++ Libraries](https://github.com/boostorg/boost)
* [Google Protocol Buffers](https://github.com/google/protobuf)
* [Tendermint TMSP](https://github.com/tendermint/tmsp) - just the protobuf spec. is required

These have been added as submodules and will be cloned and built using the steps above.
