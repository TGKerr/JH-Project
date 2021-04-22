#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "rsa.h"
#include "keys.h"

const modulus_t ROOT_MOD = 0x8577C82D;
const exponent_t ROOT_PUB_EXP = 65537;
const exponent_t ROOT_PRIV_EXP = 0x5ECB56ED;

int main() {
    int outLen = (getLenWithPadding(BASE_CERT_LEN) / 3) * 4;

    printf("%d\n", outLen);

    uint8_t* signature = (uint8_t*)encrypt(ROOT_PRIV_EXP, ROOT_MOD, BASE_CERT, BASE_CERT_LEN);

    for (int i = 0; i < outLen; i++) {
        printf("%#x, ", signature[i]);
    }

    printf("\n");

    bool success = verifySignature(BASE_CERT, BASE_CERT_LEN, signature, SIG_LEN, ROOT_PUB_EXP, ROOT_MOD);

    printf("%d\n", success);

    bool verifyExisting = verifySignature(BASE_CERT, BASE_CERT_LEN, SIGNATURE, SIG_LEN / sizeof(message_t), ROOT_PUB_EXP, ROOT_MOD);

    printf("%d\n", verifyExisting);

    free(signature);
}
