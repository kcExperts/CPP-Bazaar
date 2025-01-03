### Project Code 1

Contains code for my first attempt at Networking:
Server:  
    + Thread for Listening  
    + Thread for Receiving  
    + Thread for Sending  
Client: 
    + Thread for Receiving  
    + Thread for Sending  

## Versions
V1.0.0:  
    + Seperated the code into networkCodeTCP.h and .cpp
    + Basic functionality is done. The server is centralized and uses TCP.
    + Added comments for clarity
    + User can now specify their own handle data function.
    + Functions all now uses references.

V0.3.0:  
    + Completed basic server functionality  
    - Weird problem with Network_Client_Information networkClientInfo found when trying to change it to a reference.

V0.2.2:  
    + Added basic Client code  
    - Turns out (after MUCH debugging) that my installation of mingw was breaking condition variables. Fixed

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

## Issues

- [x] Fix this whole cv debacle as the receiving thread will get stuck if the server is empty, even if isReceiving is set to false.
- [x] Move the position of Network_HandleData(dataReceived); in Network_Server_Receive. Add a check for when data send is successfull so that data can be properly handled.  
- [x] When networkClientInfo becomes a reference and is put that way in each function, setting a message adds an extra unknown char that should not be present. It could also not be the message setting and instead be a server side problem.
