/* mbed Microcontroller Library
 * Copyright (c) 2019 ARM Limited
 * SPDX-License-Identifier: Apache-2.0
 */
/*
Written by:
Thomas Powell:  10576368
Benedict Fletcher:  10583227
*/

#include "F429_Mega_Shell_Header.h"
#include "TextLCD/TextLCD.h"
#include <cstdio>
#include <cstdio>
#include <string>
#include "FIFO.h"
#include "SD_Class.h"

//prototypes
void Traffic_Lights();
void LCD_BackLight_Effect();
void Bar_Flash();
void clearMatrix();
void matrix_scan();
void plotMatrix();
int seg7num(int);
void seg7clear();
void count_thread();
float potav();
float ldrav();
void environment_data();
void export_data();
void blink_led();
void plotPointOnMatrix(int);
void plotScrollingGraph(int[16]);
float getMin(int[16], int);
float getMax(int[16], int);
float getRange(float,float);
void inputToBar();
float add_to_array(int input);
void reset_led_matrix();

Thread t1;
Thread t2;
Thread t3;
Thread t4;
Thread t5;
Thread t6;
Thread t7;
Thread t8;

Timer stopwatch;
FIFO fifo(25);     //Sampling every 5 seconds for 60 seconds, size must be greater than 12
SD_CLASS sdCard;
DigitalIn userButton(USER_BUTTON);
DigitalOut blueLED(LED2);      //blue LED because green led1 doesn't seem to work
volatile bool running = false;
int myInputVaulesTwo [16] = {0};
int valueSelect = 0;            //light
int valueCount = 0;


int outLEDStripVaule = 0;
int main()
{   
    //Mount and read the sd card
    sdCard.mount(fifo);
    printf("Mounted the SD card\n");
    sdCard.clear_sdcard();
    seg7clear();


    if (sdCard.is_mounted()){                           //Check if the sd card is mounted and if it is light the blue led
        blueLED = 1;
    }

    Traffic_Lights_2.output();
    Traffic_Lights_2.mode(OpenDrainNoPull);
    Traffic_Lights_2=1;

    Pedestrian.output();
    Pedestrian.mode(OpenDrainNoPull);

    Pedestrian=0;
    ThisThread::sleep_for(DELAY);                       //wait_us(1000000);
    Pedestrian=1;

    // 7 segment display setup
    spi.format(8,0);                                    // 8bits, Rising edge, +VE Logic Data
    spi.frequency(1000000);                             //1MHz Data Rate
    oe=0;                                               //Enable Output NB can be more selective than this if you wish
    
    
    printf("Staring Program..\n");
    t1.start(Traffic_Lights);
    t2.start(LCD_BackLight_Effect);
    t3.start(Bar_Flash);
    t4.start(matrix_scan);
    t5.start(count_thread);
    t6.start(environment_data);
    t7.start(export_data);
    t8.start(blink_led);
    

    myLCD.cls();
    myLCD.printf("SECaM LDR\n");
    myLCD.locate(0,1);myLCD.printf("Switch=");
    buzzer.period_us(2273);
    buzzer=0.5f;wait_us(200000);buzzer=0;

    char switchNum=' ';
    while(true){
        if (swA)switchNum='A';
        if (swB)switchNum='B';
        if (swC)switchNum='C';
        if (swD)switchNum='D';

        switch(switchNum){
            case 'A': buzzer.period_us(350);buzzer=0.5f; outLEDStripVaule = outLEDStripVaule + 1; break;//spk.period_us(2273);
            case 'B': buzzer.period_us(3);buzzer=0.5f; valueSelect = (valueSelect + 1) % 3; wait_us(2000); reset_led_matrix(); break;//The wait is ok here because it only happens when the button is pressed so it doesn't block everything often
            case 'C': buzzer.period_us(370);buzzer=0.5f; break;//spk.period_us(1912);
            case 'D': buzzer.period_us(380);buzzer=0.5f; break;//spk.period_us(1704);
            default:switchNum=' ';buzzer=0;break;
        }

        myLCD.locate(7, 1);myLCD.printf("%c",switchNum);
        myLCD.locate(10,1);myLCD.printf("%4.2fV",potav());
        myLCD.locate(10,0);myLCD.printf("%4.2fV",ldrav());
        switchNum=0;

        if(userButton){
            ThisThread::sleep_for(200ms);               //Debounce switch
            printf("User button pressed\n");
            if(sdCard.is_mounted()){                    //Check if the sdcard is mounted
                                                        //IF it is mounted
                running = true;                         //set running to true for the blink function that is running in the background
                sdCard.unmount(fifo);                   //unmount the sd card
                running = false;                        //set running to false to stop the blink function
                blueLED = 0;                            //turn the blue led off
                printf("Unmounted the SD card!\n");
            } else {                                    //if the sd card is not mounted
                running = true;                         //set running true to start the blinky function 
                sdCard.mount(fifo);                     //mount the sd card
                running = false;                        //set running to false to stop the blinky function
                blueLED = 1;                            //turn the blue led on
                printf("Mounted the SD card\n");
            }
        }
        wait_us(200000);
    }
}




