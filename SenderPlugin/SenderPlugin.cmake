cmake_minimum_required(VERSION 3.2)

project(SenderPlugin)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(NPCAP_ROOT "C:/npcap-sdk-1.13")

add_compile_options(-Wall)

find_package(Poco REQUIRED JSON Net Foundation Util NetSSL)
find_package(OpenSSL REQUIRED)
set ( CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON )	

set(LIBRARY_SOURCE SenderPlugin/SenderPlugin.cpp SenderPlugin/ISender.cpp SenderPlugin/ISender.h)

add_library(Sender SHARED ${LIBRARY_SOURCE})
target_link_libraries(Sender PRIVATE Poco::Foundation Poco::JSON Poco::Util Poco::Net Poco::NetSSL "${NPCAP_ROOT}/Lib/x64/Packet.lib" "${NPCAP_ROOT}/Lib/x64/wpcap.lib")
target_include_directories(Sender PRIVATE "${NPCAP_ROOT}/include")
