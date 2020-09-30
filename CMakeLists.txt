cmake_minimum_required(VERSION 2.8)

project(flexible-network)

set(CMAKE_CXX_STANDARD 11)

option(debug "build debug version" ON)

if (UNIX)
    set(CMAKE_C_FLAGS "-fPIC -Wall -pthread")
    set(CMAKE_CXX_FLAGS "-fPIC -Wall -pthread -Wno-unused")
    if (debug)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g3 -ggdb")
    else()
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Os")
    endif()
endif()

message(STATUS "${PROJECT_NAME} cmake_cxx_flags: ${CMAKE_CXX_FLAGS}")

if (CMAKE_SYSTEM_NAME MATCHES "Linux")
    INCLUDE_DIRECTORIES(/usr/local/include)
    LINK_LIBRARIES(/usr/local/lib)
endif()

INCLUDE_DIRECTORIES(.)

if(CMAKE_SYSTEM_NAME MATCHES "Windows")
    find_package(Boost COMPONENTS system filesystem regex thread)
    INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIRS})
else()
    find_package(Boost 1.67 COMPONENTS system filesystem regex thread)
endif()

set (TRANSPORT "transport/BaseTransport.cpp" "transport/TcpTransport.cpp")
if (UNIX)
    set (TRANSPORT ${TRANSPORT} "transport/UnixSocketTransport.cpp")
endif()

#aux_source_directory(transport TRANSPORT)
aux_source_directory(protocol PROTOCOL)
aux_source_directory(factory FACTORY)
aux_source_directory(listener LISTENER)

add_executable(${PROJECT_NAME}-client "main.cpp" ${TRANSPORT} ${PROTOCOL} ${FACTORY} ${LISTENER})

if (UNIX)
    if (APPLE)
        target_link_libraries(${PROJECT_NAME}-client ${Boost_LIBRARIES} pthread)
    else (APPLE)
        target_link_libraries(${PROJECT_NAME}-client ${Boost_LIBRARIES} pthread rt)
    endif (APPLE)
else()
    target_link_libraries(${PROJECT_NAME}-client ${Boost_LIBRARIES})
endif(UNIX)

add_library(${PROJECT_NAME}-shared SHARED ${TRANSPORT} ${PROTOCOL} ${FACTORY} ${LISTENER})

set_target_properties(${PROJECT_NAME}-shared PROPERTIES OUTPUT_NAME ${PROJECT_NAME})

if (UNIX)
    if (APPLE)
        target_link_libraries(${PROJECT_NAME}-shared ${Boost_LIBRARIES} pthread)
    else (APPLE)
        target_link_libraries(${PROJECT_NAME}-shared ${Boost_LIBRARIES} pthread rt)
    endif(APPLE)
else()
    target_link_libraries(${PROJECT_NAME}-shared ${Boost_LIBRARIES})
endif(UNIX)

SET(LIBRARY_OUTPUT_PATH lib)
install(TARGETS ${PROJECT_NAME}-shared LIBRARY DESTINATION lib)
install(TARGETS ${PROJECT_NAME}-client RUNTIME DESTINATION .)
