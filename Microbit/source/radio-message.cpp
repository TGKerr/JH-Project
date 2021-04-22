/** Authors:
 * Ross Kelso        - C6
 * Paul Lancaster    - C2
 * Josh Wood         - C2
 * Bhuvan Bezawada   - C7
 * Ville Kuosmanen   - C8
 * Ignacy Debicki    - C4
 * Ria Chakrabarti   - C4
 **/

#include "radio-message.h"

// Has to be in range 0-255, C supergroup assigned range 169-255, arbitarly decided
#define SUPER_GROUP_NUMBER 200


/**
 * Initialise the group to use and enable use of radio communication.
 **/
void initRadio(MicroBit* uBit) {
  int r = uBit->radio.enable();
  if(r == MICROBIT_NOT_SUPPORTED){
    uBit->panic(43);
  }

  uBit->radio.setGroup(SUPER_GROUP_NUMBER);
}

messageId_t generateMessageId(MicroBit* micro){
  return micro->random(INT_MAX);
}

/**
 * Checksum function (murmur3_32)
 *
 * Taken from https://en.wikipedia.org/wiki/MurmurHash
 * Modified so that it returns checkSum_t instead of unit32_t
 * Modified to use defined SEED instead of seed parameter
 **/
checkSum_t murmur3_32(const uint8_t* body, size_t len) {
  checkSum_t h = SEED;
  if (len > 3) {
    const uint32_t* key_x4 = (const uint32_t*) body;
    size_t i = len >> 2;
    do {
      uint32_t k = *key_x4++;
      k *= 0xcc9e2d51;
      k = (k << 15) | (k >> 17);
      k *= 0x1b873593;
      h ^= k;
      h = (h << 13) | (h >> 19);
      h = (h * 5) + 0xe6546b64;
    } while (--i);
    body = (const uint8_t*) key_x4;
  }
  if (len & 3) {
    size_t i = len & 3;
    uint32_t k = 0;
    body = &body[i - 1];
    do {
      k <<= 8;
      k |= *body--;
    } while (--i);
    k *= 0xcc9e2d51;
    k = (k << 15) | (k >> 17);
    k *= 0x1b873593;
    h ^= k;
  }
  h ^= len;
  h ^= h >> 16;
  h *= 0x85ebca6b;
  h ^= h >> 13;
  h *= 0xc2b2ae35;
  h ^= h >> 16;
  return h;
}


/**
 * Send the data packet.
 **/
int sendPacket(MicroBit* uBit, CRadioPacket* packet, unsigned int packetSize) {
  // Create a packet buffer
  PacketBuffer pb(packetSize);

  uint8_t* buff = pb.getBytes();
  int index = 0;
  // Bytes 0-3 is for the destination address
  // Bytes 4-8 is for the source address
  memcpy(buff + index, &(packet->destination), sizeof(address_t));
  index += sizeof(address_t);
  memcpy(buff + index, &(packet->source), sizeof(address_t));
  index += sizeof(address_t);

  // Bit shift by 4 merges flags and depth into the buffer as 1 byte.
  // Depth will be followed by flags due to endianness
  byte_t depthFlags = packet->flags | (packet->depth << 4);

  memcpy(buff + index, &depthFlags, sizeof(byte_t));
  index += sizeof(byte_t);

  // Store metadata about the data packet
  memcpy(buff + index, &(packet->messageId), sizeof(messageId_t));
  index += sizeof(messageId_t);

  memcpy(buff + index, &(packet->bodyLength), sizeof(bodyLength_t));
  index += sizeof(bodyLength_t);

  memcpy(buff + index, &(packet->checksum), sizeof(checkSum_t));
  index += sizeof(checkSum_t);

  // Copy over the body of the message
  memcpy((buff + index /*+ PACKET_HEADER_SIZE*/), packet->body, packet->bodyLength);

  return uBit->radio.datagram.send(pb);
}


int sendDiscoveryMessage(MicroBit* uBit, char depth){
  address_t dest = 0; // Destination is 0 because this should be broadcast to all microbits.
  address_t src = getUBitAddress();
  bool ack = false;
  bool ref = false;
  bool encrypted = false;
  bool discovery = true;
  messageId_t msgId = generateMessageId(uBit);
  char body[0]; // Empty body
  bodyLength_t bodyLength = 0;

  return sendMessageDetail (uBit, dest, src, depth, encrypted, ack, ref, discovery,
                      msgId, body, bodyLength);
}


/**
 * Build a data packet for use.
 * Return <0 if unable to build the packet struct (e.g. due to packet being NULL).
 **/
