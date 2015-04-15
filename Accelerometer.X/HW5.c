//This code is to demonstrate the use of accelerometer and draw the g reading on the led screen
//Author: Michael Sun 
//Date: April 15 2015

#include"i2c_display.h"
#include"char_lib.h"
#include<xc.h> // processor SFR definitions
#include<sys/attribs.h> // __ISR macro
#include"accel.h"

// DEVCFG0
#pragma config DEBUG = OFF // no debugging
#pragma config JTAGEN = OFF // no jtag
#pragma config ICESEL = ICS_PGx1 // use PGED1 and PGEC1
#pragma config PWP = OFF // no write protect
#pragma config BWP = OFF // not boot write protect
#pragma config CP = OFF // no code protect

// DEVCFG1
#pragma config FNOSC = PRIPLL // use primary oscillator with pll
#pragma config FSOSCEN = OFF // turn off secondary oscillator
#pragma config IESO = OFF // no switching clocks
#pragma config POSCMOD = HS // high speed crystal mode
#pragma config OSCIOFNC = OFF // free up secondary osc pins
#pragma config FPBDIV = DIV_1 // divide CPU freq by 1 for peripheral bus clock
#pragma config FCKSM = CSDCMD // do not enable clock switch
#pragma config WDTPS = PS1048576 // slowest wdt
#pragma config WINDIS = OFF // no wdt window
#pragma config FWDTEN = OFF // wdt off by default
#pragma config FWDTWINSZ = WINSZ_25 // wdt window at 25%

// DEVCFG2 - get the CPU clock to 40MHz
#pragma config FPLLIDIV = DIV_2 // divide input clock (8 MHz) by 2 to be in range 4-5MHz
#pragma config FPLLMUL = MUL_20 // multiply clock after FPLLIDIV
#pragma config UPLLIDIV = DIV_2 // divide clock after FPLLMUL
#pragma config UPLLEN = ON // USB clock on
#pragma config FPLLODIV = DIV_2 // divide clock by 1 to output on pin

// DEVCFG3
#pragma config USERID = 0 // some 16bit userid
#pragma config PMDL1WAY = ON // not multiple reconfiguration, check this
#pragma config IOL1WAY = ON // not multimple reconfiguration, check this
#pragma config FUSBIDIO = ON // USB pins controlled by USB module
#pragma config FVBUSONIO = ON // controlled by USB module

int start_position[2], center_position[2];
int main ()
{
    
    __builtin_disable_interrupts();

// set the CP0 CONFIG register to indicate that
// kseg0 is cacheable (0x3) or uncacheable (0x2)
// see Chapter 2 "CPU for Devices with M4K Core"
// of the PIC32 reference manual
__builtin_mtc0(_CP0_CONFIG, _CP0_CONFIG_SELECT, 0xa4210583);

// no cache on this chip!

// 0 data RAM access wait states
BMXCONbits.BMXWSDRM = 0x0;

// enable multi vector interrupts
INTCONbits.MVEC = 0x1;

// disable JTAG to be able to use TDI, TDO, TCK, TMS as digital
DDPCONbits.JTAGEN = 0;

__builtin_enable_interrupts();


TRISBbits.TRISB7 = 0;       // set up LED1 pin as a digital output
//    int i,a;
//    display_init();
//    for (i = 0; i<10; ++i)
//    {
//        a = 30+i;
//        display_pixel_set(15,a,1);
//        display_draw();
//    }
    char input[100]; 
    int i=0;
    short accels[3]; // accelerations for the 3 axes
    short mags[3]; // magnetometer readings for the 3 axes
    short temp; 
    
    acc_setup();    //initialize accelerometer
	display_init(); //initialize LED screen
    

    
    float xg, yg, zg;

    
    while(1)
    {
    start_position[0] = 0;
    start_position[1] = 0;
    center_position[0] = 32;
    center_position[1] = 64;
    acc_read_register(OUT_X_L_A, (unsigned char *) accels, 6);
    acc_read_register(OUT_X_L_M, (unsigned char *) mags, 6);
    acc_read_register(TEMP_OUT_L, (unsigned char *) &temp, 2);
    
    xg = (float) accels[0]/16000;
    yg = (float) accels[1]/16000;
    zg = (float) accels[2]/16000;
    
    sprintf(input,"x: %.2f y: %.2f z: %.2f ", xg,yg,zg);
    display_reset();
    display_draw();
    display_ggraph(xg,yg);
    while(input[i])
    {

        display_message(input[i]);
        i++;
        start_position[1] = start_position[1]+5;
//        if(start_position[1]+5>128)
//        {start_position[0]+1;}
//        else
//        {start_position[1] = start_position[1]+5;}
    }
    i = 0;
    display_draw();
    _CP0_SET_COUNT(0);
    while(_CP0_GET_COUNT()<5000000)
    {;}
    
    }
	return (0);
}
void display_message(char input)
{
    char value;
    int i, m;
    for (m = 0; m< 5; ++m)
    {  
        int temp;
        char binary[8];     //initialize binary to be 0b00000000
        value = read_ascii(input,m); //retrieve the hex number from the ascii table
        //hex[m]= value; //assign the read value to a temp hex number holder 
        
        for (i=0;i<8;++i)
        {
            temp = value;
            temp = temp >> 7-i;
            temp = hextobinary(temp);
            binary[7-i]=temp;
        }
        
        for (i = 0; i < 8; ++i)
        {       
            display_pixel_set(start_position[0]+i,start_position[1]+m,binary[i]);
        }
        
    }

}
void display_reset (void)
{
    int m,n;
    for (m=0;m<64;++m)
    {
        for (n=0;n<128;++n)
        {
            display_pixel_set(m,n,0);
        }
    }
}
void  display_ggraph (float x, float y)
{
    int lengthx, lengthy,i,m;
    lengthx = x*20;
    lengthy = y*20;
    
    for (i = -2; i<3; ++i)  //set center position always be lit up
    {
        for (m = -2; m<3; ++m)
        {display_pixel_set(center_position[0]+i,center_position[1]+m,1);}
    }
   
    
    for(i = -2; i<3; ++i)
    {
    if (lengthx > 0)
    {        
        for(m = 0; m<=lengthx; ++m)
        {
            display_pixel_set(center_position[0]+i,center_position[1]+1+m,1);
        }
    }
    if(lengthx <= 0)
    {
         for(m = 0; m>=lengthx; --m)
        {
            display_pixel_set(center_position[0]+i,center_position[1]-1+m,1);
        }
    }
    
    if (lengthy > 0)
    {        
        for(m = 0; m<=lengthy; ++m)
        {
            display_pixel_set(center_position[0]+1+m,center_position[1]+i,1);
        }
    }
    if (lengthy <= 0)
    {
         for(m = 0; m>=lengthy; --m)
        {
            display_pixel_set(center_position[0]-1+m,center_position[1]+i,1);
        }
    }
    }
}
int hextobinary (int input)
{   
    int result;
    if (input & 1)
    {result = 1;}
    else
    {result = 0;}
    return result;
}