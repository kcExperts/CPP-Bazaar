cmake_minimum_required(VERSION "3.30.2")

project("ModularChatScreen")

set(CMAKE_CXX_STANDARD 17) #It is what I have
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)


SET(PRECOMPILE
    src/precomp.h
)

SET(HEADERS
    src/ChatModule.h
    src/ChatModuleDefines.h
    src/ChatObject.h
    src/networkcodeTCP.h
)

SET(CODEFILES
    src/ChatModule.cpp
    src/ChatObject.cpp
    src/networkcodeTCP.cpp
)

add_executable(main src/main.cpp ${PRECOMPILE} ${HEADERS} ${CODEFILES})
target_link_libraries(main ws2_32 raylib)
target_precompile_headers(main PRIVATE ${PRECOMPILE})