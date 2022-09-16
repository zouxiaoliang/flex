cmake_minimum_required(VERSION 2.8.12)

project(flex-cmake LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 11)

option(debug "build debug version" ON)
option(build_test "build test projects" ON)

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

function(flexMain)

    if (NOT HAS_THIRDPARTY_BOOST)
        message(FATAL_ERROR "you need import 'boost-cmake' project from github, cmd `git clone xxx`")
    endif()

    set (FACTORY
        "factory/Connector.cpp"
        "factory/AutoConnector.cpp"
        )
    set (PROTOCOL
        "protocol/BaseProtocol.cpp"
        "protocol/GenericProtocol.cpp"
        )
    set (TRANSPORT
        "transport/BaseTransport.cpp"
        "transport/TcpTransport.cpp"
        "transport/Strategy.cpp"
        "transport/SslTransport.cpp"
        )
    set(LISTENER "listener/Listener.cpp")

    if (UNIX)
        set (TRANSPORT ${TRANSPORT} "transport/UnixSocketTransport.cpp")
        if (CMAKE_SYSTEM_NAME MATCHES "Linux")
            set (TRANSPORT ${TRANSPORT} "transport/NetlinkTransport.cpp")
        endif()
    endif()

    add_library(thirdparty_libflex INTERFACE)
    add_library(thirdparty_libflex_static ${TRANSPORT} ${PROTOCOL} ${FACTORY} ${LISTENER})

    if (UNIX)
        if (APPLE)
            target_link_libraries(thirdparty_libflex_static thirdparty_boost pthread)
        else (APPLE)
            target_link_libraries(thirdparty_libflex_static thirdparty_boost pthread rt)
        endif(APPLE)
    endif(UNIX)
    target_include_directories(thirdparty_libflex_static PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
    target_link_libraries(thirdparty_libflex_static thirdparty_boost)

    target_link_libraries(thirdparty_libflex INTERFACE thirdparty_libflex_static)
    target_include_directories(thirdparty_libflex SYSTEM INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

endfunction()

function(testMain)
    add_executable(flex-test main.cpp)
    target_link_libraries(flex-test thirdparty_libflex thirdparty_boost)
    if (CMAKE_SYSTEM_NAME MATCHES "Linux")
        target_link_libraries(flex-test libunwind.a libc++abi.a dl rt)
    endif()
endfunction()

flexMain()
if (build_test)
    testMain()
endif()
