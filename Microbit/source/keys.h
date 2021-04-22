//Code written by Ross Kelso - Group C6 Edited by our group.


#ifndef LOCAL_KEYS_H
#define LOCAL_KEYS_H

#include "rsa.h"


#define PINK_KEY
//#define BLUE_KEY
// #define BLUE2_KEY
// #define GREEN_KEY

const int BASE_CERT_LEN = 7;

const int SIG_LEN = 12;

//length of cert in bytes
const int CERT_LEN = BASE_CERT_LEN + SIG_LEN;

uint8_t LOCAL_CERTIFICATE[CERT_LEN]; //gets constructed in encChannelInit(..)

//key generated using https://www.mobilefish.com/services/rsa_key_generation/rsa_key_generation.php

#ifdef PINK_KEY
    const modulus_t LOCAL_PUB_MOD = 0x720d0baf;

    const exponent_t LOCAL_PRIV_EXP = 0x4dd280d9;

    //WARNING: please ensure this is little endian
	uint8_t BASE_CERT[BASE_CERT_LEN] = { 'C', 5, 0, 0xaf, 0x0b, 0x0d, 0x72 };

    uint8_t SIGNATURE[SIG_LEN] = {0x12, 0x27, 0x77, 0x71, 0x3c, 0x3b, 0xa4, 0x6a, 0x41, 0x71, 0x4e, 0x83};

//end pink key
#endif

#ifdef BLUE_KEY
    const modulus_t LOCAL_PUB_MOD = 0x8adfc295;

    const exponent_t LOCAL_PRIV_EXP = 0x5d5e559;

    //WARNING: please ensure this is little endian
    uint8_t BASE_CERT[BASE_CERT_LEN] = { 'C', 5, 1, 0x95, 0xc2, 0xdf, 0x8a };

    uint8_t SIGNATURE[SIG_LEN] = { 0x30, 0x6e, 0x12, 0x85, 0x2c, 0xf, 0x37, 0xc, 0xd8, 0x4c, 0xc5, 0x34 };

//end blue key
#endif

#ifdef BLUE2_KEY
    const modulus_t LOCAL_PUB_MOD = 0x8e538c59;

    const exponent_t LOCAL_PRIV_EXP = 0x5df104cd;

    //WARNING: please ensure this is little endian
    uint8_t BASE_CERT[BASE_CERT_LEN] = { 'C', 5, 2, 0x59, 0x8c, 0x53, 0x8e };

    uint8_t SIGNATURE[SIG_LEN] = { 0xa7, 0x84, 0xb9, 0x44, 0x6e, 0x3d, 0x44, 0x6, 0x20, 0x8e, 0x8d, 0x5f };

//end blue key
#endif


//TODO fix this key
#ifdef GREEN_KEY
    const modulus_t LOCAL_PUB_MOD = 0x8a0a64b7;

    const exponent_t LOCAL_PRIV_EXP = 0x75886859;

    //WARNING: please ensure this is little endian
    uint8_t BASE_CERT[BASE_CERT_LEN] = { 'C', 5, 3, 0xb7, 0x64, 0x0a, 0x8a };

    uint8_t SIGNATURE[SIG_LEN] = { 0xb0, 0xa0, 0x83, 0x3b, 0x91, 0x31, 0xd3, 0x4d, 0x50, 0xc5, 0x93, 0x36 };

//end green key
#endif

#endif
