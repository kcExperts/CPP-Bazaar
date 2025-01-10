## NETWORK CODE 2

This contains improved TCP network code that runs on fewer threads.

# TODO

- Need to add signature for each client, ie a username instead of its position in the client_vector

# FIX

- If client username too long and connects to server, server breaks
- If client successfully connects with good username, server stuck on error 7


- Client program never terminates peacefully if server sends it a message
static inline int
__gthread_mutex_destroy (__gthread_mutex_t *__mutex)
{
  if (__gthread_active_p ())
    return __gthrw_(pthread_mutex_destroy) (__mutex);
  else
    return 0;
}

SEGMENTATION FAULT ON RETURN _GLTHROW blah blah

UPDATE: int bytesReceived = recv(client.socket, (char*)&data_Received, sizeof(data_Received), 0);
The line above causes the problem