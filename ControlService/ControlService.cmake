cmake_minimum_required(VERSION 3.2)

project(ControllerService)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_compile_options(-Wall)
find_package(Poco REQUIRED Foundation Util)
find_package(OpenSSL REQUIRED)
find_library(version version.dll)
set(SRC ControlService/ControlService.cpp ControlService/main.cpp ../Logger.cpp)
set(HEADERS ControlService/ControlService.h ControlService/CRC32.h ../Logger.h)

add_executable(ControlService ${SRC} ${HEADERS})
target_sources(ControlService PRIVATE ControlService/ControlService.rc)
target_link_libraries(ControlService PRIVATE Poco::Foundation Poco::Util version)