#MINT INSTALL - only executed once

BEGIN_BOLD='\e[1;93m'
END_BOLD='\e[0m'

if [ -e bin/ ]
then
    echo -e "${BEGIN_BOLD}ParallelLib prerequisites already installed. Building.${END_BOLD}"
else
    echo -e "${BEGIN_BOLD}ParallelLib prerequisites not found. Installing GCC and TBB SOs.${END_BOLD}"
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
    echo -e "${BEGIN_BOLD}ParallelLib prerequisites have been installed. Starting build.${END_BOLD}"
fi

#PARALLEL LIB
#compile as static library
echo -e "${BEGIN_BOLD}Building ParallelLib as a shared library (.so).${END_BOLD}"
c++ ParallelLib/*.cpp \
-lpthread             \
-fPIC                 \
-shared               \
-O3                   \
-o bin/libParallelLib.so
echo -e "${BEGIN_BOLD}Installing ParallelLib to /usr/lib.${END_BOLD}"
sudo cp bin/libParallelLib.so /usr/lib

#TEST SUITE
#compile as executable
echo -e "${BEGIN_BOLD}Building TestSuite as an unoptimized executable.${END_BOLD}"
c++ TestSuite/*.cpp                                 \
/usr/lib/libParallelLib.so                          \
/usr/lib/libtbb.so.2                                \
/usr/lib/libtbbmalloc.so.2                          \
/usr/lib/libtbbmalloc_proxy.so.2                    \
-lpthread                                           \
-I ../ParallelLib/                                  \
-I ../ExternalLibraries/dlib/                       \
-I ../ExternalLibraries/tbb-2020.0-lin/tbb/include/ \
-fopenmp                                            \
-o bin/0-TestSuite

echo -e "${BEGIN_BOLD}Building TestSuite as an O3 executable.${END_BOLD}"
c++ TestSuite/*.cpp                                 \
/usr/lib/libParallelLib.so                          \
/usr/lib/libtbb.so.2                                \
/usr/lib/libtbbmalloc.so.2                          \
/usr/lib/libtbbmalloc_proxy.so.2                    \
-lpthread                                           \
-I ../ParallelLib/                                  \
-I ../ExternalLibraries/dlib/                       \
-I ../ExternalLibraries/tbb-2020.0-lin/tbb/include/ \
-fopenmp                                            \
-O2                                                 \
-o bin/2-TestSuite

echo -e "${BEGIN_BOLD}Running TestSuite O0.${END_BOLD}"
mkdir result0
cd result0
sudo ./../bin/0-TestSuite
cd ..

echo -e "${BEGIN_BOLD}Running TestSuite O2.${END_BOLD}"
mkdir result3
cd result3
sudo ./../bin/2-TestSuite
cd ..

echo -e "${BEGIN_BOLD}Script completed. Check results in directories 'result0' and 'result3'.${END_BOLD}"

