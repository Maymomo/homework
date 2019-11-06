cmake_minimum_required(VERSION 2.8)

project(homework CXX C)
set (CMAKE_CXX_STANDARD 11)
add_subdirectory(libevent)

include_directories("${CMAKE_CURRENT_BINARY_DIR}/libevent/include")
include_directories(libevent/include)

message(STATUS "${CMAKE_CURRENT_BINARY_DIR}/libevent/include")

set(homework_server_src 
    Conn.h
    Conn.cc
    Server.h
    Server.cc
    Parser.h
    Parser.cc
    ServerMain.cc
)

set(homework_client_src
    Client.h
    Client.cc
    Parser.h
    Parser.cc
    ClientMain.cc
)

add_executable(server ${homework_server_src})
target_link_libraries(server event)

add_executable(client ${homework_client_src})
target_link_libraries(client event)