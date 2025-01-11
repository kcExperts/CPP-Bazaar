# NETWORK CODE 2  

**TCP network code designed for multi-client server communication. Uses server-client structure.**  

---

## Overview  

Network Code 2 is an improved TCP network implementation that runs on fewer threads with new features and cleaner code. The benefit of this version is that it supports a basic built-in validation system for client identification and access control.

This project is a step forward in my network code journey.  

---

## Features  

- **Client Connection with Validation:**  
  - Clients must provide a valid code to connect to the server.  
  - Invalid codes or usernames result in disconnection, with the reason being sent to the client.  

- **Cross-Platform Compatibility:**  
  - Currently functional on Windows; plans for Linux compatibility are underway.  

- **Documentation and Testing:**  
  - A basic test program is included in `main.cpp`.  
  - Comprehensive documentation is planned for future updates.  

---

## Releases  

### V1  
- Full network code functionality.  
- Clients can connect by providing a valid code.  
- Automatic disconnection with feedback for invalid usernames or incorrect codes.  

---

## Folder Structure  

- **`NC PROJECT CODE 2`**  
  - **`main.cpp`**  
    - A basic test to verify the program's functionality.  
  - **`CMakeLists.txt`**  
    - Contains the configuration for building the project.  
  - **`src/`**  
    - **`ncCodeTCP.cpp` and `ncCodeTCP.h`**  
      - The core implementation of the networking library.  

---

## Future Improvements  

- Expand functionality to support Linux platforms.  
- Enhance documentation to include usage examples and API references.  
- Implement mechanisms for sending and receiving different types of data.  
