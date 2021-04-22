//Code written by Ross Kelso - Group C6 Edited by our group.


#include "rsa.h"
#include <stdio.h>
#include <stdlib.h>

//perform rsa on a single 24 bit chunk
message_t rsa(exponent_t exponent, modulus_t modulus, message_t msg) {
    //based on psudeo code section https://en.wikipedia.org/wiki/Modular_exponentiation
    long long result = 1;
    long long base = msg;
    while (exponent > 0) {
        if (exponent % 2 == 1) {
            result = (result * base) % modulus;
        }
        base = (base * base) % modulus;
        exponent = exponent / 2;
    }

    return (message_t) (result);
}

//round the length up to tehe nearest multiple of 3
int getLenWithPadding(int len) {
    int outLen = len + (BLOCK_SIZE - (len % BLOCK_SIZE)); //TODO test
    // int outLen = len;
    // if (len % 3 == 1) {
    //     outLen += 2;
    // } else if (len % 3 == 2) {
    //     outLen += 1;
    // }
    return outLen;
}

//encrypt a byte array using the provided exponent ad modulus
message_t* encrypt(exponent_t exp, modulus_t modulus, uint8_t* arr, int len) {
    len = getLenWithPadding(len);

    message_t* result = (message_t*)malloc(len);
    int index = 0;
    for (int i = 0; i < len; i += BLOCK_SIZE, index++) {
        uint32_t block = 0;
        uint32_t byte0 = arr[i]  << 16;

        uint32_t byte1;
        if (i + 1 < len) {
            byte1 = arr[i + 1]  << 8;
        } else {
            byte1 = 0;
        }

        uint32_t byte2;
        if (i + 2 < len) {
            byte2 = arr[i + 2];
        } else {
            byte2 = 0;
        }


        block = block | byte0;
        block = block | byte1;
        block = block | byte2;


        message_t msg = { block };
        message_t enc = rsa(exp, modulus, msg);

        result[index] = enc;
    }

    return result;
}

//decrypt the provided message array with the provided exponent and modulus
//note: length is in bytes
// uint8_t* decrypt(exponent_t exp, modulus_t modulus, message_t* arr, int len) {
//     len = getLenWithPadding(len);
//
//     uint8_t* result = (uint8_t*)malloc(len);
//
//     for (int i = 0; i < (len / 3); i++) {
//         message_t msg = arr[i];
//         message_t dec = rsa(exp, modulus, msg);
//
//         uint8_t byte0 = (uint8_t)(dec >> 16);
//         uint8_t byte1 = (uint8_t)(dec >> 8);
//         uint8_t byte2 = (uint8_t)(dec);
//
//         result[i * 3] = byte0;
//
//         if ((i * 3) + 1 < len) {
//             result[(i * 3) + 1] = byte1;
//
//             if ((i * 3) + 2 < len) {
//                 result[(i * 3) + 2] = byte2;
//             }
//         }
//     }
//
//     return result;
// }

//in this version len is i message_ts
uint8_t* decrypt(exponent_t exp, modulus_t modulus, message_t* arr, int len) {
    uint8_t* result = (uint8_t*)malloc(len * BLOCK_SIZE);

    for (int i = 0; i < len; i++) {
        message_t msg = arr[i];
        message_t dec = rsa(exp, modulus, msg);

        uint8_t byte0 = (uint8_t)(dec >> 16);
        uint8_t byte1 = (uint8_t)(dec >> 8);
        uint8_t byte2 = (uint8_t)(dec);

        result[i * BLOCK_SIZE] = byte0;
        result[(i * BLOCK_SIZE) + 1] = byte1;
        result[(i * BLOCK_SIZE) + 2] = byte2;
    }

    return result;
}

//verify if a certificate is valid i.e. was signed by the root key
bool verifySignature(uint8_t* base, int baseLen, uint8_t* signature, int sigLen, exponent_t exp, modulus_t mod) {
    uint8_t* expectedBase = decrypt(exp, mod, (message_t*)signature, sigLen);

    bool valid = true;

    for (int i = 0; i < baseLen; i++) {
        //printf("%#x  %#x, ", base[i], expectedBase[i]);
        if (base[i] != expectedBase[i]) {
            valid = false;
            break;
        }
    }

    free(expectedBase);

    return valid;
}
