cmake_minimum_required(VERSION "3.30.2")

project("Minesweeper")

set(CMAKE_CXX_STANDARD 17) 
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)


SET(PRECOMPILE
    src/precomp.h
)

SET(HEADERS
    src/minesweeper.h
)

SET(CODEFILES
src/minesweeper.cpp
)

add_compile_options(
    -static 
    -static-libgcc 
    -static-libstdc++ 
    )
add_link_options(
    -static 
    )

add_executable(main src/main.cpp ${PRECOMPILE} ${HEADERS} ${CODEFILES})
target_link_libraries(main ws2_32 raylib)
target_precompile_headers(main PRIVATE ${PRECOMPILE})