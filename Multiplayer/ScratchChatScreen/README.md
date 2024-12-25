# ChatScreen

## Description

A modular chatscreen implemented using winsock2. Uses threads.

## Version History

V1:
    - Added Select and Join Menu
    - Added utility function to coding easier
    - Added support for typing

V0: 
    - Created basic window that supports proper drag.
    - Created loading screen with rotating texture.
    - Framework created.

## TODO

- Fix
    - [ ] Figure out what is going on with centering of textures and destination rectangles
    - [ ] Ensure animations are not tied to FPS
    - [ ] Fix Dragging Breaking Everything

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
        - [ ] Manual Host Menu
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