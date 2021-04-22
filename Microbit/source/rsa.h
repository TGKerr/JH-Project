//Code written by Ross Kelso - Group C6 Edited by our group.


#include <stdint.h>
#include <stdbool.h>

#ifndef RSA_H
#define RSA_H


//the type of a prime number UNUSED
typedef uint16_t prime_t;

//the size of a modulus
typedef uint32_t modulus_t;

//the size of an exponent
typedef uint32_t exponent_t;

//the size of a message block (input should be 24 bytes)
typedef uint32_t message_t;

//perform rsa on a message block
message_t rsa(exponent_t exp, modulus_t modulus, message_t msg);

const int BLOCK_SIZE = 3;

//encrypt an input array
//note the resulting array was malloced and should be freed
modulus_t* encrypt(exponent_t exp, modulus_t modulus, uint8_t* arr, int len);

//decrypt the input array (len is in bytes)
//note the resulting array was malloced and should be freed
uint8_t* decrypt(exponent_t exp, modulus_t modulus, message_t* arr, int len);

int getLenWithPadding(int len);

bool verifySignature(uint8_t* base, int baseLen, uint8_t* signature, int sigLen, exponent_t exp, modulus_t mod);

#endif
