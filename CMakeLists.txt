cmake_minimum_required(VERSION 2.8)

project(boost-protobuf)

set(CMAKE_CXX_STANDARD 11)

INCLUDE_DIRECTORIES(/usr/local/include)
LINK_LIBRARIES(/usr/local/lib)

find_package(Boost 1.67 COMPONENTS system filesystem regex thread REQUIRED)

add_executable(${PROJECT_NAME}-client "main.cpp" "Transport.cpp" "Protocol.cpp" "BaseProtocol.cpp" "BaseFactory.cpp" "ClientFactory.cpp")
target_link_libraries(${PROJECT_NAME}-client ${Boost_LIBRARIES} pthread)

add_library(${PROJECT_NAME}-shared SHARED "Transport.cpp" "Protocol.cpp" "BaseProtocol.cpp" "BaseFactory.cpp" "ClientFactory.cpp")
set_target_properties(${PROJECT_NAME}-shared PROPERTIES OUTPUT_NAME ${PROJECT_NAME})
target_link_libraries(${PROJECT_NAME}-shared ${Boost_LIBRARIES} pthread)
