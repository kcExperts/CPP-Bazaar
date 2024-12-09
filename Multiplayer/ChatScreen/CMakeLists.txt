cmake_minimum_required(VERSION 3.25)
project(chat)

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
    src/button.cpp
    src/chatclient.cpp
    src/chatdata.cpp
    src/chatscreen.cpp
    src/chatserver.cpp
    src/mouse.cpp
    src/program.cpp
    src/button.h
    src/chatclient.h
    src/chatdata.h
    src/chatscreen.h
    src/chatserver.h
    src/mouse.h
    src/program.h
    src/constants.h
    src/screenSizeTransfer.h
    src/precomp.h
    
    src/programMain.cpp
)

# add_executable(server ${SERVER})
# add_executable(client ${CLIENT})
add_executable(main ${PROGRAM})
# Precompile the header for 'main' target
target_precompile_headers(main PRIVATE "src/precomp.h")
# target_link_libraries(server ws2_32 raylib)
# target_link_libraries(client ws2_32 raylib)
target_link_libraries(main raylib ws2_32)
