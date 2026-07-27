# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "release")
  file(REMOVE_RECURSE
  "CMakeFiles\\testui_autogen.dir\\AutogenUsed.txt"
  "CMakeFiles\\testui_autogen.dir\\ParseCache.txt"
  "testui_autogen"
  )
endif()