int buildPacketStruct(CRadioPacket* packet, address_t dest, address_t source,
                      char depth, bool encrypted, bool ack, bool ref, bool discovery, messageId_t messageId,
                      char* body, bodyLength_t bodyLength) {

  if (packet == NULL){
    return -1; // Failed
  }

  // 0 out the initial flags
  char flags = 0;

  // Set flag based on acknowledgement or refusal of message
  if (ack){
    flags = flags | FLAG_ACK;
  }

  if (ref){
    flags = flags | FLAG_REF;
  }

  if (encrypted){
    flags = flags | FLAG_ENCRYPTED;
  }

  if (discovery){
    flags = flags | FLAG_DISCOVER;
  }

  // Assign members
  packet->destination = dest;
  packet->source = source;
  packet->depth = depth;
  packet->flags = flags;
  packet->messageId = messageId;
  packet->bodyLength = bodyLength;
  packet->body = body;

  // Only doing the checksum on the body for now as easier to achieve.
  //packet->checksum = murmur3_32((uint8_t*)body, bodyLength);
  packet->checksum = 0; //TODO fix the chesum

  return 0; // Successful
}


/**
 * High level function.
 * Parameters should all be provided by end user.
 **/
int sendTextMessage(MicroBit* uBit, address_t dest, char depth, char* body, bodyLength_t bodyLength) {
  // Get source address
  address_t src = getUBitAddress();

  // Set flags
  bool encrypted = false;
  bool ack = false;
  bool ref = false;
  bool discovery = false;

  // Get message ID
  messageId_t messageId = generateMessageId(uBit);

  return sendMessageDetail(uBit, dest, src, depth, encrypted, ack, ref, discovery, messageId, body, bodyLength);
}

/**
 * Use the component parts of the message (that are dynamically created)
 * to build up a message to send.
 **/
int sendMessageDetail(MicroBit* uBit, address_t dest, address_t source, char depth, bool encrypted, bool ack, bool ref, bool discovery,
                      messageId_t messageId, char* body, bodyLength_t bodyLength) {

  // Store the packet size
  unsigned int packetSize = PACKET_HEADER_SIZE + bodyLength;

  // Create a packet to use
  CRadioPacket packet;

  // Build up the packet
  buildPacketStruct(&packet, dest, source, depth, encrypted, ack, ref, discovery, messageId, body, bodyLength);

  return sendPacket(uBit, &packet, packetSize);
}


/**
 * Parses the given bytes which represents a packet and places the data into the CRadioPacket buffer.
 * The data in the CRadioPacket is completely independant of the given bytes array which means
 * that the bytes array can be safely modified or freed without affecting the CRadioPacket buffer.
 *
 * size_t len: Specifies the maximum number of bytes to read.
 *             Included to help prevent overflow issues if a non-packet is passed in.
 *
 * Returns the number of bytes read or < 0 if creating the packet was unsuccessful.
 **/
int parsePacket(char* bytes, CRadioPacket* buff) {
  // Used to stored the next index in the bytes array to be parsed.
  int index = 0;

  // Copy the destination address
  memcpy(&(buff->destination),(bytes), sizeof(address_t));
  index = index + sizeof(address_t);

  // Copy the source address
  memcpy(&(buff->source), (bytes+index), sizeof(address_t));
  index = index + sizeof(address_t);

  // Copy the depth value
  // Masking out the last 4 bits as theses are used for the flag, not the depth.
  // 11110000 = 8 + 4 + 2 + 1 = 15
  // Index not incremented because the data for the depth and flags comes from the same byte.
  memcpy(&(buff->depth), (bytes+index), sizeof(byte_t));
  //buff->depth = (buff->depth & 15);
  buff->depth = (buff->depth & 0b11110000);
  buff->depth = buff->depth >> 4;

  // Copy the flags
  memcpy(&(buff->flags), (bytes+index), sizeof(byte_t));
  // Reversal of the bit shift is done to condense the flags and depth together
  //buff->flags = (buff->flags << 4);
  index = index + sizeof(byte_t);

  // Copy the messageID
  memcpy(&(buff->messageId), (bytes+index), sizeof(messageId_t));
  index = index + sizeof(messageId_t);

  // Copy the bodyLength
  memcpy(&(buff->bodyLength), (bytes+index), sizeof(bodyLength_t));
  index = index + sizeof(bodyLength_t);

  // Copy the checksum value
  memcpy(&(buff->checksum), (bytes+index), sizeof(checkSum_t));
  index = index + sizeof(checkSum_t);

  //due to a bug in the microbit firmware attempting to malloc a pointer of
  //size 0 results in the microbit panicing
  if (buff->bodyLength) {

    // Allocates new memory for the body to be stored in.
    char* newBodyPointer = (char*)malloc(buff->bodyLength);

    // Copy the body into this new memory.
    memcpy(newBodyPointer, (bytes+index), buff->bodyLength);

    // Store the location of the body within the packet.
    buff->body = newBodyPointer;
    // Increase the index so that it can be returned to show the number of bytes read.
    //buff->body = bytes + index;
    index = index + buff->bodyLength;
  } else {
      buff->body = NULL;
  }

  return index;
}


/**
 * Returns the address of a microbit
 **/
address_t getUBitAddress() {
  // Hardcoded address of the base of the Factory Information Configuration (FICR) struct
  const uint32_t *baseAddress = (uint32_t*)0x10000000;

  // The offset of the DEVICEID attribute in the struct
  const int idOffset = 25;

  // Store the address information
  const uint32_t* address = baseAddress + idOffset;

  return *address;
}
