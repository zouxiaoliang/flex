cmake_minimum_required(VERSION 2.8)

project(boost-protobuf-client)

set(CMAKE_CXX_STANDARD 11)

INCLUDE_DIRECTORIES(/usr/local/include)
LINK_LIBRARIES(/usr/local/lib)

find_package(Boost 1.67 COMPONENTS system filesystem regex thread REQUIRED)

add_executable(${PROJECT_NAME} "main.cpp" "Transport.cpp" "Protocol.cpp" "ClientFactory.cpp")
target_link_libraries(${PROJECT_NAME} ${Boost_LIBRARIES} pthread)
