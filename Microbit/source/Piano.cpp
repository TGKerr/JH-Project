#include "piano.h"

int CHIP_ADDRESS = 0x0D << 1;
bool    INITIALISED = 0;

//set addresses of all the keys 
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

//set key sensitivity
int    keySensitivity = 32;  

int    keyNoiseThreshold = 1;
int    keyRegValue = 0x0000;

//define the keys as 1 million/key frequency
int million = 1000000;

int Cmid = million/261;
int Csh = million/277;
int D = million/293;
int Efl = million/311;
int E = million/329;
int F = million/349;
int Fsh = million/370;
int G = million/392;
int Afl = million/415;
int A = million/440;
int Bfl = million/466;
int B = million/493;
int C = million/523;


//Initialise the Piano - this function was provided to us
void initPiano()
{
    char buff[1] = {0};
    char buff2[2] = {0};
    char buff3[5] = {0};

    micro->io.P1.setPull(PullUp);
    buff[0] = 0;
    micro->i2c.write(CHIP_ADDRESS, buff, 1, false);
    micro->i2c.read(CHIP_ADDRESS, buff, 1,false);

    while(buff[0] != 0x11) {
        micro->i2c.read(CHIP_ADDRESS, buff, 1, false);
		// micro->display.scroll("Not Docked");
		// menu(100);
    }

    // Change sensitivity (burst length) of keys 0-14 to keySensitivity (default is 8);
    for(int sensitivityReg = 54; sensitivityReg < 69; sensitivityReg++) {
        buff2[0] = sensitivityReg;
        buff2[1] = keySensitivity;
        micro->i2c.write(CHIP_ADDRESS, buff2, 0);
    }

    // Disable key 15 as it is not used
    buff2[0] = 69;
    buff2[1] = 0;
    micro->i2c.write(CHIP_ADDRESS, buff2, 2);

    // Set Burst Repetition to keyNoiseThreshold (default is 5);
    buff2[0] = 13;
    buff2[1] = keyNoiseThreshold;
    micro->i2c.write(CHIP_ADDRESS, buff2, 2);
            
    //Configure Adjacent Key Suppression (AKS) Groups
    //AKS Group 1: ALL KEYS
    for (int aksReg = 22; aksReg < 37; aksReg++) {
        buff2[0] = aksReg;
        buff2[1] = 1;
        micro->i2c.write(CHIP_ADDRESS, buff2, 2);
    }

    // Send calibration command
    buff2[0] = 10;
    buff2[1] = 1;
    micro->i2c.write(CHIP_ADDRESS, buff2, 2);

    // Read all change status address (General Status addr = 2);
    buff[0] = 2;
    micro->i2c.write(CHIP_ADDRESS, buff, 1);
    micro->i2c.read(CHIP_ADDRESS, buff3, 5, 0);
    // Continue reading change status address until /change pin goes high

	micro->display.scroll(micro->io.P1.getDigitalValue());

    buff[0] = 2;
    micro->i2c.write(CHIP_ADDRESS, buff, 1);
    micro->i2c.read(CHIP_ADDRESS, buff3, 5, 0);
    INITIALISED = 1;
    

}

//Convert Musical Message back to an int array so that it can be played
void parseMusicMessage(char* message){
	//number of notes
	micro->display.scroll("Music");
	int keyPresses = (strlen(message))/4;
	//micro->display.scroll((int)strlen(message));
	//initialise an array to store musical message
	int song[keyPresses];
	ManagedString music(message);       
	//Convert musical message from char* to int array
	for(int i = 0; i < keyPresses; i++){
		song[i] = atoi(music.substring(4*i, 4).toCharArray());
	}
	//Play musical message
	for(int i = 0; i < keyPresses; i++){
			micro->io.P0.setAnalogPeriodUs(song[i]);
			micro->io.P0.setAnalogValue(511);
			micro->sleep(333);
			micro->io.P0.setAnalogValue(0);
			micro->sleep(100);
	}
	return;
}

