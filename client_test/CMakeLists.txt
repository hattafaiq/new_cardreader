#find_package(QT COMPONENTS Core NAMES Qt6 Qt5 )
find_package (Qt5 COMPONENTS Widgets )

if(Qt5_FOUND)
find_package(gRPC CONFIG REQUIRED)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

add_executable(testui 
    main.cpp 
    cardreader.cpp 
    cardreader.h
)

target_include_directories(testui PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/../rpc_card
)

target_link_libraries(testui 
       Qt${QT_VERSION_MAJOR}::Widgets
       
    gRPC::gpr gRPC::grpc gRPC::grpc++ gRPC::grpc++_alts 
       card_crt
       
)
endif()