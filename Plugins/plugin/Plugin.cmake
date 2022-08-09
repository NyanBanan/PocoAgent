cmake_minimum_required(VERSION 3.22.3)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_compile_options(-Wall)

project(plugin CXX)

find_package(Poco REQUIRED Foundation Util)

set( SOURCE_LIB
        Plugins/plugin/PluginLibrary.cpp
        Plugins/AbstractPlugin.cpp
        Plugins/plugin/PcapMethods.cpp
        Plugins/plugin/PcapStatistics.cpp
        ../TaskFactory.cpp
        Plugins/AbstractPlugin.h
        Plugins/plugin/PcapMethods.h
        Plugins/plugin/PcapStatistics.h
        ../Logger.h
        ../TaskFactory.h
)	

set ( CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON )	

include_directories("C:/msys64/mingw64/include")
include_directories("C:/npcap-1/include")

add_library(Plugin SHARED ${SOURCE_LIB} )

target_link_libraries(Plugin PRIVATE Poco::Foundation Poco::Util)
target_link_libraries(Plugin PRIVATE "C:/npcap-1/Lib/x64/wpcap.lib")
target_link_libraries(Plugin PRIVATE "C:/npcap-1/Lib/x64/Packet.lib")