//Read Keys Pressed by User - this function was provided to us
int readKeyPress(){
    char buff[1] = {0};
    char buff2[2] = {0};
    char buff3[5] = {0};

    buff[0] = 2;
    micro->i2c.write(CHIP_ADDRESS, buff, 1, false);
    micro->i2c.read(CHIP_ADDRESS, buff3, 5, false);

    //Address 3 is the addr for keys 0-7 (this will then auto move onto Address 4 for keys 8-15, both reads stored in buff2)
    buff[0] = 3;
    micro->i2c.write(CHIP_ADDRESS, buff, 1, false);
    micro->i2c.read(CHIP_ADDRESS, buff2, 2, false);

    //keyRegValue is a 4 byte number which shows which keys are pressed
    int keyRegValue = (buff2[1] + (buff2[0] * 256));
	//if(keyRegValue != 0) micro->display.scroll(keyRegValue);

    return keyRegValue;
    }

//Send Musical Message
//Convert from int array to char*, then use radio function sendTextMessage
void sendMusic(int* music, int length){
    //Create buffers to store Musical Message
	char* buffer = (char*) calloc(1, sizeof(char) * 47);
	char* tempBuf = (char*) calloc(1, sizeof(char)*5);
	//Add "Music " to start of the message so that it can be identified as a musical message
	strcat(buffer, "Music ");
	//Add each note to the char*
	for(int i = 0; i < length; i++){
		//micro->display.scroll(i, 20);
		sprintf(tempBuf, "%d", music[i]);
		strcat(buffer, tempBuf);
	}
	//"Music 3831383134123183128653039"
	//micro->display.scroll(buffer);
	//Send Musical Message
	sendTextMessage(micro, 5, 1, buffer, 47);
	free(tempBuf);
	free(buffer);
}

//Choose from an existing song to send
void chooseSong(MicroBit* micro, int scrollSpeed){
	micro->sleep(500);
	//Define melodies of 2 songs: Happy Birthday to you & We wish you a Merry Christmass 
	int happyBirth[6] = {Cmid, Cmid, D, Cmid, F, E};
	int christmas[8] = {Cmid, F, F, G, F, E, D, D};
	int index = 0;
	//Define Menue of Songs
	ManagedString menu = "12";
	//Allow user to pick and then send song
  	while(true){
  	    //iterate through song menu
        int y_tilt = micro->accelerometer.getY();
        if(micro->buttonB.isPressed()) index = modulo(index+1, 2);
        if(micro->buttonA.isPressed()) index = modulo(index-1, 2);
		if(micro->buttonA.isPressed() && micro->buttonB.isPressed()){
			if(index == 0) micro->display.scroll("Happy Birthday", scrollSpeed);
			if(index == 1) micro->display.scroll("Merry Christmas", scrollSpeed);
		} 
        micro->display.print(menu.charAt(index));
        //If microbit is tilted forwards then send currently selected song
        if(y_tilt < -700){
            if(index == 0){
				sendMusic(happyBirth, 6);
				return;
			} 
            if(index == 1) {
				sendMusic(christmas, 8);
				return;
        	}
		}
		//If microbit is tilted backwards exit
        if(y_tilt > 700){
            break;
        }
        micro->sleep(200);
    }
}


