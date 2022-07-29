mkdir build
cd build
cmake ..  -D CMAKE_C_COMPILER=C:/msys64/mingw64/bin/gcc.exe -D CMAKE_CXX_COMPILER=C:/msys64/mingw64/bin/g++.exe -G "MinGW Makefiles" 
cmake --build .
pause