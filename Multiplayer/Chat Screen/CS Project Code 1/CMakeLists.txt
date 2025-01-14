cmake_minimum_required(VERSION 3.25)
project(CS Project Code 1)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(SERVER
    src/main.cpp
    src/chatserver.h
    src/chatserver.cpp
    src/chatdata.h
    src/chatdata.cpp
    src/constants.h
)

set(CLIENT
    src/client.cpp
    src/chatdata.h
    src/chatdata.cpp
    src/chatclient.h
    src/chatclient.cpp
    src/constants.h
)

set(PROGRAM
    src/chatclient.cpp
    src/chatdata.cpp
    src/chatscreen.cpp
    src/windowserver.cpp
    src/windowclient.cpp
    src/window.cpp
    src/windowdata.cpp
    src/chatclient.h
    src/chatdata.h
    src/chatscreen.h
    src/windowserver.h
    src/windowclient.h
    src/constants.h
    src/screenSizeTransfer.h
    src/window.h
    src/windowdata.h
)
#-------------------------------------------------------------------------------
#Put this shit here before idk what this does
add_compile_options(
    -static 
    -static-libgcc 
    -static-libstdc++ 
    )
add_link_options(
    -static 
    )

    
# FetchContent to download raylib if not available
include(FetchContent)

FetchContent_Declare(
    raylib
    GIT_REPOSITORY https://github.com/raysan5/raylib.git
    GIT_TAG 4.5.0  # Replace with the desired raylib version
)

FetchContent_MakeAvailable(raylib)
#-------------------------------------------------------------------------------
add_executable(windowS src/windowmainserver.cpp ${PROGRAM})
add_executable(windowC src/windowmainclient.cpp ${PROGRAM})
target_link_libraries(windowS ws2_32 raylib)
target_link_libraries(windowC ws2_32 raylib)
target_precompile_headers(windowS PRIVATE "src/precomp.h")
target_precompile_headers(windowC PRIVATE "src/precomp.h")
# add_executable(server ${SERVER})
# add_executable(client ${CLIENT})
# target_link_libraries(server ws2_32 raylib)
# target_link_libraries(client ws2_32 raylib)
