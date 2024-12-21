cmake_minimum_required(VERSION "3.30.2")

project("ModularChatScreen")

set(CMAKE_CXX_STANDARD 17) #It is what I have
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(main src/main.cpp)