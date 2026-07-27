cmake_minimum_required(VERSION 3.10)
project(card_reader_cr1)
set(CMAKE_CXX_STANDARD 20)


find_package(gRPC CONFIG REQUIRED)

add_subdirectory(rpc_card)
add_subdirectory(rpc_main)
add_subdirectory(client_test)
add_subdirectory(httplib)
add_subdirectory(src)
add_subdirectory(src_test)