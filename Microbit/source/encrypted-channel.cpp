//Code written by Ross Kelso - Group C6 Edited by our group.

#include "encrypted-channel.h"
#include "keys.h"

static MicroBit* uBit;

ChannelList* openChannels = NULL;

void (*message_recieved) (char* body, int len) = handleMessage;

void encChannelInit(MicroBit* u) {
    uBit = u;
    memcpy(LOCAL_CERTIFICATE, BASE_CERT, BASE_CERT_LEN);
    memcpy(LOCAL_CERTIFICATE + BASE_CERT_LEN, SIGNATURE, SIG_LEN);
}

//generate a nonce
nonce_t generateNonce() {
    //nonce_t nonce = uBit->random(MAX_NONCE);
    //TODO this is far more random than the random function, which seems to always report the same value
    nonce_t nonce = (nonce_t)uBit->systemTime();

    // uBit->display.scroll((int)nonce);

    return nonce;
    // return 42;
}


bool verifyCert(uint8_t* packetBody) {
    //verify certificate
    uint8_t* idAndKey = (uint8_t*)malloc(BASE_CERT_LEN);
    memcpy(idAndKey, packetBody, BASE_CERT_LEN);


    uint8_t* signature = (uint8_t*)malloc(SIG_LEN);
    memcpy(signature, packetBody + BASE_CERT_LEN, SIG_LEN);

    //DEBUG: test if the signature matches the one for this microbit (both have the same sig atm)
    // for (int i = 0; i < SIG_LEN; i++) {
    //     if (signature[i] != SIGNATURE[i]) {
    //         uBit->display.scroll((int)signature[i]);
    //         uBit->display.scroll((int)SIGNATURE[i]);
    //     }
    // }

    bool valid = verifySignature(idAndKey, BASE_CERT_LEN, signature, SIG_LEN / sizeof(message_t), PUB_EXPONENT, ROOT_PUB);

    //uBit->display.scroll((int)valid);

    free(idAndKey);
    free(signature);

    return valid;
}

bool verifyNonce(message_t* encSignedNoncePtr, Channel* channel) {
    //decrypt and verify the nonce

    //decrypt the nonce
    uint8_t* signedNonce = decrypt(LOCAL_PRIV_EXP, LOCAL_PUB_MOD, encSignedNoncePtr, ENC_SIGNED_NONCE_MSG_T_SIZE);

    //verify the signature
    nonce_t* noncePtr = (nonce_t*)decrypt(PUB_EXPONENT, channel->pubKey, (message_t*)signedNonce, SIGNED_NONCE_MSG_T_SIZE);

    nonce_t nonce = *((nonce_t*)noncePtr);

    //uBit->display.scroll((int)nonce);
    //uBit->display.scroll((int)channel->lastNonceSent);

    bool valid = nonce == channel->lastNonceSent;

    //increment the nonce for security reasons
    channel->lastNonceSent++;


    free(signedNonce);
    free(noncePtr);


    // uBit->display.scroll((int)valid);

    return valid;
}

nonce_t decryptNonce(message_t encNonce) {
    //decrypt nonce
    uint8_t* decNonce = decrypt(LOCAL_PRIV_EXP, LOCAL_PUB_MOD, &encNonce, ENC_NONCE_MSG_T_SIZE);
    nonce_t newNonce = *((nonce_t*)decNonce);

    free(decNonce);

    // uBit->display.scroll((int)newNonce);

    return newNonce;
}

modulus_t getPubKeyFromCert(char* certPtr) {
    modulus_t pubKey = 0;
    memcpy(&pubKey, certPtr + ID_LEN, sizeof(modulus_t));

    return pubKey;
}

void sendReqCert(Channel* channel) {
    //certificate then nonce
    int len = CERT_LEN + sizeof(nonce_t);
    char* body = (char*)malloc(len);

    memcpy(body, LOCAL_CERTIFICATE, CERT_LEN);

    nonce_t nonce = generateNonce();

    memcpy(body + CERT_LEN, &nonce, sizeof(nonce_t));

    sendMessageDetail(uBit, channel->address, local_address, DEPTH, true, false, false, true, generateMessageId(uBit), body, len);

	//uBit->display.scroll("Send");

    channel->lastNonceSent = nonce;

    free(body);
}

