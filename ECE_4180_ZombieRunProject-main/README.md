# ECE 4180 Final Project
Final Project for ECE 4180, Spring 2024 semester  
Group members: Binish Patel, Mohammed Rahil Hussain  

## Zombie Run Project Overview

Our project is about an immersive Zombie Run game wherein a player (player A) inputs the number of ‘zombies’ that are chasing after another player (player B) on the Bluefruit app. Player B must cover a certain distance to avoid the zombies within a 1 minute time frame. If player B wins, then a happy tune will play and the uLCD will print a game-won message. However, if player B does not outrun the zombies, the speaker will play a sad tune and the uLCD will show a game-over message.
The Bluefruit module connected to the mbed will receive one of 8 button inputs from the app. The uLCD will then print the number of ‘zombies’ selected. The Adafruit's latitude and longitude inputs are used in concert with the haversine formula to calculate how far Player B has run. We set up an RTOS thread to continually take readings from the Bluetooth module depending on the game mode (if game mode is 0, then that means that currently player A is selecting the number of zombies so we look out for 'B' or button packets. If game mode = 1, then player B is running, so we look out for the location packets 'L'.)  
There is also a 5 second countdown before the run starts to give players time to prepare and a 10 second cooldown between rounds to allow player B to rest.  
A pushbutton in the circuit allows player B to quit whenever they choose.

![Breadboarding](/breadboard.jpg)

## Components

-	MBED LPC1768 Microcontroller
-	Adafruit Bluefruit LE UART Friend - Bluetooth Low Energy (BLE)
-	uLCD-144-G2 128 by 128 Smart Color LCD
-	Speaker and Class D high-efficiency audio amp
-	Pushbutton Switch
-	Battery Holder and 4 AA Cells

### MBED LPC1768 Microcontroller
![MBED LPC1768 Microcontroller](https://os.mbed.com/media/uploads/jhawkins38/mb.jpg)

### Adafruit Bluefruit LE UART Friend - Bluetooth Low Energy (BLE)
![Adafruit Bluefruit LE UART Friend - Bluetooth Low Energy (BLE)](https://os.mbed.com/media/uploads/4180_1/ble.jpg)

| mbed | Adafruit BLE | Power Supply |
| --- | --- | --- |
| | gnd | - |
| | Vin (3.3-16V) | + |
| nc | RTS | |
| | CTS | - |
| p27 (Serial RX) | TXO | |
| p28 (Serial TX) | RXI | |


### uLCD-144-G2 128 by 128 Smart Color LCD
![uLCD-144-G2 128 by 128 Smart Color LCD](https://os.mbed.com/media/uploads/4180_1/8185.png)

| mbed | uLCD Header | uLCD cable | Power Supply | 
| ---- | ----------- | ---------- | --- |
| | 5V | 5V | + |
| |	Gnd |	Gnd | - |
| TX=P9	| RX	| TX | |
| RX=P10 | TX |	RX | |
| P11	| Reset	| Reset | |

### Speaker and Class D high-efficiency audio amp 

![Speaker and Class D audio amp](https://os.mbed.com/media/uploads/4180_1/_scaled_classdbreakout.jpg)

| mbed | TPA2005D1 | Speaker | Power Supply |
| ---- | --- | --- | --- |
| | pwr -(gnd), in - | | - |
| | pwr+ | | + |
| p26 | in + | | |
| | out+ | + | |
| | out- | - | |

### Pushbutton Switch

![Pushbutton Switch](https://cdn.sparkfun.com/assets/parts/2/6/2/9/09190-03-L.jpg)

| mbed | Switch |
| ---- | --- |
| p8 | + |
| gnd | - |

### GT-U7 GPS Module

![GPS Module](https://www.elecbee.com/image/cache/catalog/Smart-Module/GT-U7-Car-GPS-Module-Navigation-Satellite-Positioning-Geekcreit-for-Arduino---products-that-work-wit-1354130-183-500x500.jpeg)

| mbed | GT-U7 | Power supply |
| --- | --- | --- |
| p27 | TX | |
| | VCC | + |
| | gnd | - |

### Battery Holder and 4 AA Cells
![Battery Holder](https://mm.digikey.com/Volume0/opasdata/d220001/medias/images/4846/MFG_1528_3859.jpg)

## Software and Code

We utilized Keil Arm Studio (for C++) https://studio.keil.arm.com/ to write our code. In addition to, we utilized RTOS threads to continuously collect user input and calculate distance traveled every ms and for the uLCD screen, along with appropriate mutex locks. For the positioning, we made use of the Haversine formula to find location of Player B using latitude and longitude. Multiple functions for different LCD screens with timers were used and we included a push button for a QUIT option.

![Start Screen](/home_screen.jpg)

![Select Screen](/select.jpg)

## Video Demo

[![IMAGE ALT TEXT HERE](https://img.youtube.com/vi/o2BPSTvk0WI/0.jpg)](https://www.youtube.com/watch?v=o2BPSTvk0WI)

## Future Work

We are looking into incorporating more interactive features to gauge multiple users interest. To that end, we plan on incorporating a precise and faster GPS to quickly and sensitively measure users exact position. Furthermore, we will look into adding larger touch screens for both players with player switching between every round to add game immersion. We also would love to make the game widely available on Fitness Devices such as Apple Watch, Galaxy Watch, etc. In addition to that, we were thinking of adding support for smart treadmills to motivate people to take the first step when they're trying to start their fitness journey.
