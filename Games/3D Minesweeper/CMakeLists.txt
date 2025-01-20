cmake_minimum_required(VERSION "3.30.2")

project("3D Minesweeper")

set(CMAKE_CXX_STANDARD 17) 
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

SET(PRECOMPILE
    src/precomp.h
)

SET(HEADERS
)

SET(CODEFILES
    main.cpp
)

add_compile_options(
    -static 
    -static-libgcc 
    -static-libstdc++ 
    )
add_link_options(
    -static 
    )

add_executable(main main.cpp ${PRECOMPILE} ${HEADERS} ${CODEFILES})
target_link_libraries(main ws2_32 raylib winmm)
target_precompile_headers(main PRIVATE ${PRECOMPILE})

add_custom_target(copyresources ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/art
    ${CMAKE_BINARY_DIR}/art
    COMMENT "Copying resources to the build directory" #folder that has the images
)