cmake_minimum_required(VERSION 3.22.3)

project(ControllerService)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_compile_options(-Wall)
find_package(Poco REQUIRED Foundation Util JSON)
find_package(OpenSSL REQUIRED)
find_library(version version.dll)
set(VERSION \"0.0.1\")
configure_file("${CMAKE_CURRENT_LIST_DIR}/version.json.in" "${CMAKE_CURRENT_LIST_DIR}/${PROJECT_NAME}_version.json")
set(SRC ControlService/ControlService.cpp ControlService/main.cpp)
set(HEADERS ControlService/Control.h ControlService/ControlService.h ControlService/ControlController.h ControlService/CRC32.h ../Logger.h)

add_executable(ControlService ${SRC} ${HEADERS})
#target_sources(ControlService PRIVATE ControlService/ControlService.rc)
target_link_libraries(ControlService PRIVATE Poco::Foundation Poco::Util Poco::JSON version)