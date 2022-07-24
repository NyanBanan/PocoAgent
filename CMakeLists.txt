cmake_minimum_required(VERSION 3.2)

project(PocoAgent)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_compile_options(-Wall)

find_package(Poco REQUIRED JSON Net Foundation Util NetSSL)
find_package(OpenSSL REQUIRED)

set(SRC PocoAgent.cpp Requester.cpp)
set(HEADERS PocoAgent.h Requester.h FunctionErrorsVariables.h)

add_executable(PocoAgent ${SRC} ${HEADERS})
add_library(Sender SHARED SenderImpl.cpp ISender.cpp)
target_link_libraries(PocoAgent PUBLIC Poco::Foundation Poco::JSON Poco::Util Poco::Net Poco::NetSSL) 
target_link_libraries(Sender PUBLIC Poco::Foundation Poco::JSON Poco::Util Poco::Net Poco::NetSSL) 