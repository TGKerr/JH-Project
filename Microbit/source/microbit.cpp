#include "keyboard.h"
//Set up global variables for keyboard usage.
bool setup = false; 
int selected_case = 0;
int selected_char = 0;
int selected_dest = 0;
int messageLength = 0;
int ScrollSpeed = 100;
const int maxMessage = 50;
//Define Image of lock used when the microbit is locked
MicroBitImage lock("0,255,255,255, 0\n0,255,0,255,0\n255,255,255,255,255\n255,255,0,255,255\n255,255,255,255,255\n"); 
char* locked;
char** filters;
int filterSize = 0;
char* username;
mBits* foundUBits;
int counter;
int messageIDs[5];

// Initialise the keyboard variables, assigning memory space needed for sending messages.
void initKey(){
	setup = true;
	locked = (char*) malloc(sizeof(char*) * maxMessage);
	locked[0] = '\0';
	filters = (char**) malloc(sizeof(char*) * 5);
	username = (char*) malloc(sizeof(char*) * 20);
	username[0] = '\0';
	setup = true;
	foundUBits = (mBits*) malloc(sizeof(mBits));
	foundUBits->name = (char*) malloc(sizeof(char) * 20);
	foundUBits->name = "Broadcast";
	(foundUBits->address) = 0;
	foundUBits->next = NULL;
	foundUBits->prev = foundUBits;
	for(int i = 0; i < 5; i++){
		messageIDs[i] = NULL;
	}
}

//Check If message contains words filtered for
int filtered(char* received){
    char* local = (char*) malloc(sizeof(char)* maxMessage);	//Create a copy of the message.
    strcpy(local, received);
    //micro->display.scroll("F");
    char* word;
    word = strtok(local, " ");		//Check each word of the received message.
    while(word != NULL){
        //micro->display.scroll(word, 50);
        for(int i = 0; i < filterSize; i++){
            if(strcmp(word, filters[i]) == 0){	//If worrd is equal to a filtered term then don't display it.
                free(local);
                return 1;
            }
        }
        word = strtok(NULL, " ");
    }
    free(local);	//Free allocated memory.
    return 0;
}

//Check incoming message to see if it is a request by examing the first word
//If the first word is one of those specified return 1
//If not, return 0
int appCheck(char* message){
    char* local = (char*) malloc(sizeof(char)* maxMessage);	//Allocate space for message.
    strcpy(local, message);	
    char* firstWord = strtok(local, " ");	//Split by space - check first word is News.
    if(strcmp(firstWord, "News") == 0){
        free(local);
        return 1;
    }
	if(strcmp(firstWord, "Weather") == 0){
        free(local);
        return 1;
    }
	if(strcmp(firstWord, "Time") == 0){
        free(local);
        return 1;
    }
	if(strcmp(firstWord, "Music") == 0){
		parseMusicMessage(&message[6]); 
        free(local);
        return 0;
    }
    free(local);
    return 0;
}

//Checks that the discovery flag is set in incoming message.
bool discovery(byte_t flags){
	return (flags & (1)) == (1);
}

//Set flag
bool ref(byte_t flags){
	return (flags & 1 << 1) == (1 << 1);
}

//Checks that the ack flag is set in incoming message.
bool ack(byte_t flags){
	return (flags & 1 << 2) == (1 << 2);
}

//Check if Encrypted
bool checkEncrypt(byte_t flags){
	return (flags & 1 << 3) == (1 << 3);
}

//Print a uBit name - for debug.
void printUBit(mBits* found){
	micro->serial.send(found->name, SYNC_SPINWAIT);
}

//Add a newly discovered uBit.
void addToEnd(mBits* list, mBits* toAdd){
	mBits* listTemp = list;
	while(listTemp->next != NULL){
	 	if(listTemp->address == toAdd->address){
			free(toAdd->name);
			free(toAdd);
			return;
		 } 
		listTemp = listTemp->next;	//Get end of list - add new node.
	}
	listTemp->next = toAdd;
	return;
}

