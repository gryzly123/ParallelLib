#MINT INSTALL - only executed once
if [ -e bin/ ]
then
    echo "ParallelLib prerequisites already installed. Building."
else
    #install required packages (GCC, C++)
    sudo add-apt-repository ppa:jonathonf/gcc
    sudo apt-get update
    sudo apt install g++

    #setup tbb dynamic libraries
    sudo cp ../ExternalLibraries/tbb-2020.0-lin/tbb/lib/intel64/gcc4.8/libtbb.so.2 /usr/lib/
    sudo cp ../ExternalLibraries/tbb-2020.0-lin/tbb/lib/intel64/gcc4.8/libtbbmalloc.so.2 /usr/lib/
    sudo cp ../ExternalLibraries/tbb-2020.0-lin/tbb/lib/intel64/gcc4.8/libtbbmalloc_proxy.so.2 /usr/lib/

    #create buildfolder that also marks prerequisites as installed
    mkdir bin
    echo "ParallelLib prerequisites have been installed. Starting build."
fi

#PARALLEL LIB
#compile as static library
mv ParallelLib/main.skip ParallelLib/main.cpp
c++ ParallelLib/*.cpp -lpthread -o bin/static_parallellib.o
ar rcs bin/libParallelLib.a bin/static_parallellib.o
ranlib bin/libParallelLib.a
#rm bin/static_parallellib.o

#TEST SUITE
mv ParallelLib/main.cpp ParallelLib/main.skip

#compile as executable
c++ TestSuite/*.cpp                                 \
-lpthread                                           \
-fopenmp                                            \
ParallelLib/*.cpp                                   \
-I ../ParallelLib/                                  \
-I ../ExternalLibraries/dlib/                       \
-I ../ExternalLibraries/tbb-2020.0-lin/tbb/include/ \
../ExternalLibraries/tbb-2020.0-lin/tbb/lib/intel64/gcc4.8/libtbb.so.2 \
../ExternalLibraries/tbb-2020.0-lin/tbb/lib/intel64/gcc4.8/libtbbmalloc.so.2 \
../ExternalLibraries/tbb-2020.0-lin/tbb/lib/intel64/gcc4.8/libtbbmalloc_proxy.so.2 \
-o bin/exeTestSuite

mv ParallelLib/main.skip ParallelLib/main.cpp

#-Xlinker -rpath -Xlinker $LD_LIBRARY_PATH -L $LD_LIBRARY_PATH  
#../ExternalLibraries/tbb-2020.0-lin/tbb/lib/intel64/gcc4.8/libtbb_debug.so.2 \
#../ExternalLibraries/tbb-2020.0-lin/tbb/lib/intel64/gcc4.8/libtbb_preview.so.2 \
#../ExternalLibraries/tbb-2020.0-lin/tbb/lib/intel64/gcc4.8/libtbb_preview_debug.so.2 \
#../ExternalLibraries/tbb-2020.0-lin/tbb/lib/intel64/gcc4.8/libtbbbind.so.2 \
#../ExternalLibraries/tbb-2020.0-lin/tbb/lib/intel64/gcc4.8/libtbbbind_debug.so.2 \
#../ExternalLibraries/tbb-2020.0-lin/tbb/lib/intel64/gcc4.8/libtbbmalloc_debug.so.2 \
#../ExternalLibraries/tbb-2020.0-lin/tbb/lib/intel64/gcc4.8/libtbbmalloc_proxy_debug.so.2 \
#bin/final/libParallelLib.a                         \
#../ExternalLibraries/tbb-2020.0-lin/tbb/lib/intel64/gcc4.8/* \


