find_package(CURL CONFIG REQUIRED)

add_library(httplib STATIC
    networkcurl.cpp 
    clienthttp.cpp
    clienthttp.h 
    networkcurl.h
)

target_link_libraries(httplib PUBLIC
  CURL::libcurl
  )

