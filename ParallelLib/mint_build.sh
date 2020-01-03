##install required packages - you only need to do this once

#sudo add-apt-repository ppa:jonathonf/gcc
#sudo apt-get update
#sudo apt install g++

##if windows chooses wrong file encoding, you can revert to ASCII by using
#sudo apt install enca
#enconv stdafx.h -L polish -x ASCII

mkdir bin
mkdir bin/static
mkdir bin/final
c++ ParallelLib/*.cpp -lpthread -o bin/static/parallellib.o
ar rcs bin/final/libParallelLib.a bin/static/parallellib.o
ranlib bin/final/libParallelLib.a 

mkdir testbin
c++ TestSuite/*.cpp -I ParallelLib/ -I bin/final -o testbin/test.lexe

