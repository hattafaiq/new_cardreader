project(rpc_card)

set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_executable(rpc_main 
    main.cpp 
    server.h 
    server.cpp
)

target_include_directories(rpc_main PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/../rpc_card
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${CMAKE_CURRENT_SOURCE_DIR}/../src
    ${CMAKE_CURRENT_SOURCE_DIR}/../httplib
)

target_link_libraries(rpc_main PRIVATE 
    gRPC::gpr gRPC::grpc gRPC::grpc++ gRPC::grpc++_alts 
    card_crt
    card_crt_lib
    httplib
)

install(FILES param.json DESTINATION bin\\card_reader )
install(TARGETS rpc_main
    LIBRARY DESTINATION bin\\card_reader
    RUNTIME DESTINATION bin\\card_reader)