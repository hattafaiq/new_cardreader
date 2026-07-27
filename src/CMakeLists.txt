

add_executable(Card_reader_cr1_card_reader_cr1 main.cpp )

add_library(card_crt_lib 
    serial_command_cr.h 
    serial_command_cr.cpp 
)

target_include_directories(card_crt_lib PUBLIC 
${CMAKE_CURRENT_SOURCE_DIR}/../libs
)

target_link_libraries(card_crt_lib PUBLIC 
${CMAKE_CURRENT_SOURCE_DIR}/../libs/CRT_310_NR01.lib
)


target_include_directories( Card_reader_cr1_card_reader_cr1 PRIVATE 
${CMAKE_CURRENT_SOURCE_DIR}/../libs/rapidjson-master/include 
)

target_link_libraries(Card_reader_cr1_card_reader_cr1 PRIVATE 
    card_crt_lib
    )