//Blink function toggles the led in a long running loop
void blink_led() {
    while (true) {
        if (running){
            blueLED = !blueLED;                         //toggles blue led
            ThisThread::sleep_for(100ms);
        }
    }
}

float potav(){
    unsigned int adc_sample[SAMPLES+1];
    float Voltage;
    adc_sample[0]=0;
    for(int i=1; i<=SAMPLES; i++){
        adc_sample[i] = pot_an_input.read_u16();
        adc_sample[0]+=adc_sample[i];
    }
    Voltage = 3.3f * ((float)adc_sample[0]/(float)SAMPLES)/65535.0f;
    return Voltage;
}

float ldrav(){
    unsigned int adc_sample[SAMPLES+1];
    float Voltage;
    adc_sample[0]=0;
    for(int i=1; i<=SAMPLES; i++){
        adc_sample[i] = ldr.read_u16();
        adc_sample[0]+=adc_sample[i];
    }
    Voltage = 3.3f * ((float)adc_sample[0]/(float)SAMPLES)/65535.0f;
    return Voltage;
}

void LCD_BackLight_Effect(){
    for(float i=0; i<1.0f; i+=0.01){
        myLCD_BL=i;
        ThisThread::sleep_for(20ms);
    }
    myLCD_BL=1.0f;
}

void Traffic_Lights(){
    while (true)
    {

        Pedestrian=0;

        Traffic_Lights_1 = RED;
        Traffic_Lights_2 = 3;//7-GREEN;
        ThisThread::sleep_for(DELAY);

        Pedestrian = 1;
        Traffic_Lights_1 = RED + AMBER;
        Traffic_Lights_2 = 5;//7-AMBER;
        ThisThread::sleep_for(DELAY);
        Traffic_Lights_1 = GREEN;
        Traffic_Lights_2 = 6;
        ThisThread::sleep_for(DELAY);
        Traffic_Lights_1 = AMBER;
        Traffic_Lights_2 = 4;
        ThisThread::sleep_for(DELAY);
    }
}

