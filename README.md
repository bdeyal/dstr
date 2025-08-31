# dstr
dynamic C strings, similar to C++ std::string

# Build
mkdir ./build
cd ./build
cmake .. -DCMAKE_C_COMPILER=gcc -DCMAKE_CXX_COMPILER=g++
make

# Clean
make clean

# Test
## Linux
make test
or:
./dstrtest
