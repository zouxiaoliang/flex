cmake_minimum_required(VERSION 2.8)

project(flexible-network)

set(CMAKE_CXX_STANDARD 11)

option(debug "build debug version" ON)
set(CMAKE_C_FLAGS "-fPIC -Wall")
set(CMAKE_CXX_FLAGS "-fPIC -Wall")
if (debug)
    set(CMAKE_CXX_FLAGS " -g3 -ggdb")
endif()

INCLUDE_DIRECTORIES(/usr/local/include)
LINK_LIBRARIES(/usr/local/lib)

INCLUDE_DIRECTORIES(.)

find_package(Boost 1.67 COMPONENTS system filesystem regex thread)

add_executable(${PROJECT_NAME}-client "main.cpp" "transport/Transport.cpp" "protocol/Protocol.cpp" "protocol/BaseProtocol.cpp" "factory/BaseFactory.cpp" "factory/ClientFactory.cpp" "BaseAcceptor.cpp")
if (APPLE)
    target_link_libraries(${PROJECT_NAME}-client ${Boost_LIBRARIES} pthread)
else (APPLE)
    target_link_libraries(${PROJECT_NAME}-client ${Boost_LIBRARIES} pthread rt)
endif(APPLE)

add_library(${PROJECT_NAME}-shared SHARED "transport/Transport.cpp" "protocol/Protocol.cpp" "protocol/BaseProtocol.cpp" "factory/BaseFactory.cpp" "factory/ClientFactory.cpp" "BaseAcceptor.cpp")
set_target_properties(${PROJECT_NAME}-shared PROPERTIES OUTPUT_NAME ${PROJECT_NAME})

if (APPLE)
    target_link_libraries(${PROJECT_NAME}-shared ${Boost_LIBRARIES} pthread)
else (APPLE)
    target_link_libraries(${PROJECT_NAME}-shared ${Boost_LIBRARIES} pthread rt)
endif(APPLE)

SET(LIBRARY_OUTPUT_PATH lib)
install(TARGETS ${PROJECT_NAME}-shared LIBRARY DESTINATION lib)
install(TARGETS ${PROJECT_NAME}-client RUNTIME DESTINATION .)