//Add MicroBit
void addMBit(CRadioPacket* packet){
	mBits* found = (mBits*) malloc(sizeof(struct mBits));
	found->name = (char*) malloc(sizeof(char) * packet->bodyLength);
	found->name = packet->body;
	address_t address = packet->source;
	found->address = address;
	found->next = NULL;
	foundUBits->prev = found;
	addToEnd(foundUBits, found);
}

//Check if microbit is known, return true/false
bool knownMicro(address_t address){
	mBits* listTemp = foundUBits;
	while(listTemp->next != NULL){
	 listTemp = listTemp->next;	//Get end of list - add new node.
	 	if(listTemp->address == address) return true;
	}
	return false;
}

//Handle Incoming Message
//If filter size is 0 or (message cotains word in filter array and is > 0)
//Display the Message
void handleMessage(char* message, int length){
	 if(filterSize == 0 || filtered(message) && length > 0){
		micro->display.scroll("Received: ", ScrollSpeed);
		// char* addressBuf = (char*) malloc(sizeof(char) * 40);
		// char* locaddressBuf = (char*) malloc(sizeof(char) * 40);
		// sprintf(locaddressBuf, "%d", getUBitAddress());
		// sprintf(addressBuf, "%d", packet->destination);
		// micro->display.scroll(locaddressBuf, 50);
		// micro->display.scroll(addressBuf, 50);
        micro->display.scroll(message, ScrollSpeed);
        micro->sleep(1000);
    }
}

//Check if Id was recently seen (i.e. in messagesIDs array)
bool recentlySeen(messageId_t id){
	for(int i = 0; i < 5; i++){
		if(id == messageIDs[i]) return true;
	}
	return false;
}

//Upon receiving a radio message.
void onData(MicroBitEvent e)
{
    CRadioPacket* packet = (CRadioPacket*) malloc(sizeof(CRadioPacket)); 		//Assign memory for an incoming packet.
    PacketBuffer buff =  micro->radio.datagram.recv();							//Read input from radio.
	char* s = (char*) buff.getBytes();
    parsePacket(s, packet);														//Parse bytes into malloc'd memory.
	if(recentlySeen(packet->messageId)){										//Discard already encountered messages.
		free(packet);
		return;
	}
	messageIDs[counter] = packet->messageId;									//Add meesage to already seen array.
	counter = (counter + 1) % 5;

	if(packet->destination != 0 && packet->destination != 5 && packet->destination != getUBitAddress()){    //Message not for local uBit- send on.
		if(packet->depth > 1){
		 	//sendTextMessage(micro, packet->destination, packet->depth-1, packet->body, packet->bodyLength);
			sendMessageDetail(micro, packet->destination, packet->source, packet->depth-1, checkEncrypt(packet->flags), ack(packet->flags), ref(packet->flags), discovery(packet->flags), packet->messageId, packet->body, packet->bodyLength);

		}
		free(packet);
		return;
	}
	if(checkEncrypt(packet->flags)){    //Message Encrypted - handle appropriately
		//micro->display.scroll("here", 30);
		handleEncryptedMessage(*packet);
		free(packet);
		return;
	}
	if(discovery(packet->flags) && ack(packet->flags)){ //Upon receiving an acknowledgement
		micro->display.scroll("Ack", 30);
		addMBit(packet);
		if(getChannel(packet->source) == NULL) openChannel(packet->source);
		free(packet);
		return;
	}
	if(discovery(packet->flags) && !ack((packet->flags))){  //Upon receiving a discovery message.
		// if(!knownMicro(packet->source)){
		// 	sendDiscoveryMessage(micro, 1);
		// }
		sendMessageDetail(micro, packet->source, getUBitAddress(), 1, false, true, false, true, generateMessageId(micro), "C5-Micro", 8);
		free(packet);
		return;
	}
    if(packet->destination == 5 && !checkEncrypt(packet->flags) && appCheck(packet->body)){ //Check message is for a specific application.
		micro->serial.clearTxBuffer();
		micro->serial.clearRxBuffer();
        micro->serial.send(packet->body, SYNC_SPINWAIT);
		micro->sleep(5000);
        char* news = (char*) micro->serial.read(micro->serial.rxBufferedSize() ,ASYNC).toCharArray();
		micro->display.scroll(news, 20);
        if(strlen(news) > 0)sendTextMessage(micro, packet->source, 1, news, strlen(news)+1);
		//micro->serial.clearRxBuffer();
        free(packet);
        return;
    }
    if(packet->destination == getUBitAddress()) handleMessage(packet->body, packet->bodyLength);    //Check message is directly for this uBit.
    free(packet);
}


