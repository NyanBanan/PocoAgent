cmake_minimum_required(VERSION 3.22.3)

project(PocoAgent)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_compile_options(-Wall)
find_package(Poco REQUIRED JSON Net Foundation Util NetSSL)
find_package(OpenSSL REQUIRED)
link_directories(${CMAKE_BINARY_DIR})
set(VERSION \"0.0.1\")
configure_file("${PROJECT_SOURCE_DIR}/AgentService/version.json.in" "${PROJECT_SOURCE_DIR}/AgentService/${PROJECT_NAME}_version.json")
set(SRC AgentService/PocoAgent.cpp ../Logger.cpp AgentService/ServerTasks.cpp AgentService/RESTInter.cpp AgentService/main.cpp)
set(HEADERS AgentService/PocoAgent.h AgentService/ServerTasks.h ../Logger.h AgentService/RESTInter.h AgentService/LibraryLoader.h)

add_executable(PocoAgent ${SRC} ${HEADERS} )
#target_sources(PocoAgent PRIVATE AgentService/AgentService.rc)
target_link_libraries(PocoAgent PRIVATE Poco::Foundation Poco::JSON Poco::Util Poco::Net Poco::NetSSL libPlugin)