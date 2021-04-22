#include "menu.h"

MicroBit microB;		//Global Variables for settings and displaying.
MicroBit* micro;
int speedScroll;
int brightness;
char* userName;
address_t local_address;
MicroBitImage light("0,255,0,255, 0\n255,0,255,0,255\n0,255,0,255,0\n255,0,255,0,255\n0,255,0,255,0\n");
MicroBitImage speedIm("0,0,255,0, 0\n0,0,0,255,0\n255,255,255,255,255\n0,0,0,255,0\n0,0,255,0,0\n");


int main(){			// On startup - 
	micro = &microB;			//Set pointer to Micro:Bit.
    microB.init();				//Initialises all needed Micro:Bit functionality.
	initKey();					//Initialses the keyboard functionality.
	initRadio(&microB);			//Initialises the radio functionality.
    startup();					//Plays the startup animation.
	microB.messageBus.listen(MICROBIT_ID_RADIO, MICROBIT_RADIO_EVT_DATAGRAM, onData, MESSAGE_BUS_LISTENER_QUEUE_IF_BUSY);		//Listens for incoming radio messages.
	sendDiscoveryMessage(&microB, 1);		//Sends out a discovery message to ask for all microbit ID's within range.
    microB.serial.setRxBufferSize(2000);		//Sets serial buffer to max size.
	speedScroll = 100;						//Sets standard scroll speed to 100.
	menu(speedScroll);						//Enters Main menu.
}

void piano(){					//Piano application
	microB.sleep(500);
	ManagedString menu = "RPS";	//R - Record, P - Play, S - Send
	bool record;
	int index = 0;
  	while(true){	//Loop for user inputs
        int y_tilt = microB.accelerometer.getY();
        if(microB.buttonB.isPressed()) index = modulo(index+1, 3);	//Right on B
        if(microB.buttonA.isPressed()) index = modulo(index-1, 3);  //Left on A
		if(microB.buttonA.isPressed() && microB.buttonB.isPressed()){
			if(index == 0) microB.display.scroll("Record", speedScroll);
			if(index == 1) microB.display.scroll("Play", speedScroll);
			if(index == 2) microB.display.scroll("Send Song", speedScroll);
		} 
        microB.display.print(menu.charAt(index));		//Display Char selected.
        if(y_tilt < -700){
            if(index == 0) pianoMain(&microB, true);	//Enter app selected.
            if(index == 1) pianoMain(&microB, false);
			if(index == 2) chooseSong(&microB, speedScroll);
			return;
        }
        if(y_tilt > 700){	//Return;
            break;
        }
        microB.sleep(200);
    }
	microB.sleep(500);
}

void menu(int speedScroll){	//Home menu
	microB.sleep(500);
	ManagedString menu = "KSETWPD";	//K - Keyboard, S - Settings, E - Email, T - Time, W - Weather, P - Piano
	int index = 0;
    while(true){		//While nothing selected, constantly check user input.
        int y_tilt = microB.accelerometer.getY();
        if(microB.buttonB.isPressed()) index = modulo(index+1, 7);	//Move right, back to start if off end.
        if(microB.buttonA.isPressed()) index = modulo(index-1, 7);  //Move left, to end if move off end.
        microB.display.print(menu.charAt(index));	//Print the currently selected app.
        if(y_tilt < -700){
            if(index == 0) keyboard(&microB, speedScroll);
            if(index == 1) settings();
            if(index == 2) break;
            if(index == 3) time();
			if(index == 4) weather();
			if(index == 5) piano();
			if(index == 6) sendDiscoveryMessage(&microB, 1);
        }
		if(microB.buttonB.isPressed() && microB.buttonA.isPressed()){		//Press A and B to display currently selected app's full name.
			if(index == 0) microB.display.scroll("Keyboard", speedScroll);
            if(index == 1) microB.display.scroll("Settings", speedScroll);;
            if(index == 2) microB.display.scroll("Exit", speedScroll);
            if(index == 3) microB.display.scroll("Time", speedScroll);
			if(index == 4) microB.display.scroll("Weather", speedScroll);
			if(index == 5) microB.display.scroll("Piano", speedScroll);
			if(index == 6) microB.display.scroll("Discovery", speedScroll);

		}
        // if(y_tilt > 700){		//If tilt forward - turn off.
        //     break;
        // }
        microB.sleep(200);
    }
    microB.display.scroll("Goodbye!", speedScroll);
    release_fiber();
}


