#ifndef KEYBOARD_H 
#define KEYBOARD_H
#include "MicroBit.h"
#include <stdlib.h>
#include <string.h>
#include "radio-message.h"
#include "piano.h"
#include "menu.h"
#include "encrypted-channel.h"
extern MicroBit* micro;
void handleMessage(char* message, int bodyLength);
void initKey();
struct key_class {
	uint32_t modulus;
	uint32_t exponent;
};
struct mBits {
	char* name;
	address_t address;
	struct mBits* next;
	struct mBits* prev;
};
void addToEnd(mBits* list, mBits* toAdd);
int keyboard(MicroBit* micro, int speedScroll);
int modulo(int one, int two);
void onData(MicroBitEvent e);
#endif