message_t* signAndEncryptNonce(Channel* channel) {
    // uBit->display.scroll((int)channel->nextNonceToSend);
    message_t* signedNonce = encrypt(LOCAL_PRIV_EXP, LOCAL_PUB_MOD, (uint8_t*)&(channel->nextNonceToSend), sizeof(nonce_t));


    message_t* encSignedNonce = encrypt(PUB_EXPONENT, channel->pubKey, (uint8_t*)(signedNonce), sizeof(message_t)/*it should be just one message_t*/);

    free(signedNonce);

    //increment the nonce for security reasons
    channel->nextNonceToSend++;

    return encSignedNonce;
}

void sendCertSendVerify(Channel* channel) {
    //certificate, encrypted nonce, signed and encrypted prev nonce
    int len = CERT_LEN + encNonceLen + encSignedNonceLen;
    char* body = (char*)malloc(len);

    memcpy(body, LOCAL_CERTIFICATE, CERT_LEN);

    nonce_t nonce = generateNonce();

    message_t* encNonce = encrypt(PUB_EXPONENT, channel->pubKey, (uint8_t*)&nonce, sizeof(nonce_t));

    memcpy(body + CERT_LEN, encNonce, encNonceLen);

    free(encNonce);

    message_t* encSignedNonce = signAndEncryptNonce(channel);

    memcpy(body + CERT_LEN + encNonceLen, encSignedNonce, encSignedNonceLen);

    free(encSignedNonce);

    sendMessageDetail(uBit, channel->address, local_address, DEPTH, true, true, false, false, generateMessageId(uBit), body, len);

    channel->lastNonceSent = nonce;

    free(body);
}

void sendCertSendReady(Channel* channel) {
    //encrypted nonce, signed and encrypted previous nonce, optional payload
    int len = encNonceLen + encSignedNonceLen;
    char* body = (char*)malloc(len);

    nonce_t nonce = generateNonce();

    message_t* encNonce = encrypt(PUB_EXPONENT, channel->pubKey, (uint8_t*)&nonce, sizeof(nonce_t));

    memcpy(body, encNonce, encNonceLen);

    free(encNonce);

    message_t* encSignedNonce = signAndEncryptNonce(channel);

    memcpy(body + encNonceLen, encSignedNonce, encSignedNonceLen);

    free(encSignedNonce);

	uBit->display.scroll("Sent");

    sendMessageDetail(uBit, channel->address, local_address, DEPTH, true, true, false, false, generateMessageId(uBit), body, len);

    channel->lastNonceSent = nonce;

    free(body);
}



//check if a channel exists
bool channelExists(address_t addr) {
    return getChannel(addr) != NULL;
}

//open a channel to a microbit or the server (i.e. send REQ_CERT)
Channel* openChannel(address_t addr) {
    Channel* channel = createChannel(addr);

    //send req cert
    sendReqCert(channel);

    channel->stage = CERT_SEND_VERIFY;

    return channel;
}

//create a channel struct and add it to the list
Channel* createChannel(address_t addr) {
    Channel* channel = (Channel*)malloc(sizeof(Channel));

    //initialise the channel
    channel->address = addr;
    channel->stage = REQ_CERT;
    channel->pubKey = 0;
    channel->lastNonceSent = 0;
    channel->nextNonceToSend = 0;

    //add the channel to the list
    ChannelList* listItem = (ChannelList*)malloc(sizeof(channelList));
    listItem->head = channel;
    listItem->tail = openChannels;
    openChannels = listItem;

    return channel;
}

//get a channel from the list by address
Channel* getChannel(address_t addr) {
    //uBit->display.scroll((int)addr);
    ChannelList* ptr = openChannels;
    while (ptr != NULL) {
        //uBit->display.scroll((int)(ptr->head->address));
        if (ptr->head->address == addr) {
            return ptr->head;
        }
        ptr = ptr->tail;
    }
    return NULL;
}

