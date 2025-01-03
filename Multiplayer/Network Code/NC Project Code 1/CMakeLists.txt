cmake_minimum_required(VERSION "3.30.2")

project("NC Project Code 1")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(main main.cpp ChatObject.cpp ChatObject.h networkcodeTCP.cpp networkcodeTCP.h)
target_link_libraries(main ws2_32)