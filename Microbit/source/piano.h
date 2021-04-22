#ifndef PIANO_H
#define PIANO_H
#include "MicroBit.h"
#include "menu.h"
void initPiano();
int readKeyPress();
int pianoMain(MicroBit* micro, bool record);
void chooseSong(MicroBit* micro, int scrollSpeed);
void parseMusicMessage(char* message);
#endif
