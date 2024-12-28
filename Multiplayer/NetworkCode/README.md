Contains test code for the networking portion of the ScratchChatScreen.

## Issues

- [x] Fix this whole cv debacle as the receiving thread will get stuck if the server is empty, even if isReceiving is set to false.

## Versions

V0.2.1:
    + Fixed the issue where killing one thread forces the other to become deadlocked. 
    - Removed the predicates for the condition variables, as they were not needed.

V0.2:  
    + Rewrote the majority of the code making better use of mutexes and condition variables.  
    - All std::promise variables removed

V0.1.1:  
    + Partial fix to the threads not ending. Issue is not completely fixed.

V0.1:  
    + Added basic code, server side has yet to be finished, but a majority of the code is there.  
    - Note that the code gets stuck on attempting to join the listen thread. Issue will be fixed.  