//Main function used to play the piano
//2 choices while playing: free play, record melody (up to 10 notes long)
int pianoMain(MicroBit* microB, bool record)
{
    // Initialise the micro:bit runtime.
    initPiano();
    //Initialise array to store recorded notes
	int recorded[10] = { };
	int counter = 0;
	bool recording = record;
	//Display current selected mode
	if(recording) micro->display.scroll("REC", 50);
	else micro->display.scroll("PLAY");
    while(1)
    {
		if(microB->buttonA.isPressed()) return 1;
        //If the array is filled and the mode selected is recording
        //Then send recording and reset counter to 0
		if((counter == 10 | microB->buttonB.isPressed()) && recording){
			micro->display.scroll("Sending");
			sendMusic(recorded, counter);
			for(int i = 0; i < counter; i++){
				micro->io.P0.setAnalogPeriodUs(recorded[i]);
				micro->io.P0.setAnalogValue(511);
				micro->sleep(333);
				micro->io.P0.setAnalogValue(0);
				micro->sleep(100);
			}
			return 1;
		}
        //Record key that user is pressing
		int key = readKeyPress();

		// for(int i = 0; i < 8; i++) {
		// 	micro->io.P0.setAnalogPeriodUs(christmas[i]);
		// 	micro->io.P0.setAnalogValue(511);
		// 	micro->sleep(500);
		// 	micro->io.P0.setAnalogPeriodUs(0);
		// 	micro->sleep(100);
		// }
        
        //Check which key the user is pressing
        //For the correct key, play sound and add to array if the mode is recroding
        if (key == KEY_K9) {
            micro->io.P0.setAnalogPeriodUs(Cmid);
			micro->io.P0.setAnalogValue(511);
			micro->sleep(100);
			micro->io.P0.setAnalogPeriodUs(0);
			if (recording) {
				 recorded[counter] = Cmid;
				 counter ++;
			}
        }
        if (key == KEY_K1) {
            micro->io.P0.setAnalogPeriodUs(Csh);
			micro->io.P0.setAnalogValue(511);
			micro->sleep(100);
			micro->io.P0.setAnalogPeriodUs(0);
			if (recording) {
				 recorded[counter] = Csh;
				 counter ++;
			}

        }
        if (key == KEY_K10) {
            micro->io.P0.setAnalogPeriodUs(D);
			micro->io.P0.setAnalogValue(511);
			micro->sleep(100);
			micro->io.P0.setAnalogPeriodUs(0);
			if (recording) {
				 recorded[counter] = D;
				 counter ++;
			}
        }
        if (key == KEY_K2) {
            micro->io.P0.setAnalogPeriodUs(Efl);
			micro->io.P0.setAnalogValue(511);
			micro->sleep(100);
			micro->io.P0.setAnalogPeriodUs(0);
			if (recording) {
				 recorded[counter] = Efl;
				 counter ++;
			}
        }
        if (key == KEY_K11) {
            micro->io.P0.setAnalogPeriodUs(E);
			micro->io.P0.setAnalogValue(511);
			micro->sleep(100);
			micro->io.P0.setAnalogPeriodUs(0);
			if (recording) {
				 recorded[counter] = E;
				 counter ++;
			}
        }
        if (key == KEY_K12) {
            micro->io.P0.setAnalogPeriodUs(F);
			micro->io.P0.setAnalogValue(511);
			micro->sleep(100);
			micro->io.P0.setAnalogPeriodUs(0);
			if (recording) {
				 recorded[counter] = F;
				 counter ++;
			}
        }
        if (key == KEY_K3) {
            micro->io.P0.setAnalogPeriodUs(Fsh);
			micro->io.P0.setAnalogValue(511);
			micro->sleep(100);
			micro->io.P0.setAnalogPeriodUs(0);
			if (recording) {
				 recorded[counter] = Fsh;
				 counter ++;
			}
        }
        if (key == KEY_K13) {
            micro->io.P0.setAnalogPeriodUs(G);
			micro->io.P0.setAnalogValue(511);
			micro->sleep(100);
			micro->io.P0.setAnalogPeriodUs(0);
			if (recording) {
				 recorded[counter] = G;
				 counter ++;
			}
        }
        if (key == KEY_K4) {
            micro->io.P0.setAnalogPeriodUs(Afl);
			micro->io.P0.setAnalogValue(511);
			micro->sleep(100);
			micro->io.P0.setAnalogPeriodUs(0);
			if (recording) {
				 recorded[counter] = Afl;
				 counter ++;
			}
        }
        if (key == KEY_K14) {
            micro->io.P0.setAnalogPeriodUs(A);
			micro->io.P0.setAnalogValue(511);
			micro->sleep(100);
			micro->io.P0.setAnalogPeriodUs(0);
			if (recording) {
				 recorded[counter] = A;
				 counter ++;
			}
        }
        if (key == KEY_K5) {
            micro->io.P0.setAnalogPeriodUs(Bfl);
			micro->io.P0.setAnalogValue(511);
			micro->sleep(100);
			micro->io.P0.setAnalogPeriodUs(0);
			if (recording) {
				 recorded[counter] = Bfl;
				 counter ++;
			}
        }
        if (key == KEY_K6) {
            micro->io.P0.setAnalogPeriodUs(B);
			micro->io.P0.setAnalogValue(511);
			micro->sleep(100);
			micro->io.P0.setAnalogPeriodUs(0);
			if (recording) {
				 recorded[counter] = B;
				 counter ++;
			}
        }
        if (key == KEY_K7) {
            micro->io.P0.setAnalogPeriodUs(C);
			micro->io.P0.setAnalogValue(511);
			micro->sleep(100);
			micro->io.P0.setAnalogPeriodUs(0);
			if (recording) {
				 recorded[counter] = C;
				 counter ++;
			}
        }
        

        //micro->sleep(500);
    }



}