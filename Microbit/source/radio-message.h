/** Authors:
 * Ross Kelso        - C6
 * Paul Lancaster    - C2
 * Josh Wood         - C2
 * Bhuvan Bezawada   - C7
 * Ville Kuosmanen   - C8
 * Ignacy Debicki    - C4
 * Ria Chakrabarti   - C4
 **/

#ifndef RADIO_MESSAGE_H
#define RADIO_MESSAGE_H

// Imports
#include <stdint.h>
#include <ctype.h>
#include "MicroBit.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

// Super group ID
#define SUPER_GROUP_ID 3

// The radio group / channel for super group c.
#define SUPER_GROUP_C_GROUP 30

// The number of groups in a super group
#define NUMBER_OF_GROUPS 8

// Seed value for hash function
// Choose arbitarly, has to be the same for all microbits in the super group in-order
// to be able to hash a message to check for correctness.
#define SEED 0xC

// If FLAG_ACK and FLAG_REF bits are 0 then original message / initial message
// If FLAG_ACK and FLAG_REF bits are 1 then default to refused.
#define FLAG_ENCRYPTED 0b00001000 // Indicates that this message it to be sent to a microbit.
#define FLAG_ACK 0b00000100 // Indicates this is an acknowledgement message.
#define FLAG_REF 0b00000010 // Indicates this is a refusal message.
#define FLAG_DISCOVER 0b00000001 // Indicates that this is a discovery message.

typedef uint32_t address_t;
typedef uint32_t messageId_t;
typedef uint16_t bodyLength_t;
typedef uint32_t checkSum_t;
typedef char byte_t;

// Size of the packet header in bytes
const int PACKET_HEADER_SIZE = sizeof(address_t) + sizeof(address_t) + sizeof(byte_t) + sizeof(messageId_t)
                             + sizeof(bodyLength_t) + sizeof(checkSum_t);

// Radio packet struct
typedef struct cRadioPacket {
    address_t destination;
    address_t source;
    byte_t depth;
    byte_t flags;
    messageId_t messageId;
    bodyLength_t bodyLength;
    checkSum_t checksum;
    char* body; // Pointer to an array of size bodyLength, undefined behaviour if this isn't followed.
} CRadioPacket;

/**
 * Broadcasts a message to all microbits asking them to send back an acknowledgement with their microbit address.
 **/
int sendDiscoveryMessage(MicroBit* uBit, char depth);

// Function prototype for initialising microbit radio communication
void initRadio(MicroBit* uBit);

// Checksum function prototype
// Taken from wikipedia https://en.wikipedia.org/wiki/MurmurHash
checkSum_t murmur3_32(const uint8_t *body, size_t len);

// Function prototype for sending a packet
int sendPacket(MicroBit* uBit, CRadioPacket *packet, unsigned int packetSize);

// Function prototype for building a packet struct
int buildPacketStruct(CRadioPacket *packet, address_t dest, address_t source,
                      char depth, bool toUBit, bool ack, bool ref, bool discovery, messageId_t messageId,
                      char *body, bodyLength_t bodyLength);

// Function prototype for generating a message ID
messageId_t generateMessageId(MicroBit* micro);

// Function prototype for sending a text message
int sendTextMessage(MicroBit* uBit, address_t dest, char depth, char *body, bodyLength_t bodyLength);

// Function prototype for sending the full details of the message
int sendMessageDetail(MicroBit* uBit, address_t dest, address_t source, char depth, bool toUBit, bool ack, bool ref, bool discovery,
                        messageId_t messageId, char *body, bodyLength_t bodyLength);

// Function prototype for parsing a data packet
int parsePacket(char *bytes, CRadioPacket *buff);

// Function prototype for getting the address of a microbit
address_t getUBitAddress();

#endif