void Bar_Flash()
{
    //Three groups of 8 LED's
    //When sending data, two of the three sections with either 
    //be 'full' on or 'full' off. Only the section where the vaule
    //is in the range of needs to be changed to fit the output.

    //The one ajustable vaule from the three inputs
    int barAmountFull; //Out of 24

    //With 24 LED's the boundry are set
    int firstBoundry = 8;
    int secondBoundry = 16;
    int thirdBoundry = 24;

    while(true){

        //Using singal input button, cylce through the posible outputs
        if (outLEDStripVaule == 0) 
        {
            //Temprature
            barAmountFull = 5;
        }
        else if (outLEDStripVaule == 1) 
        {
            //Pressure
            barAmountFull = 9;
        }
        else if (outLEDStripVaule == 2)
        {
            //LDR
            barAmountFull = 15;
        }
        else if (outLEDStripVaule == 3)
        {
            //Some other displayed output?
            barAmountFull = 24;
        }
        else 
        {    
            //Reset to start of loop
            outLEDStripVaule = 0;
        }

        

        
        //-----------------------------------------------------------------------
        //Setting which lights should apear on
        if(barAmountFull <= firstBoundry) //If input is below or equal to 8
        {
            int outputOnLED = 1; //Set to one for the mulitlpication
            for(int i = 1; i <= barAmountFull; i++)
            {
                outputOnLED = outputOnLED * 2;  //Vaule needs to be in binnary form to be displayed
            }
            outputOnLED = outputOnLED - 1;  //Fill in EVERY light thats lit up below that point

            RGBoe = 0; //Enable Output

            RGBled = outputOnLED; RGBcol = Rgb; RGBcol = rgb;   //Red
            RGBled = 0; RGBcol = rGb; RGBcol = rgb;     //Green
            RGBled = 0; RGBcol = rgB; RGBcol = rgb;     //Blue
            ThisThread::sleep_for(DELAY/4);

            RGBoe = 1;//Disable Output
        }
        else if(barAmountFull <= secondBoundry) //Repeat for second range - Between 9 and 16
        {
            int outputOnLED = 1;
            for(int i = 1; i <= barAmountFull - firstBoundry; i++) //We still only have a range of 8 bits, so 8 need to be removed
            {
                outputOnLED = outputOnLED * 2;
            }
            outputOnLED = outputOnLED - 1;

            RGBoe = 0; //Enable Output

            RGBled = 255; RGBcol = Rgb; RGBcol = rgb;   //Red
            RGBled = outputOnLED; RGBcol = rGb; RGBcol = rgb;     //Green
            RGBled = 0; RGBcol = rgB; RGBcol = rgb;     //Blue
            ThisThread::sleep_for(DELAY/4);

            RGBoe = 1;//Disable Output
        }
        else //Between 17 and 24
        {
            int outputOnLED = 1;
            for(int i = 1; i <= barAmountFull - secondBoundry; i++) //As the maths needs to between 1 and 8, the other unnessary vaules are reduced
            {
                outputOnLED = outputOnLED * 2;
            }
            outputOnLED = outputOnLED - 1;

            RGBoe = 0; //Enable Output

            RGBled = 255; RGBcol = Rgb; RGBcol = rgb;   //Red
            RGBled = 255; RGBcol = rGb; RGBcol = rgb;     //Green
            RGBled = outputOnLED; RGBcol = rgB; RGBcol = rgb;     //Blue
            ThisThread::sleep_for(DELAY/4);

            RGBoe = 1;//Disable Output
        }
    }

}


void matrix_scan(void)  //Sets the mins and maxes and creates the array to send to the led matrix
{
    while (true) {    
    int numOfElements = 16;
    int yVauleScale = 8;
    float min;
    float max;
                                    //Set the min and maxes for each seperate sensor
    if (valueSelect == 0) {         //temperature
        min = 0;
        max = 50;
    } else if (valueSelect == 1) {  //pressure
        min = 950.0;
        max = 1050.0;
    } else if(valueSelect == 2) {   //light
        min = 0.0;
        max = 2.5;
    };
    
    float range = getRange(min, max);                                   //Get the range of the chosen sensor's min and max
    float rescale = range / yVauleScale;                                //Rescale the range to fit the number of leds, 8
    int myChangedVaules [16];                                           //Initilise the myChangedValues array as 16 bits 

    for(int i = 0; i < numOfElements; i++)                              //Cycle through each element in the array and...
    {
        myChangedVaules[i] = ((myInputVaulesTwo[i] - min) / rescale);   //set it to the scaled version of the input
    }
    
    plotScrollingGraph(myChangedVaules);                                //Plot the array on the led matrix
    }
}