//Add the selected character to the message.
void add_char(char* message, ManagedString uppercase, ManagedString lowercase, ManagedString symbols){
    if(selected_case == 0){	//If uppercase.
        message[messageLength] = uppercase.charAt(selected_char);
    }
    if(selected_case == 1){	//If lowercase.
        message[messageLength] = lowercase.charAt(selected_char);
    }
    if(selected_case == 2){	//If symbols.
        message[messageLength] = symbols.charAt(selected_char);
    }
    messageLength++;	//Increment message length.
    micro->display.print('Y');	//Print success char.
    message[messageLength] = '\0';	//Set end of string char.
    micro->sleep(500);	//Sleep
}

//Delte last letter added to the message
void del_char(char* message){
    if (messageLength > 0){	//Set last char to string end.
        message[messageLength-1] = '\0';
        messageLength--;	//Decrement message length.
    }
    micro->display.print('N');	//Print character deleted notification.
    micro->sleep(500);

}

//Changes case of keyboard dependign upon what pin is pressed.
void updateCase() {	
    if (micro->io.P0.isTouched()){
        selected_case = 0;
    }
    if (micro->io.P1.isTouched()){
        selected_case = 1;
    }
    if (micro->io.P2.isTouched()){
        selected_case = 2;
    }

}


//Display the appropriate character depending upon what case is selcted and at what index the user is currently at.
void updateDisplay(ManagedString uppercase, ManagedString lowercase, ManagedString symbols){
    if(selected_case == 0){
        micro->display.print(uppercase.charAt(selected_char));
    } else if(selected_case == 1){
        micro->display.print(lowercase.charAt(selected_char));
    } else if(selected_case == 2){
        micro->display.print(symbols.charAt(selected_char));
    }
    micro->sleep(100);

}

//General modulo function.
int modulo(int x, int y){
    return (x % y + y) % y;	
}

//Lock Screen
//Display lock Image while user does not unlock device
int lockScreen(){
    while (micro->accelerometer.getX() < 1000){	
        micro->display.print(lock);
    }
    return 1;

}

//Send Radio News Request
void radioNewsRequest(char* message){
    sendTextMessage(micro, 5, 1, message, strlen(message)+1); //Send new request.
    return;
}

//Making a news request, if succesfull display news
void newsRequest(char* message){
	char* news = (char* ) malloc(sizeof(char) * maxMessage + 5);	//Allocate space for "News " to be added to start of message.
    strcpy(news, "News ");	//Copy news to a new string.
    if(username[0] == '\0'){	//If no username is set, then add the current message to the end of the created News request.
		strcat(news, message);
	} else {
		strcat(news, username);
	}
	micro->serial.clearTxBuffer();
    micro->serial.send(news, SYNC_SPINWAIT);	//Attemp to send across serial.
	micro->sleep(500);
    if(micro->serial.rxBufferedSize() < 25){	//If fails - make radio news request.
        micro->display.scroll(news, ScrollSpeed);
        radioNewsRequest(news);
    }
    while(micro->serial.rxBufferedSize() != 0){	//If succeeds print news.
        //if(micro->serial.rxBufferedSize() == 0) break;
        micro->display.scroll(micro->serial.read(20,ASYNC), ScrollSpeed);
    }
	free(news);
}

