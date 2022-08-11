mkdir build
cd build
mkdir Plugins
cmake ..  -D CMAKE_C_COMPILER=C:/msys64/mingw64/bin/gcc.exe -D CMAKE_CXX_COMPILER=C:/msys64/mingw64/bin/g++.exe -G "MinGW Makefiles" 
cmake --build .
cd /D C:
mkdir ngids
pause