#include "mbed.h"
#include "rtos.h"
#include "PinDetect.h"
#include "uLCD_4DGL.h"
#include "SDFileSystem.h"
#include "GPS.h"
// #include "icm20948.h"

/**
Game loop:
Keep a pushbutton function so that the game can be ended any time

1. Player A enters # zombie
    a. Give 10 seconds to enter the zombie numbers or choose 4 by default
2. Player B gets countdown
    a. Also displays # of zombies
3. Player B has to run (10 seconds)
    a. Keep a gps function thread that runs concurrently with the main thread
    b. During the 10 seconds, constantly take readings from GPS function and add to total_distance
    c. After the 10 seconds, if total_distance < input, "YOU GOT BIT :(", else "YOU SURVIVED!"
    d. Add a speaker to beep after 10 seconds

4. Result + cooldown = success or failure
    a. Keep for 5 seconds to combine with cool down
    b. Keep a pushbutton for player B to press to quit

6. Repeat
*/
 
uLCD_4DGL uLCD(p9,p10,p11);
// RawSerial gps(p13, p14);
RawSerial blue(p13, p14);
PinDetect pb(p8);
BusOut myled(LED1,LED2,LED3,LED4);
RawSerial  pc(USBTX, USBRX); // computer
PwmOut speaker(p26);
GPS gps(NC, p27);

union f_or_char {
    float f;
    char  c[4];
};

int quit_game = 0;

float run_time = 10; // seconds to run

// OPTION 1 -- GPS MODULE
char ns, ew, tf, status, c;
int fq, nst, fix, date;                                     // fix quality, Number of satellites being tracked, 3D fix
float latitude, longitude, old_lat, old_lon, timefix, speed, altitude;
char cDataBuffer[500];

float gps_reading = 0.0;
#define PI 3.14159
unsigned long p_buff[4];

// OPTION 2 -- IMU
float prevAccX = 0.0f, prevAccY = 0.0f, prevAccZ = 0.0f;

int count = 0;
Mutex lcd_mutex;

Timer timer;
int num_zombies = 4;

/*
0 = picking zombies
1 = running
*/
int game_mode = 0;

/**
Speeds to select from
*/

float s1 = 1.34;
float s2 = 1.52;
float s3 = 1.7;
float s4 = 1.85;
float s5 = 2.0;
float s6 = 2.2;
float s7 = 2.4;
float s8 = 2.6;

float input_speed = s4;

float ran;

/** SPEAKER TONES */
#define NOTE_C4  261.63
#define NOTE_E4  329.63
#define NOTE_G4  392.00
#define NOTE_A3  220.00
#define NOTE_E3  164.81
#define NOTE_D3  146.83

void quit(void) {
    myled[3] = 1;
    quit_game = 1;
}

// WELCOME SCREEN
void setup_screen(void) {
    lcd_mutex.lock();
    uLCD.background_color(WHITE);
    uLCD.cls();

    uLCD.color(GREEN);
    uLCD.text_width(2);
    uLCD.text_height(2);

    // Print the welcome message
    uLCD.printf("\n\nZOMBIE\nGAME");
    wait(5);
    lcd_mutex.unlock();
}

/** https://os.mbed.com/users/4180_1/notebook/adafruit-bluefruit-le-uart-friend---bluetooth-low-/ */
void blue_thread_button(const void args) {
    char bnum=0;
    char bhit=0;
    char bchecksum=0;
    char temp=0;
    union f_or_char x,y,z;
    float a;

    while(1) {
        while (!blue.readable()) {
            //pc.printf("Unreadable\n");
        }
        // pc.printf("Readable\n");
        // lcd_mutex.lock();        
        if (blue.getc()=='!') {
            myled[0] = 1;
            if ((blue.getc()=='B')) { //button data packet
                // pc.printf("Receiving button input\n");
                bnum = blue.getc(); //button number
                bhit = blue.getc(); //1=hit, 0=release
                myled[1] = 1;
                if (blue.getc()==char(~('!' + 'B' + bnum + bhit))) { //checksum OK?
                    myled = bnum - '0'; //current button number will appear on LEDs
                    switch (bnum) {
                        case '1': //number button 1
                            if (bhit == '1') {
                                input_speed = s1;
                                num_zombies = 1;
                                // myled[1] = 1;
                            } else {
                                // myled[1] = 0;
                            }
                            
                            // myled[1]=1;
                            break;
                        case '2': //number button 2
                            if (bhit == '1') {
                                input_speed = s2;
                                num_zombies = 2;
                                // myled[2] = 1;
                            } else {
                                // myled[2] = 0;
                            }
                            // myled[bnum-'1']=blue.getc()-'0';
                            break;
                        case '3': //number button 3
                            if (bhit == '1') {
                                input_speed = s3;
                                num_zombies = 3;
                                // myled[3] = 1;
                            } else {
                                // myled[3] = 0;
                            }
                            // myled[bnum-'1']=blue.getc()-'0';
                            break;
                        case '4': //number button 4
                            if (bhit == '1') {
                                input_speed = s4;
                                num_zombies = 4;
                                // myled[3] = 1;
                            } else {
                                // myled[3] = 0;
                            }
                            // myled[bnum-'1']=blue.getc()-'0';
                            break;
                        case '5': //button 5 up arrow
                            if (bhit=='1') {
                                input_speed = s5;
                                num_zombies = 5;
                            }
                            break;
                        case '6': //button 6 down arrow
                            if (bhit=='1') {
                                input_speed = s6;
                                num_zombies = 6;
                            }
                            break;
                        case '7': //button 7 left arrow
                            if (bhit=='1') {
                                input_speed = s7;
                                num_zombies = 7;
                            }
                            break;
                        case '8': //button 8 right arrow
                            if (bhit=='1') {
                                input_speed = s8;
                                num_zombies = 8;
                            }
                            break;
                        default:
                            input_speed = s4;
                            num_zombies = 4;
                            break;
                    }
                }
            }
        }
    }
}  