//Check if filter may be added, then ad filter to filter array
void addFilter(char* message){
    if(filterSize > 5){	//Allowed up to 5 filters
        micro->display.scroll("Too many filters");
    }
    //Check if input is already in filter array
	for(int i = 0; i < filterSize; i++){
		if(filters[i] == NULL) break;
		if(strcmp(filters[i], message) == 0){
			free(filters[i]);
			filters[i] = NULL;
			filterSize--;
			return;
		}
	}
    char* filter  = (char*) malloc(sizeof(char) * maxMessage);	//Allocate space for filter.
    strcpy(filter, message);	//Copy message into filter.
    filters[filterSize] = filter;	//Add filter to array.
    micro->display.scroll(filters[filterSize], ScrollSpeed);	//Display the filter.
    filterSize++;	//Increment counter.
}

//When sending a message choose a recipient.
void sendMessage(char* message){
	micro->sleep(500);
	mBits* selected = foundUBits;
	while(true){	
	    //Select destination by looping through choices using A and B buttons
		int y_tilt = micro->accelerometer.getY();
		if(micro->buttonB.isPressed()){
			if(selected->next == NULL){
				selected = foundUBits;
			} else {
				selected = selected->next;
			}
		}
		if(micro->buttonA.isPressed()){
				selected = selected->prev;
		}
		//If the microbit is tilted forward send text message
		if(y_tilt < -500){
		    //If selected address = 0 send text message
			if(selected->address == 0){
				sendTextMessage(micro, selected->address, 3, message, strlen(message)+1);
			}
			//Else decide if the message should be sent in encrypted or unencrypted version 
			micro->display.scroll("A Encrypted | B Unencrypted", 50);
			while(true){
				if(micro->buttonA.isPressed()){
					sendMessageOverChannel(getChannel(selected->address), message, strlen(message)+1);
					return;
				}
				if(micro->buttonB.isPressed()){
					sendTextMessage(micro, selected->address, 3, message, strlen(message)+1);
					return;
				}
			}
		}
		micro->display.scrollAsync(selected->name, 30);
		micro->sleep(100);
	}
}

//Choose what to do with keyboard input from a menu
//Call correct function associated to option picked
void messageLocation(ManagedString destinations, char* message, int* messageLength){
    int chosenDest = 0;
    while(true){
        int y_tilt = micro->accelerometer.getY();
        //Display currently selected option
        micro->display.print(destinations.charAt(chosenDest));
        //Move left/right when the user presses the A or B button
        if(micro->buttonA.isPressed()){
            chosenDest = modulo(chosenDest+1, 6);
            micro->sleep(100);
        }
        if(micro->buttonB.isPressed()){
            chosenDest = modulo(chosenDest-1, 6);
            micro->sleep(100);
        }
        //If both A and B button are pressed simultanoues display full name of current option
		if(micro->buttonA.isPressed() && micro->buttonB.isPressed()){
			if(chosenDest == 0) micro->display.scroll("MicroBit", ScrollSpeed);
			if(chosenDest == 1) micro->display.scroll("News", ScrollSpeed);
			if(chosenDest == 2) micro->display.scroll("Filter", ScrollSpeed);
			if(chosenDest == 3) micro->display.scroll("Lock", ScrollSpeed);
			if(chosenDest == 4) micro->display.scroll("Quit", ScrollSpeed);
			if(chosenDest == 5) micro->display.scroll("Set Username", ScrollSpeed);
		}
		//If microbit is tilted forward then call the relevant function 
        if (y_tilt < -500){
            if(chosenDest == 0){  //Micro
				sendMessage(message);
                micro->sleep(100);
                message[0] = '\0';
                *messageLength = 0;
                return;

            }
            if(chosenDest == 1){        //News
                newsRequest(message);
                 message[0] = '\0';
                *messageLength = 0;
                micro->sleep(100);
                return;
            }
            if(chosenDest == 2){        //Filter
                addFilter(message);
                message[0] = '\0';
                *messageLength = 0;
                micro->sleep(100);
                return;
            }
            if(chosenDest == 3){        //Lock
                lockScreen();
				strcpy(locked, message);
                message[0] = '\0';
                *messageLength = 0;
                micro->sleep(100);
                return;
            }
			if(chosenDest == 4){		//Quit
				message[0] = '\0';
				*messageLength = 0;
				micro->sleep(100);
				menu(ScrollSpeed);
			}
			if(chosenDest == 5){		//Set Username
				strcpy(username, message);
				message[0] = '\0';
				*messageLength = 0;
				micro->display.scroll(username);
				micro->sleep(100);
			}
        }
        if (y_tilt > 500){
            return;
        }
        
        
        micro->sleep(100);
    }
}

