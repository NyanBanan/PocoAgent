cmake_minimum_required(VERSION 3.2)

project(PocoAgent)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(NPCAP_ROOT "C:/npcap-sdk-1.13")

add_compile_options(-Wall)
include("${CMAKE_CURRENT_LIST_DIR}/SenderPlugin/SenderPlugin.cmake")
find_package(Poco REQUIRED JSON Net Foundation Util NetSSL)
find_package(OpenSSL REQUIRED)
link_directories(${CMAKE_BINARY_DIR})
set(SRC PocoAgent.cpp ServerTasks.cpp Logger.cpp)
set(HEADERS PocoAgent.h ServerTasks.h Logger.h)
#add_library(libSender SHARED IMPORTED)
add_executable(PocoAgent ${SRC} ${HEADERS})
target_link_libraries(PocoAgent PRIVATE Poco::Foundation Poco::JSON Poco::Util Poco::Net Poco::NetSSL libSender) 



