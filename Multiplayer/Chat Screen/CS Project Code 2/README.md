# ChatScreen

## Description

A modular chatscreen. Uses NC Project Code 1

## Version History

V1.3:  
    - Disabled allowing "." in port text.  

V1.2:  
    + Fixed resolution scaling for the loading menu, as I had forgotten about it.

V1.1:  
    + Resolution support has been implemented.

V1.0:  
    + All network code has been implemented.  
    + All client code has been implemented.  
    + All server code has been implemented.  
    + Chat screen has been set.  
    + Loading screen is used between long waiting times.  
    - Settings menu has yet to be implemented.  
    - Resolution support has yet to be implemented.

V0.2:  
    + Added better text writing.  
    + Rewrote position code. Drawing now applies a linear transformation to match position to actual on screen position.  
    - Next Patch should include some basic network code.

V0.1:  
    + Added Select and Join Menu.  
    + Added utility function to coding easier.  
    + Added support for typing.

V0.0:  
    + Created basic window that supports proper drag.  
    + Created loading screen with rotating texture.  
    + Framework created.

## TODO

- Fix
    - [ ] ADD THE DESTRUCTOR IN ORDER TO PROPERLY DESTROY THREADS!
    - [x] Figure out what is going on with centering of textures and destination rectangles
    - [ ] Ensure animations are not tied to FPS
    - [x] Fix Dragging Breaking Everything
    - [ ] When inputing a username that is exactly MAX_MSG_LEN long, it breaks when displaying to the chat window
    - [ ] Add a better disconnect message when a client disconnects without first sending anything
    - [ ] Error messages on host and join screens (see timeout on join) are off centered when scaled up
 
- GUI
    - [x] Add support for different resolutions
    - [x] Create window
    - [x] Setup Buttons
        - [x] Use container that can hold identifier and button bounds/position
    - [x] Window Drag
        - [x] Implement window drag
        - [x] Window should not be able to be dragged out of the screen
        - [x] Window drag should not be sticky
    - [ ] Menus
        - [x] StartMenu, should be able to set username here
        - [x] Manual Join Menu
        - [x] Manual Host Menu
        - [ ] Option for menu joining and host to be automated outside of program
        - [ ] Settings menu for window transparency and color
    - [x] Chat Window
        - [x] Support different lobby sizes
        - [x] Support different text lengths
        - [x] Display using dequeue of chars
        - [x] Allow server to see if a client has joined
        - [x] Allow server to see if a client has left
        - [ ] Allow a client to see if another client has left

