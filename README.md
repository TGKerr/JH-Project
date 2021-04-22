# Micro:Bit - Group C5

## Created By:
#### Rachel Speirs, Kirsty McGuire, Tommy Kerr, Ellie Fowler, Max Triebnigg

## Instruction For Getting Started
### Prerequisites
Install serial for communication between the micro:bit and computer
```
pip install --user serial
```

Install requests for sending http requests
```
pip install --user requests
```

Install microfs for communication with microbit
```
pip install --user microfs
```

### Setting up Database
Create a database using the dump file provided in the main folder.

If the server is running on localhost, there are no changes which need to be made. 
However, if not, lines 17 and 39 of “/website/JS/Client.js” will need to be changed from “localhost” to the hosted location. 
“/Python Middleware/json_io.py” lines 29, 35 and 45 will need to be changed to the hosted location instead of "127.0.0.1" as well.

### Building the Code
1. Set up yotta
2. Execute yotta build in the "/Microbit" folder
3. Put "/Microbit/build/bbc-microbit-classic-gcc/source/JH-combined.hex" onto the micro:bits

## To Run
1. Execute the file “/Python Middleware/json_io.py”
2. In a new terminal execute the file "server.js"
3. Have at least one micro:bit plugged into the computer running the server
4. To use the micro:bit refer to the file "micro:bit Manual"