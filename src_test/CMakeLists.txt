# Memaksa compiler menggunakan standar C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(Card_readerTest main.cpp )

add_library(card_crt_lib_aa
    serial_command_cr.h 
    serial_command_cr.cpp 
)

target_include_directories(card_crt_lib_aa PUBLIC 
${CMAKE_CURRENT_SOURCE_DIR}/../libs
)

target_link_libraries(card_crt_lib_aa PUBLIC 
${CMAKE_CURRENT_SOURCE_DIR}/../libs/CRT_310_NR01.lib
)


target_include_directories( Card_readerTest PRIVATE 
${CMAKE_CURRENT_SOURCE_DIR}/../libs/rapidjson-master/include 
)

target_link_libraries(Card_readerTest PRIVATE 
    card_crt_lib
    )