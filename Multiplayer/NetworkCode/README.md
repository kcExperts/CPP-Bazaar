Contains test code for the networking portion of the ScratchChatScreen.

## Issues

- [ ] Fix this whole cv debacle as the receiving thread will get stuck if the server is empty, even if isReceiving is set to false. I presume this would also happen when the server is full to the listening thread. I am probably incorrectly using condition variables.

## Versions

V0.2:  
    + Rewrote the majority of the code making better use of mutexes and condition variables.  
    - All std::promise variables removed

V0.1.1:  
    + Partial fix to the threads not ending. Issue is not completely fixed.

V0.1:  
    + Added basic code, server side has yet to be finished, but a majority of the code is there.  
    - Note that the code gets stuck on attempting to join the listen thread. Issue will be fixed.  