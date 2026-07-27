# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Release")
  file(REMOVE_RECURSE
  "client_test\\CMakeFiles\\testui_autogen.dir\\AutogenUsed.txt"
  "client_test\\CMakeFiles\\testui_autogen.dir\\ParseCache.txt"
  "client_test\\testui_autogen"
  )
endif()