void removeChannel(address_t addr) {
    //TODO this could probably be neater
    ChannelList* ptr = openChannels;

    while (openChannels != NULL && openChannels->head->address == addr) {
        ChannelList* temp = openChannels;

        openChannels = openChannels->tail;

        free(temp->head);
        free(temp);
    }

    while (ptr != NULL && ptr->tail != NULL) {
        if (ptr->tail->head->address == addr) {
            ChannelList* channelList = ptr->tail;

            //remove the elemnt from the list
            if (channelList->tail == NULL) {
                ptr->tail = NULL;
            } else {
                ptr->tail = channelList->tail;
            }

            //free the element
            free(channelList->head);
            free(channelList);
        }

        ptr = ptr->tail;
    }
}

//close a channel
void closeChannel(address_t addr) {
    // uBit->display.scroll("Closing");
    //send an encryption refuse packet to close the connection
    sendMessageDetail(uBit, addr, local_address, DEPTH, true, false, true, false, generateMessageId(uBit), NULL, 0);

    //remove the channel from the channel list if it exists
    removeChannel(addr);
}



//send a message over an encrypted channel
//returns whether the operation was successful
bool sendMessageOverChannel(Channel* channel, char* payload, int len) {
    if (channel != NULL && channel->stage == ENCRYPTED_MSG) {
        //send a message
        int sizeEncPayload = (getLenWithPadding(len) / BLOCK_SIZE) * sizeof(message_t);
        int totalLen = encNonceLen + encSignedNonceLen + sizeEncPayload;
        char* body = (char*)malloc(totalLen);

        nonce_t nonce = generateNonce();

        message_t* encNonce = encrypt(PUB_EXPONENT, channel->pubKey, (uint8_t*)&nonce, sizeof(nonce_t));

        memcpy(body, encNonce, encNonceLen);

        free(encNonce);

        message_t* encSignedNonce = signAndEncryptNonce(channel);

        memcpy(body + encNonceLen, encSignedNonce, encSignedNonceLen);

        free(encSignedNonce);

        //copy the payload
        message_t* encPayload = encrypt(PUB_EXPONENT, channel->pubKey, (uint8_t*)payload, len);
        memcpy(body + encNonceLen + encSignedNonceLen, encPayload, sizeEncPayload);

        free(encPayload);

        sendMessageDetail(uBit, channel->address, local_address, DEPTH, true, true, false, false, generateMessageId(uBit), body, totalLen);

        channel->lastNonceSent = nonce;

        free(body);
		uBit->display.scroll("Enc");
        return true;
    } else {
		uBit->display.scroll("Enc Failed");
        return false;
    }
}

void decryptBody(message_t* body, int remainingLength) {
    int numMessageT = remainingLength / sizeof(message_t);

    char* payload = (char*)decrypt(LOCAL_PRIV_EXP, LOCAL_PUB_MOD, body, numMessageT);

    message_recieved(payload, numMessageT * BLOCK_SIZE);

    free(payload);
}

