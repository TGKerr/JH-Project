#include "MicroBit.h"

MicroBit uBit;

int CHIP_ADDRESS = 0x0D << 1;
bool    INITIALISED = 0;
    
int        KEY_K0 = 0x100;
int        KEY_K1 = 0x200;
int        KEY_K2 = 0x400;
int        KEY_K3 = 0x800;
int        KEY_K4 = 0x1000;
int        KEY_K5 = 0x2000;
int        KEY_K6 = 0x4000;
int        KEY_K7 = 0x8000;
int        KEY_K8 = 0x01;
int        KEY_K9 = 0x02;
int        KEY_K10 = 0x04;
int        KEY_K11 = 0x08;
int        KEY_K12 = 0x10;
int        KEY_K13 = 0x20;
int        KEY_K14 = 0x40;
    
int    keySensitivity = 8;  

int    keyNoiseThreshold = 5;
int    keyRegValue = 0x0000;

int million = 1000000;



void initPiano()
{
    char buff[1] = {0};
    char buff2[2] = {0};
    char buff3[5] = {0};

    uBit.io.P1.setPull(PullUp);
    buff[0] = 0;
    uBit.i2c.write(CHIP_ADDRESS, buff, 1, false);
    uBit.i2c.read(CHIP_ADDRESS, buff, 1,false);

  //  uBit.display.scroll("x :)");


    while(buff[0] != 0x11) {
        uBit.i2c.read(CHIP_ADDRESS, buff, 1, false);
     //   uBit.display.scroll("y :)");
    }
    
   // uBit.display.scroll("z :)");

    // Change sensitivity (burst length) of keys 0-14 to keySensitivity (default is 8);
    for(int sensitivityReg = 54; sensitivityReg < 69; sensitivityReg++) {
        buff2[0] = sensitivityReg;
        buff2[1] = keySensitivity;
        uBit.i2c.write(CHIP_ADDRESS, buff2, 0);
    }

    // Disable key 15 as it is not used
    buff2[0] = 69;
    buff2[1] = 0;
    uBit.i2c.write(CHIP_ADDRESS, buff2, 2);

    // Set Burst Repetition to keyNoiseThreshold (default is 5);
    buff2[0] = 13;
    buff2[1] = keyNoiseThreshold;
    uBit.i2c.write(CHIP_ADDRESS, buff2, 2);
            
    //Configure Adjacent Key Suppression (AKS) Groups
    //AKS Group 1: ALL KEYS
    for (int aksReg = 22; aksReg < 37; aksReg++) {
        buff2[0] = aksReg;
        buff2[1] = 1;
        uBit.i2c.write(CHIP_ADDRESS, buff2, 2);
    }

    // Send calibration command
    buff2[0] = 10;
    buff2[1] = 1;
    uBit.i2c.write(CHIP_ADDRESS, buff2, 2);

    // Read all change status address (General Status addr = 2);
    buff[0] = 2;
    uBit.i2c.write(CHIP_ADDRESS, buff, 1);
    uBit.i2c.read(CHIP_ADDRESS, buff3, 5, 0);
    // Continue reading change status address until /change pin goes high
    //uBit.display.scroll("1",50);

	uBit.display.scroll(uBit.io.P1.getDigitalValue());

       // uBit.display.scroll("2",50);

        buff[0] = 2;
        uBit.i2c.write(CHIP_ADDRESS, buff, 1);
        uBit.i2c.read(CHIP_ADDRESS, buff3, 5, 0);
        INITIALISED = 1;
    
   // uBit.display.scroll("3 :)",50);

}

int readKeyPress(){
    char buff[1] = {0};
    char buff2[2] = {0};
    char buff3[5] = {0};

    buff[0] = 2;
    uBit.i2c.write(CHIP_ADDRESS, buff, 1, false);
    uBit.i2c.read(CHIP_ADDRESS, buff3, 5, false);

    //Address 3 is the addr for keys 0-7 (this will then auto move onto Address 4 for keys 8-15, both reads stored in buff2)
    buff[0] = 3;
    uBit.i2c.write(CHIP_ADDRESS, buff, 1, false);
    uBit.i2c.read(CHIP_ADDRESS, buff2, 2, false);

    //keyRegValue is a 4 byte number which shows which keys are pressed
    int keyRegValue = (buff2[1] + (buff2[0] * 256));
	//if(keyRegValue != 0) uBit.display.scroll(keyRegValue);

    return keyRegValue;
    }