//Check if message is correct pasword
void checkUnlock(char* message){
	if(strcmp(message, locked) == 0){
		locked[0] = '\0';
		micro->display.scroll('Y');
	}else {
		micro->display.scroll('N');
	}
	message[0] = '\0';
	messageLength = 0;
}

//Displays Keyboard, once user has decided on input calls relevant functions
int keyboard(MicroBit* micro, int speed) {
    ScrollSpeed = speed;
	char* message;
	int x_tilt, y_tilt;

	if(!setup){
		message = (char*) malloc(sizeof(char) * maxMessage);
		message[0] = '\0';
	}	

    //Define Keyboard arrays 
    ManagedString uppercase("ABCDEFGHIJKLMNOPQRSTUVWXYZ");
    ManagedString lowercase("abcdefghijklmnopqrstuvwxyz");
    ManagedString symbols(" 0123456789.,\'?!()/\":+-=£@");
    //Define options menu
    ManagedString destinations("MNFLQU");
    

    while(1) {
        
		micro->sleep(500);

        updateDisplay(uppercase, lowercase, symbols);

        //Get current accelerometer values
        x_tilt = micro->accelerometer.getX();
        y_tilt = micro->accelerometer.getY();

        //If A and B are pressed check if microbit is locked
        //If microbit is locked then check if input is pasword
        //If microbit is unlocked then send call user sepcified fuction
        if(micro->buttonA.isPressed() && micro->buttonB.isPressed()){
			if(locked[0] != '\0') checkUnlock(message);
             else messageLocation(destinations, message, &messageLength);
        }

        //Move to letter on the left of current letter, loop if on th edge
        else if (micro->buttonA.isPressed()){
            selected_char = modulo(selected_char-1, 26);
            //micro->display.scroll(selected_char);
            updateDisplay(uppercase, lowercase, symbols);
        }

        //Move to the letter on the right of current letter, loop if on the edge
        else if (micro->buttonB.isPressed()){
            selected_char = modulo(selected_char+1, 26);
            updateDisplay(uppercase, lowercase, symbols);
            //micro->display.scroll("Received");
            // CRadioPacket* packet = (CRadioPacket*) malloc(sizeof(CRadioPacket)); 
            // char* s = (char* ) micro->radio.datagram.recv().getBytes();
            // parsePacket(s, packet);
            //micro->display.scroll(packet->body);
        }


        updateCase();
        
        //Add currently selected letter if microbit is tilted forward
        if (y_tilt < -700 && messageLength < maxMessage-1){
            add_char(message, uppercase, lowercase, symbols);
        }

        //Deleted currently selected letter if microbit is tilted backward
        if (y_tilt > 700)
            del_char(message);
            
        //Display current input if the microbit is tilted to the right
        if (x_tilt > 500)
            micro->display.scroll(message, ScrollSpeed);
            
        //Add a space if the microbit is tilted to the left    
        if (x_tilt < -500 && messageLength < maxMessage-1){
            message[messageLength] = ' ';
            messageLength++;
            message[messageLength] = '\0';
            micro->display.print('S');
            micro->sleep(500);
        }

    }
}
