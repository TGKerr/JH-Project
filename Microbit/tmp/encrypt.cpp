#include "project.h"

unsigned long long p = 53173;
unsigned long long q = 32869;
unsigned long long n = 2239219757;
unsigned long long phi = 1747657296;
unsigned long long e = 65537;
unsigned long long d = 1590384365;


uint32_t rsa_modExp(unsigned int b, long long e, long long m) {
	// base %= modulus;
  	// uint32_t result = 1;
  	// while (exp > 0) {
    // 	if (exp & 1) result = (result * base) % modulus;
    // 	base = (base * base) % modulus;
    // 	exp >>= 1;
  	// }
  	// return result;
	micro->display.scroll("A", 1);
  if (b < 0 || e < 0 || m <= 0){
    micro->display.scroll("encryption error", 50);
    exit(1);
  }
  b = b % m;
  if(e == 0) return 1;
  if(e == 1) return b;
  if( e % 2 == 0){
    return ( rsa_modExp(b * b % m, e/2, m) % m );
  }
  if( e % 2 == 1){
    return ( b * rsa_modExp(b, (e-1), m) % m );
  }

}

// encrypts message using provided key
char* rsa_encrypt(char *message, int message_size) {
  char* encrypted = (char*) malloc(sizeof(char)*(ceil((float)message_size/3.0)*4));
  if(encrypted == NULL){
    fprintf(stderr, "Error: Heap allocation failed.\n");
    return NULL;
  }
  int i = 0;
   for(i=0; i < message_size/3; i++){
	 unsigned int encryptbytes = ((unsigned char*)message)[(i*3)] + (((unsigned char*)message)[(i*3)+1] << 8) + (((unsigned char*)message)[(i*3)+2] << 16);
     uint32_t encryptedNum = rsa_modExp(encryptbytes, e, n);
	 encrypted[i*4] = (encryptedNum >> 24) & 0xFF;
	 encrypted[i*4+1] = (encryptedNum >> 16) & 0xFF;
	 encrypted[i*4+2] = (encryptedNum >> 8) & 0xFF;
	 encrypted[i*4+3] = (encryptedNum) & 0xFF;
   }
  return encrypted;
}

char* rsa_decrypt(char* encrypted, int message_size){
	char* decrypted = (char*) malloc((message_size/4)*3);
	micro->display.scroll(message_size);
	for(int i = 0; i < message_size/4; i++){
		    unsigned int valueToDecrypt = *((int *) (encrypted + i*4));
			unsigned int decryptedNum = rsa_modExp(valueToDecrypt, d, n);
			micro->display.scroll((int) decryptedNum);
			decrypted[i*3+2] = (decryptedNum >> 16) & 0xFF;
			decrypted[i*3+1] = (decryptedNum >> 8) & 0xFF;
	 		decrypted[i*3] = (decryptedNum) & 0xFF;
	}
	return decrypted;

}