void clearMatrix(void)  //clears the matrix
{
    cs=0;               //CLEAR Matrix
    spi.write(0x00);    //COL RHS
    spi.write(0x00);    //COL LHS
    spi.write(0x00);    //ROX RHS
    cs=1;
}

void count_thread(){
    seg7clear();
    unsigned char counter=0;
    while(true){
        seg7num(counter);
        counter++;
        if (counter>99){counter=0;}
        thread_sleep_for(1000);
    }
}

int seg7num(int num){
    int temp,count=0;

    if (num<0||num>99){return -1;} // Out of Range check

    while(count<2){
        if (count==0){temp=(num/10)%10;} // Tens
        if (count==1){temp=num%10;}      // Units
        switch(temp){
            case 0: seg7=A+B+C+D+E+F;break;
            case 1: seg7=B+C;break;
            case 2: seg7=A+B+D+E+G;break;
            case 3: seg7=A+B+C+D+G;break;
            case 4: seg7=B+C+F+G;break;
            case 5: seg7=A+C+D+F+G;break;
            case 6: seg7=C+D+E+F+G;break;
            case 7: seg7=A+B+C;break;
            case 8: seg7=A+B+C+D+E+F+G;break;
            case 9: seg7=A+B+C+D+F+G;break;
            default: seg7=0;break;
        }
        if (count==0){LatE1=1;LatE1=0;} //Latch Tens Digit
        if (count==1){LatE2=1;LatE2=0;} //Latch Units Digit
        count++;
    }
    return 0; // Return Completed OK
}

void seg7clear(void){
    seg7=0;
    LatE1=1;LatE1=0;
    LatE2=1;LatE2=0;
}
 
void environment_data(void) //Takes the sensor data and assign them to values in the environment struct, also writes data to the array depending on the valueSelect
{
    environment environ;                                        //instantiates the environment structa as envrion
    bmp280.initialize();                                        //Initilises the sensors

    while (true)
    {
        environ.light = ldrav();                                //Takes the data from the LDR
        environ.temperature=bmp280.getTemperature();            //Takes the data from the thermometer
        environ.pressure=bmp280.getPressure();                  //Takes the pressure reading
        printf("Temperature is %.1fC, pressure is %.1fmBar, light level is %.1fV\n",environ.temperature,environ.pressure, environ.light);
        fifo.addToFIFO(environ);	                            //write to the FIFO

        if (valueSelect == 0){                                  
            myInputVaulesTwo[valueCount] = environ.temperature; //Sets the data type in the array as the temperature
            valueCount = (valueCount+1)% 16;                    //Cycles through the positions in the array
            printf("Adding temperature to the array\n");
        }
        else if (valueSelect == 1){
            myInputVaulesTwo[valueCount] = environ.pressure;    //Sets the data type in the array as the pressure
            valueCount = (valueCount+1)% 16;                    //Cycles through the positions in the array
            printf("Adding pressure to the array\n");
        }
        else if (valueSelect == 2) {
            myInputVaulesTwo[valueCount] = environ.light;       //Sets the data type in the array as the light level
            valueCount = (valueCount+1)% 16;                    //Cycles through the positions in the array
            printf("Adding light to the array\n");
        }
        thread_sleep_for(WAIT_TIME_MS);                         //wait 5 seconds
    }
}

void export_data(){     //Exports the data to the sd card
    while (true) {
        if(sdCard.is_mounted()){                //if the sd card is mounted...
            sdCard.write_sdcard(fifo);          //write to the sd card
            thread_sleep_for(ONE_HUNDRED_WAIT); 
        }
    }

}