void zombie_select_screen() {
    lcd_mutex.lock();
    uLCD.background_color(WHITE); // set background color to white
    uLCD.cls(); // clear the screen
    Timer t1;

    // Set the font color and size
    uLCD.color(GREEN);
    uLCD.text_width(2);
    uLCD.text_height(2);

    // Print the message
    uLCD.printf("\n\n Select number\nof zombies...");
    lcd_mutex.unlock();

    t1.start();
    speed = s4;

    while (t1.read() < 10) {
        // collect player A's inputs from the blue_thread
        wait(0.1);
    }

    t1.stop();
    
}

void run_countdown_screen() {
    lcd_mutex.lock();
    uLCD.background_color(WHITE); // set background color to white
    uLCD.cls(); // clear the screen

    // Set the font color and size
    uLCD.color(GREEN);
    uLCD.text_width(2);
    uLCD.text_height(2);

    // Display the initial message
    uLCD.printf("\n\nThere are %d Zombies\n chasing you!", num_zombies);
    wait(1.0);

    speaker.period(1.0/500);
    speaker = 0.05;

    // Countdown loop
    for (int i = 5; i > 0; i--) {
        uLCD.cls(); // clear the screen
        uLCD.printf("\n\nThere are %d Zombies\n chasing you!\n\n %d", num_zombies, i);
        wait(0.5);
        speaker = 0.0;
        wait(0.5);
        speaker = 0.05;
    }

    speaker = 0.0;

    // Display final message
    uLCD.cls(); // clear the screen
    uLCD.printf("\n\nThere are %d Zombies\n chasing you!\n\n Go!", num_zombies);
    game_mode = 1;
    wait(1.0);
    lcd_mutex.unlock();
}

void running() {
    lcd_mutex.lock();
    uLCD.cls();
    uLCD.printf("\n\n    RUN!    \n\n");
    lcd_mutex.unlock();
    ran = 0.0; // distance covered by player B
    Timer t1;
    Timer t2;
    game_mode = 1;

    // wait(60);
    t1.start();
    while (t1.read() < run_time) {
        // pc.printf("Ran: %f", ran);
    }
    t1.stop();
    ran *= 10000;
    
    // pc.printf("Player B ran: %f\n", ran);
    game_mode = 0;

    lcd_mutex.lock();
    if (ran < (input_speed * run_time)/2) {
        uLCD.cls();
        uLCD.printf("\n\n   YOU GOT CAUGHT :(   \n\n");
        speaker.period(1.0 / NOTE_A3);
        speaker = 0.05; 
        wait(1);

        speaker.period(1.0 / NOTE_E3);
        wait(1);

        speaker.period(1.0 / NOTE_D3);
        wait(1);

        speaker = 0.0;

        wait(7);
        
    } else {
        uLCD.cls();
        uLCD.printf("\n\n   GOOD JOB! You reached safety   \n\n");
        speaker.period(1.0 / NOTE_C4);
        speaker = 0.05; 
        wait(1);
        
        speaker.period(1.0 / NOTE_E4);
        wait(1);
        
        speaker.period(1.0 / NOTE_G4);
        wait(1);

        speaker = 0.0;

        wait(7);
    }

    uLCD.cls();
    uLCD.printf("\n\n   COOLDOWN    \n\nPress button to Quit\n");
    t2.start();
    while (t2.read() < 5) {
        wait(0.1);
    }
    t2.stop();

    lcd_mutex.unlock();
    
}

//Read GPS to get current longitude and latitude and also calculate distance traveled
void readGPS() {
    while(1) {
        // if (gps) {
        lcd_mutex.lock();
        double latitude  = gps.latitude();
        double longitude = gps.longitude();
        double altitude  = gps.altitude();
        pc.printf("Latitude = %f  Longitude = %f  Altitude = %f\n\r",latitude, longitude,altitude);
        lcd_mutex.unlock();

        if(latitude != 0 && longitude != 0 && old_lat != 0 && old_lon != 0) {
            float a = sinf(old_lat)*sinf(latitude)+cosf(old_lat)*cosf(latitude)*cosf(longitude-old_lon);
            if(a > 1) a = 0.5;
            ran += (.75*acosf(a));
        }
        old_lat = latitude;
        old_lon = longitude;
    }
    Thread::wait(1000);
}
    
int main() {
    // Our code
    pb.mode(PullUp);
    pb.attach_deasserted(&quit);
    pb.setSampleFrequency();

    uLCD.cls();
    quit_game = 0;
    
    blue.baud(9600);
    gps.baud(9600);
    pc.baud(9600);

    

    speaker.period(1.0/500.0);

    Thread t1;
    Thread t2;

    t1.start(readGPS);
    t2.start(blue_thread_button);

    wait(3);

    setup_screen();

    // Wait for player A's input every 15 seconds
    while (quit_game == 0) {
        // // 1. Player A enters number of zombies
        // //      a. Give 10 seconds or enter 4 by default
        zombie_select_screen();

        // // 2. Player B gets countdown and # of zombies (3 secs)
        run_countdown_screen();

        // // 3. Player B has to run (10 seconds), produces results, keeps cooldown
        // // Reset player A input to 4 zombies.
        running();
    }
    
    lcd_mutex.lock();
    uLCD.cls();
    uLCD.color(GREEN);
    uLCD.text_height(1);
    uLCD.text_width(1);
    uLCD.printf("\n\nThanks For Playing!\n\n");
    lcd_mutex.unlock();

    return 0;
}