bool keyIsPressed(int key) {
    bool keyPressed = false;

    if (INITIALISED == 0) {
		uBit.display.scroll("Here");
        initPiano();
    }

    if (readKeyPress() == key) {
       keyPressed = true; 
    }

    return keyPressed;
}


int main()
{
    // Initialise the micro:bit runtime.
    uBit.init();

    uBit.display.scroll("A :)");
    initPiano();
    while(1)
    {
		int key = readKeyPress();
		
		//uBit.display.scrollAsync(key);
        //uBit.display.scroll("B :)");
        // Try adjusting this value!
        // This is frequency in C - 3823ish is a middle C
        if (key == KEY_K9) {
            uBit.io.P0.setAnalogPeriodUs(million/261);
			uBit.io.P0.setAnalogValue(511);
			uBit.sleep(100);
			uBit.io.P0.setAnalogPeriodUs(0);
        }
        if (key == KEY_K1) {
            uBit.io.P0.setAnalogPeriodUs(million/277);
			uBit.io.P0.setAnalogValue(511);
			uBit.sleep(100);
			uBit.io.P0.setAnalogPeriodUs(0);
        }
        if (key == KEY_K10) {
            uBit.io.P0.setAnalogPeriodUs(million/293);
			uBit.io.P0.setAnalogValue(511);
			uBit.sleep(100);
			uBit.io.P0.setAnalogPeriodUs(0);
        }
        if (key == KEY_K2) {
            uBit.io.P0.setAnalogPeriodUs(million/311);
			uBit.io.P0.setAnalogValue(511);
			uBit.sleep(100);
			uBit.io.P0.setAnalogPeriodUs(0);
        }
        if (key == KEY_K11) {
            uBit.io.P0.setAnalogPeriodUs(million/329);
			uBit.io.P0.setAnalogValue(511);
			uBit.sleep(100);
			uBit.io.P0.setAnalogPeriodUs(0);
        }
        if (key == KEY_K12) {
            uBit.io.P0.setAnalogPeriodUs(million/349);
			uBit.io.P0.setAnalogValue(511);
			uBit.sleep(100);
			uBit.io.P0.setAnalogPeriodUs(0);
        }
        if (key == KEY_K3) {
            uBit.io.P0.setAnalogPeriodUs(million/370);
			uBit.io.P0.setAnalogValue(511);
			uBit.sleep(100);
			uBit.io.P0.setAnalogPeriodUs(0);
        }
        if (key == KEY_K13) {
            uBit.io.P0.setAnalogPeriodUs(million/392);
			uBit.io.P0.setAnalogValue(511);
			uBit.sleep(100);
			uBit.io.P0.setAnalogPeriodUs(0);
        }
        if (key == KEY_K4) {
            uBit.io.P0.setAnalogPeriodUs(million/415);
			uBit.io.P0.setAnalogValue(511);
			uBit.sleep(100);
			uBit.io.P0.setAnalogPeriodUs(0);
        }
        if (key == KEY_K14) {
            uBit.io.P0.setAnalogPeriodUs(million/440);
			uBit.io.P0.setAnalogValue(511);
			uBit.sleep(100);
			uBit.io.P0.setAnalogPeriodUs(0);
        }
        if (key == KEY_K5) {
            uBit.io.P0.setAnalogPeriodUs(million/466);
			uBit.io.P0.setAnalogValue(511);
			uBit.sleep(100);
			uBit.io.P0.setAnalogPeriodUs(0);
        }
        if (key == KEY_K6) {
            uBit.io.P0.setAnalogPeriodUs(million/493);
			uBit.io.P0.setAnalogValue(511);
			uBit.sleep(100);
			uBit.io.P0.setAnalogPeriodUs(0);
        }
        if (key == KEY_K7) {
            uBit.io.P0.setAnalogPeriodUs(million/523);
			uBit.io.P0.setAnalogValue(511);
			uBit.sleep(100);
			uBit.io.P0.setAnalogPeriodUs(0);
        }
        
        //uBit.sleep(500);
    }



}