//Code written by Ross Kelso - Group C6 Edited by our group.


#ifndef LOCAL_CONFIG_H
#define LOCAL_CONFIG_H

#include "radio-message.h"

//the depth this microbit uses when sending messages
const int DEPTH = 5;

//the local address of this microbit
extern address_t local_address;

//the function that will be called when a message is decrypted
extern void (*message_recieved) (char* body, int len);

#endif
