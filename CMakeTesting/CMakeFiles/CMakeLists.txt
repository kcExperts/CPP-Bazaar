cmake_minimum_required(VERSION 3.3) #3.0+ required
project(CMAKE_Basic 
    VERSION 1.0.0
    LANGUAGES CXX
)

add_executable(test1 
    #If built on Windows platform, executable linked with subsystem:windows option.
    # Will be ignored on other systems.
    WIN32 
    main.cpp
)
#Most basic library command.
#STATIC KEYWORD: Specified a shared or dll. 
    # Default lib on windows is targetName.lib, Unix is libtargetName.a
#Could use this for modules

#add_library(
#)

#Note that for simple exe's to work on other systems, included libraries must be static

