#Importing serial libraries
import serial.tools.list_ports
import serial
#Importing Keyboard execution library
from pynput.keyboard import Key, Controller
import time
keyboard = Controller()
#Listing all serial ports available
ports = serial.tools.list_ports.comports()
serial = serial.Serial()
portList = []
#Recursive For-loop to automatically assign 
#STM32 port by default
i = 0
for onePort in ports:
    i = i+1;
    if i == 1:
        initPort = str(onePort)
    portList.append(str(onePort))
    print(str(onePort))
#Baudrate initialize
#Make sure the STM32 have the same serial speed configured
serial.baudrate = 115200
#Assigning by default the STM32 port as main
if initPort[12] == ' ':
    mainport = str(initPort[0:12]);
else:
    mainport = str(initPort[0:13]);
serial.port = mainport;
#Opening serial port
serial.open()
#While-loop to maintain executing the script and the channel open
while 1:
    #Decoding information from the serial port
    if serial.in_waiting:
        key = serial.read().decode('ascii')
        #Conditions to press and release Keyboard keys
        if key == 'e':
            #Special case for 'enter'
            keyboard.press(Key.enter);
            time.sleep(0.005);
            keyboard.release(Key.enter);
            key = '\a';    
        #Press-Release condition for Left and Right
        if key == 'a':
            keyboard.release('d');
        elif key == 'd':
            keyboard.release('a');
        #Release condition in case it's sending nothing
        if key == "\n":
            keyboard.release('w');
            keyboard.release('a');
            keyboard.release('d');
        #Press command activation
        keyboard.press(key)