void time(){		//Get time across Serial.
	micro->serial.clearTxBuffer();
    microB.serial.send("Time");
	microB.display.scroll("Time: ", speedScroll);
    microB.sleep(1000);
	if(microB.serial.rxBufferedSize() == 0){
		char* time = "Time \0";
		sendTextMessage(&microB, 5, 1, time, 6);
	}
    else microB.display.scroll(microB.serial.read(microB.serial.rxBufferedSize(), SYNC_SPINWAIT), speedScroll);
}

void weather(){		//Get weather across serial.
	micro->serial.clearTxBuffer();
    microB.serial.send("Weather");
	microB.display.scroll("Weather: ", speedScroll);
    microB.sleep(1000);
	if(microB.serial.rxBufferedSize() == 0){
		char* weather = "Weather \0";
		sendTextMessage(&microB, 5, 1, weather, 9);
	}
    else microB.display.scroll(microB.serial.read(microB.serial.rxBufferedSize(), SYNC_SPINWAIT), speedScroll);
}

// void email(){		//Get email - Likely going to remove.
//     microB.serial.send("Email ");
//     microB.sleep(1000);
//     microB.display.scroll(microB.serial.read(microB.serial.rxBufferedSize(), SYNC_SPINWAIT), speedScroll);
// }

int brightnessSetting(){	//Change brightness setting.
    brightness = microB.display.getBrightness();
    microB.sleep(100);
    while(true){
        int y_tilt = microB.accelerometer.getY(); 
        if(microB.buttonB.isPressed()) brightness = modulo(brightness+1, 256);	//Scroll through brightness options.
        if(microB.buttonA.isPressed()) brightness = modulo(brightness-1, 256);
        if(y_tilt > 700) break;							//Tilt back to quit.
        microB.display.setBrightness(brightness);		//Set brightness to selected value.
        microB.display.print('A');						//Display a sample character at that brightness.
        microB.sleep(20);
    }
    return 1;
}

int speed(){		//Change scroll speed.
    speedScroll = 100;	//Set to default speed.
    ManagedString test = "This is a test message";	//Test string for options.
    while(true){
        int y_tilt = microB.accelerometer.getY();
        if(microB.buttonB.isPressed()) speedScroll = modulo(speedScroll+1, 500);	//Scroll through speed options.
        if(microB.buttonA.isPressed()) speedScroll = modulo(speedScroll-1, 500);
        if(microB.buttonB.isPressed() && microB.buttonA.isPressed()) microB.display.scroll(test, speedScroll);	//Display sample text at that speed.
        if(y_tilt > 700) break;			//Tilt back to quit.
        microB.display.scrollAsync(speedScroll);
        microB.sleep(20);
    }
    return 1;
}

void settings(){		//Settings Application
    int index = 0;
    microB.sleep(500);
    while(true){		//Same as main menu, but now selects options settings.
        int y_tilt = microB.accelerometer.getY();
        if(microB.buttonB.isPressed()) index = modulo(index+1, 2);
        if(microB.buttonA.isPressed()) index = modulo(index-1, 2);
		if(microB.buttonA.isPressed() && microB.buttonB.isPressed()){
			if(index == 0) microB.display.scroll("Scroll Speed", speedScroll);
			if(index == 1) microB.display.scroll("Brightness", speedScroll);
		}
        if(y_tilt < -700){
            if(index == 0) speed();
            if(index == 1) brightnessSetting();
            microB.sleep(500);
        }
        if(y_tilt > 700){
            break;
        }
        if(index == 0) microB.display.print(speedIm);	//Display image for setting.
        if(index == 1) microB.display.print(light);
        microB.sleep(250);
    }
}


/*
Loops through three images to make a basic startup animation.
*/
void startup(){
	local_address = getUBitAddress();
	encChannelInit(micro);		//Sets up key information.
	microB.serial.clearTxBuffer();
    MicroBitImage startOne("0,0,0,0, 0\n0,0,0,0,0\n0,0,255,0,0\n0,0,0,0,0\n0,0,0,0,0\n");
    MicroBitImage startTwo("0,0,0,0, 0\n0,255,255,255,0\n0,255,0,255,0\n0,255,255,255,0\n0,0,0,0,0\n");
    MicroBitImage startThree("255,255,255,255,255\n255,0,0,0,255\n255,0,0,0,255\n255,0,0,0,255\n255,255,255,255,255\n");
	// microB.serial.send("Clearing");
	// microB.sleep(200);
	// microB.serial.read(microB.serial.rxBufferedSize());
    for(int i = 0; i < 3; i++){
        microB.display.print(startOne);
        microB.sleep(100);
        microB.display.print(startTwo);
        microB.sleep(100);
        microB.display.print(startThree);
        microB.sleep(100);
    }
}