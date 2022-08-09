cmake_minimum_required(VERSION 3.22.3)

project(PocoAgent)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_compile_options(-Wall)
include("${CMAKE_CURRENT_LIST_DIR}/Plugins/plugin/Plugin.cmake" REQUIRED)
include("${CMAKE_CURRENT_LIST_DIR}/AgentService/AgentService.cmake" REQUIRED)
include("${CMAKE_CURRENT_LIST_DIR}/ControlService/ControlService.cmake" REQUIRED)