void plotScrollingGraph(int inputValues [16]) //Plotting graph
{
    //The x axis is written too in two halfs, each half is set up full of zero's
    int rowValuesLeft [8] = { };
    int rowValuesRight [8] = { };
    for(int y = 0; y <=7; y++)  //For all the y axis
    {
        for(int x = 1; x <= 16; x++)    //For all of the x axis
        {
            if(inputValues[ x - 1 ] > y)
            {
                int rightSide = 0;  //If one side is calculated, the other has to be set to zero
                int leftSide = 0;   //So both are set to zero, then one is changed later on.
                if(x < 9) //Leftside
                {
                    leftSide = 1;   //Set to one for maths (as zero wouldn't change)
                    for(int i = 1; i < x; i++)
                    {
                        leftSide = leftSide * 2;
                    }
                    rowValuesLeft[y] = rowValuesLeft[y] + leftSide;

                }
                else //Rightside
                {
                    rightSide = 1;
                    for(int i = 1; i < x - 8; i++)
                    {
                        rightSide = rightSide * 2;
                    }
                    rowValuesRight[y] = rowValuesRight[y] + rightSide;
                }
            }
        }
    }
    //graphPlot(rowValuesLeft, rowValuesRight);


        for(int y = 0; y <=7; y++) //All 8 y axis points
        {
            cs = 0;
            spi.write(rowValuesRight[y]); //Right x axis
            spi.write(rowValuesLeft[y]); //Left x axis
            spi.write(y); //y axis
            cs = 1;
            //ThisThread::sleep_for(DELAY/4);
        }
}

void plotPointOnMatrix(int x, int y)
{
    int rightSide = 0;
    int leftSide = 0;
    if(x < 9) //Leftside
    {
        leftSide = 1;
        for(int i = 1; i < x; i++)
        {
            leftSide = leftSide * 2;
        }

    }
    else //Rightside
    {
        rightSide = 1;
        for(int i = 1; i < x - 8; i++)
        {
            rightSide = rightSide * 2;
        }
    }

    y = 0x03;
    cs = 0;
    spi.write(rightSide); //Right
    spi.write(leftSide); //Left
    spi.write(y); //Up
    cs = 1;
}

float getMin(int inputArray[], int r) //Fines the minimun vaule in an array
{
    float min = 1000; //Defult large number
    for(int i = 0; i < r; i++) //For all the vaules in the array
    {
        if(inputArray[i] < min) //If this vaule is smaller than the current vaule
        {
            min = inputArray[i]; //Set to new min
        }
    }
    return min;
}

float getMax(int inputArray[], int r) //Fines the max vaule in an array
{
    float max = 0; //Defult small number
    for(int i = 0; i < r; i++)  //For all the vaules in the array
    {
        if(inputArray[i] > max) //If that vaule in the array is larger
        {
            max = inputArray[i]; //Set to new max
        }
    }
    return max;
}

float getRange(float min, float max) //Get range between two points and return a vaule
{
    float Range = max - min;
    return Range;
}

void inputToBar() //Changing the output of the line of LED's
{
    printf("You put in an input into the console:\n");
    printf("This input is to change the bar vaule,\n");
    printf("What do you want to input?\n");
    printf("Temprateure -> T \n Pressure -> P \n Lightlevel -> L \n ");

    char inputChar; //= pc.getc();
    if(inputChar == 'T')
    {
        printf("Outputting Temp: \n");

    }
    else if(inputChar == 'P')
    {
        printf("Outputting Pressure: \n");

    }
    else if(inputChar == 'L')
    {
        printf("Outputting Lightlevel: \n");

    }
    else 
    {
        printf("That is not a valid input \n");
    }

}

void reset_led_matrix(){    //This resets the led matrix to 0's and also resets the values in the array to 0
    
    cs = 0;
    spi.write(0x00);                //Right x axis
    spi.write(0x00);                //Left x axis
    spi.write(0x00);                //y axis
    cs = 1;
    
    for (int i = 0; i<16; i++){     //Goes through each element in array and sets it to 0
        myInputVaulesTwo[i] = 0;
    }

}