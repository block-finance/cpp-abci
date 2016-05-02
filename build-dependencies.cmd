@echo off
set DEPENDENCIES_PREFIX=%cd%\dependencies
set BOOST_PREFIX=%DEPENDENCIES_PREFIX%\boost
set PROTOBUF_PREFIX=%DEPENDENCIES_PREFIX%\protobuf
set TMSP_PREFIX=%DEPENDENCIES_PREFIX%\tmsp
set INSTALL_PREFIX=%DEPENDENCIES_PREFIX%\install
call "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcvarsall.bat" amd64

if exist "%INSTALL_PREFIX%" (goto done)

pushd "%BOOST_PREFIX%"
call bootstrap.bat
b2 -d0 headers
set BOOST_LIBRARIES=--with-log --with-system --with-thread
b2 -d0 --prefix="%INSTALL_PREFIX%\debug" --layout=tagged %BOOST_LIBRARIES% address-model=64 link=static runtime-link=static variant=debug install
b2 -d0 --prefix="%INSTALL_PREFIX%\release" --layout=tagged %BOOST_LIBRARIES% address-model=64 link=static runtime-link=static variant=release install
popd

pushd "%PROTOBUF_PREFIX%\cmake"
if exist debug (rmdir /s /q debug)
mkdir debug
pushd debug
cmake -G "NMake Makefiles" -DCMAKE_INSTALL_PREFIX="%INSTALL_PREFIX%\debug" -DCMAKE_BUILD_TYPE=Debug -Dprotobuf_BUILD_TESTS=OFF ..
nmake install
popd
if exist release (rmdir /s /q release)
mkdir release
pushd release
cmake -G "NMake Makefiles" -DCMAKE_INSTALL_PREFIX="%INSTALL_PREFIX%\release" -DCMAKE_BUILD_TYPE=Release -Dprotobuf_BUILD_TESTS=OFF ..
nmake install
popd
popd

pushd "%TMSP_PREFIX%\types"
mkdir "%INSTALL_PREFIX%\debug\include\tmsp"
"%INSTALL_PREFIX%\release\bin\protoc" --cpp_out="%INSTALL_PREFIX%\debug\include\tmsp" types.proto
mkdir "%INSTALL_PREFIX%\release\include\tmsp"
"%INSTALL_PREFIX%\release\bin\protoc" --cpp_out="%INSTALL_PREFIX%\release\include\tmsp" types.proto
popd

:done