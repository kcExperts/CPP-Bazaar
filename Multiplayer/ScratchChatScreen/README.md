# ChatScreen

## Description

A modular chatscreen implemented using winsock2. Uses threads.

## Version History

V0.2:  
    - Added better text writing.  
    - Rewrote position code. Drawing now applies a linear transformation to match position to actual on screen position.  
    + Next Patch should include some basic network code.

V0.1:  
    - Added Select and Join Menu.  
    - Added utility function to coding easier.  
    - Added support for typing.

V0.0:  
    - Created basic window that supports proper drag.  
    - Created loading screen with rotating texture.  
    - Framework created.

## TODO

- Fix
    - [ ] Figure out what is going on with centering of textures and destination rectangles
    - [ ] Ensure animations are not tied to FPS
    - [x] Fix Dragging Breaking Everything

- GUI
    - [x] Create window
    - [ ] Setup Buttons
        - [ ] Use container that can hold identifier and button bounds/position
    - [x] Window Drag
        - [x] Implement window drag
        - [x] Window should not be able to be dragged out of the screen
        - [x] Window drag should not be sticky
    - [ ] Menus
        - [x] StartMenu, should be able to set username here
        - [ ] Manual Join Menu
        - [x] Manual Host Menu
        - [ ] Option for menu joining and host to be automated outside of program
    - [ ] Chat Window
        - [ ] Support different lobby sizes
        - [ ] Support different text lengths
        - [ ] Display using dequeue of chars


- Networking
    - [ ] Should be P2P
    - [ ] Threads
        - [ ] One thread for listening
        - [ ] One thread for joining
        - [ ] One thread for receiving
    - [ ] Chat Object