cmake_minimum_required(VERSION 3.25)
project(chat)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Add sources and headers
# set(SOURCES
#     src/server.cpp
#     src/client.cpp
# )

set(SERVER
    src/main.cpp
    src/chatserver.h
    src/chatserver.cpp
)

add_executable(server ${SERVER})
add_executable(client src/client.cpp)
target_link_libraries(server ws2_32)
target_link_libraries(client ws2_32)