void handleEncryptedMessage(CRadioPacket p) {
    Channel* channel = getChannel(p.source);

    //TODO close the channel if we get a refusal
    if (p.flags & FLAG_REF) {
        if (channel != NULL) {
            removeChannel(p.source);
        }
        return;
    }

    if (channel != NULL) {
        uBit->display.scroll(channel->stage);
        switch (channel->stage) {
            case REQ_CERT:
                //why would i expect a req cert if the channel is already open?
                //invalid channel state
                closeChannel(p.source);
                break;
            case CERT_SEND_VERIFY:
                //uBit->display.scroll("CERT_SEND_VERIFY");
                //check that the flags are valid
                if (p.flags & FLAG_ACK) {
                    bool validCert = verifyCert((uint8_t*)p.body);

                    if (validCert) {
						//uBit->display.scroll("Valid Cert");
                        channel->pubKey = getPubKeyFromCert(p.body);

                        char* ptr = p.body + CERT_LEN + encNonceLen;

                        //copy the signed nonce
                        message_t* encSignedNonce = (message_t*)malloc(encSignedNonceLen);
                        memcpy(encSignedNonce, ptr, encSignedNonceLen);

                        //verify that the noce is valid
                        bool validNonce = verifyNonce(encSignedNonce, channel);
                        free(encSignedNonce);

                        if (validNonce) {
							//uBit->display.scroll("Valid Nonce");
                            //update the nonce
                            message_t encNonce;
                            memcpy(&encNonce, p.body + CERT_LEN, sizeof(message_t));

                            channel->nextNonceToSend = decryptNonce(encNonce);

                            sendCertSendReady(channel);
                            //expecting
                            channel->stage = ENCRYPTED_MSG;

                            // uBit->display.scroll("Ready");
                        } else {
							//uBit->display.scroll("Invalid");
                            closeChannel(p.source);
                        }
                    }  else {
                        closeChannel(p.source);
                    }
                } else {
                    //invalid channel state
                    closeChannel(p.source);
                }
                break;
            case CERT_SEND_READY:
                // uBit->display.scroll("CERT_SEND_READY");
                //check that the flags are valid
                if (p.flags & FLAG_ACK) {

                    //copy the nonce
                    message_t* encSignedNonce = (message_t*)malloc(encSignedNonceLen);
                    memcpy(encSignedNonce, p.body  + encNonceLen, encSignedNonceLen);

                    //verify the nonce
                    bool validNonce = verifyNonce(encSignedNonce, channel);

                    free(encSignedNonce);

                    if (validNonce) {
                        //update the nonce
                        message_t encNonce;
                        memcpy(&encNonce, p.body, sizeof(message_t));

                        channel->nextNonceToSend = decryptNonce(encNonce);

                        //TODO read the optional payload

                        channel->stage = ENCRYPTED_MSG;

                        // uBit->display.scroll("Ready");
                    } else {
                        closeChannel(p.source);
                    }
                } else {
                    //invalid channel state
                    closeChannel(p.source);
                }
                break;
            case ENCRYPTED_MSG:
                // uBit->display.scroll("ENCRYPTED_MSG");
                //verify and update nonce
                message_t* encSignedNonce = (message_t*)malloc(encSignedNonceLen);
                memcpy(encSignedNonce, p.body  + encNonceLen, encSignedNonceLen);

                //verify the nonce
                bool validNonce = verifyNonce(encSignedNonce, channel);

                free(encSignedNonce);

                if (validNonce) {

                    //update the nonce
                    message_t encNonce;
                    memcpy(&encNonce, p.body, sizeof(message_t));

                    channel->nextNonceToSend = decryptNonce(encNonce);

                    //read the payload

                    //the length of the encryption part of the header
                    int headerLen = sizeof(message_t) + sizeof(message_t) * 2;

                    int remainingLength = p.bodyLength - headerLen;

                    message_t* payload = (message_t*)malloc(remainingLength);
                    memcpy(payload, p.body + headerLen, remainingLength);

                    decryptBody(payload, remainingLength);

                    free(payload);
                } else {
                    //invalid channel state
                    closeChannel(p.source);
                }
                break;
        }
    } else {
        if (p.flags & FLAG_DISCOVER) {
            // uBit->display.scroll("REQ_CERT");
            //this is a REQ_CERT
            channel = createChannel(p.source);
			//uBit->display.scroll((int)p.body[2]);

            bool valid = verifyCert((uint8_t*)p.body);

            //verify that this certificate was signed by the root key
            if (valid) {

                //save nonce
                nonce_t nextNonce;
                memcpy(&nextNonce, p.body + CERT_LEN, sizeof(nonce_t));
                channel->nextNonceToSend = nextNonce;

                channel->pubKey = getPubKeyFromCert(p.body);

                //send a CERT_SEND_VERIFY
                sendCertSendVerify(channel);
            } else {
                //invalid certificate close the channel
                closeChannel(p.source);
            }

            //I will expect a CERT_SEND_READY next
            channel->stage = CERT_SEND_READY;
        } else {
            //invalid state send a reject
            closeChannel(p.source);
        }
    }
}
