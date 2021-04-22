//Code written by Ross Kelso - Group C6 Edited by our group.

#ifndef ENCRYPTED_CHANNEL_H
#define ENCRYPTED_CHANNEL_H


#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include "local-config.h"
#include "radio-message.h"
#include "rsa.h"
#include "keyboard.h"

const exponent_t PUB_EXPONENT = 65537;

typedef uint8_t supergroup_id_t;
typedef uint8_t group_id_t;
typedef uint8_t microbit_id_t;
typedef uint16_t nonce_t;

//the number of bytes the identifer is in the certificate
const size_t ID_LEN = sizeof(supergroup_id_t) + sizeof(group_id_t) + sizeof(microbit_id_t);

typedef uint8_t stage_t;

const stage_t REQ_CERT = 0;
const stage_t CERT_SEND_VERIFY = 1;
const stage_t CERT_SEND_READY = 2;
const stage_t ENCRYPTED_MSG = 3;

const int ENC_NONCE_MSG_T_SIZE = 1;
const int ENC_SIGNED_NONCE_MSG_T_SIZE = 2;
const int SIGNED_NONCE_MSG_T_SIZE = 1;

const int encNonceLen = sizeof(message_t);
const int encSignedNonceLen = sizeof(message_t) * 2;

const nonce_t MAX_NONCE = 65536;

typedef struct identifier {
    supergroup_id_t supergroupId;
    group_id_t groupId;
    microbit_id_t microbitId;
} Identifier;


typedef struct certificate {
    Identifier id;
    modulus_t pubKey;
    char* signature;
} Certificate;

typedef struct channel {
    address_t address;
    //the stage of the packet that I'm expecting
    stage_t stage;
    modulus_t pubKey;
    nonce_t lastNonceSent;
    nonce_t nextNonceToSend;
} Channel;

typedef struct channelList {
    Channel* head;
    channelList* tail;
} ChannelList;

const modulus_t ROOT_PUB = 0x8577C82D;

void encChannelInit(MicroBit* u);

//generate a nonce
nonce_t generateNonce();

//verify if a certificate is valid i.e. was signed by the root key
bool verifyCertificate(Certificate* cert);

//sign the input with our private key
message_t* sign(char* input);

//check if a channel exists
bool channelExists(address_t addr);

//create a channel struct and add it to the list
Channel* createChannel(address_t addr);

//open a channel to a microbit or the server
Channel* openChannel(address_t addr);

//get a channel from the list by address
Channel* getChannel(address_t addr);

//close a channel
void closeChannel(address_t addr);

//send a message over an encrypted channel
bool sendMessageOverChannel(Channel* channel, char* payload, int len);

//handle an encrypted radio packet
void handleEncryptedMessage(CRadioPacket p);

#endif
