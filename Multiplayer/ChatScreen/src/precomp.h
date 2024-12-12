/*
Raylib and Windows share many common names for functions. Hence I put raylib inside a namespace.
*/

#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <ws2tcpip.h> //InetPtonA function

#undef DrawText

#define RAYLIB_NAMESPACE_BEGIN namespace rl {
#define RAYLIB_NAMESPACE_END }

RAYLIB_NAMESPACE_BEGIN
#include "raylib.h"
RAYLIB_NAMESPACE_END