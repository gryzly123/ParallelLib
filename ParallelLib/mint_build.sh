##install required packages - you only need to do this once

sudo add-apt-repository ppa:jonathonf/gcc
sudo apt-get update
sudo apt install g++

##if windows chooses wrong file encoding, you can revert to ASCII by using
#sudo apt install enca
#enconv stdafx.h -L polish -x ASCII

#setup tbb dynamic libraries
sudo mkdir /usr/local/lib/tbb
sudo cp ../ExternalLibraries/tbb-2020.0-lin/tbb/lib/intel64/gcc4.8/* /usr/local/lib/tbb/


#compile parallel lib
mkdir bin
mkdir bin/static
mkdir bin/final
c++ ParallelLib/*.cpp -lpthread -o bin/static/parallellib.o
ar rcs bin/final/libParallelLib.a bin/static/parallellib.o
ranlib bin/final/libParallelLib.a 


#compile test suite
mkdir testbin
c++ TestSuite/*.cpp                                 \
-lpthread                                           \
-fopenmp                                            \
ParallelLib/*.cpp                                   \
../ExternalLibraries/tbb-2020.0-lin/tbb/lib/intel64/gcc4.8/libtbb.so.2 \
../ExternalLibraries/tbb-2020.0-lin/tbb/lib/intel64/gcc4.8/libtbb_debug.so.2 \
../ExternalLibraries/tbb-2020.0-lin/tbb/lib/intel64/gcc4.8/libtbb_preview.so.2 \
../ExternalLibraries/tbb-2020.0-lin/tbb/lib/intel64/gcc4.8/libtbb_preview_debug.so.2 \
../ExternalLibraries/tbb-2020.0-lin/tbb/lib/intel64/gcc4.8/libtbbbind.so.2 \
../ExternalLibraries/tbb-2020.0-lin/tbb/lib/intel64/gcc4.8/libtbbbind_debug.so.2 \
../ExternalLibraries/tbb-2020.0-lin/tbb/lib/intel64/gcc4.8/libtbbmalloc.so.2 \
../ExternalLibraries/tbb-2020.0-lin/tbb/lib/intel64/gcc4.8/libtbbmalloc_debug.so.2 \
../ExternalLibraries/tbb-2020.0-lin/tbb/lib/intel64/gcc4.8/libtbbmalloc_proxy.so.2 \
../ExternalLibraries/tbb-2020.0-lin/tbb/lib/intel64/gcc4.8/libtbbmalloc_proxy_debug.so.2 \
-I ../ParallelLib/                                  \
-I ../ExternalLibraries/dlib/                       \
-I ../ExternalLibraries/tbb-2020.0-lin/tbb/include/ \
-o testbin/test.lexe
#bin/final/libParallelLib.a                         \
#../ExternalLibraries/tbb-2020.0-lin/tbb/lib/intel64/gcc4.8/* \


