
set(GRPC_SRC_DEPS
	card.grpc.pb.cc
	card.grpc.pb.h
	card.pb.cc
	card.pb.h
)

add_library(card_crt STATIC ${GRPC_SRC_DEPS} )

target_include_directories(card_crt PUBLIC
    ${ZLIB_INCLUDE_DIR}
)
target_compile_definitions(card_crt PUBLIC PROTOBUF_USE_DLLS)

target_include_directories(
    card_crt
    PRIVATE
    ${GRPC_INCLUDE_DIR}
    ${Protobuf_INCLUDE_DIRS}
)

target_link_libraries(card_crt PUBLIC
    gRPC::gpr gRPC::grpc gRPC::grpc++ gRPC::grpc++_alts 
)