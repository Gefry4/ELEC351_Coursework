#include "mbed.h"
#include "SD_Class.h"
#include "FIFO.h"
#include <cstdio>
#include "SDBlockDevice.h"
#include "FATFileSystem.h"
#include "Thread.h"

/******************************************************************************************************************

This Class handles everything to do with the SD card.
It creates the function to:

* Write data to the sd card
* Clear the data on the sd card
* Read the data from the sd card
* Checks if it's mounted or not
* Mounts the sd card
* Unmounts the sd card

*******************************************************************************************************************
*/


SDBlockDevice sd(PB_5, PB_4, PB_3, PF_3);


SD_CLASS::SD_CLASS(){
    
    writeLock = false;
    readLock = false;

}


int SD_CLASS::write_sdcard(FIFO &fifo) {    //Writes to the sd card the content of the queue FIFO
    int returnValue= 0;                                             //Initilise the return value
    if (!writeLock) {                                               //This makes it safe from race conditions
        writeLock = true;                                           //Turn on the writeLock
        printf("Initialise and write to a file\n");
        if (!is_mounted()) {                                        //If not mounted
            printf("Card not mounted write failed \n");
            returnValue = -1;                                       //set the return value to -1
        }else {                                                     // If it is mounted
        
            FATFileSystem fs("sd", &sd);                            //Choose the file system of sd
            FILE *fp = fopen("/sd/test.txt", "a");                  //Opens a file called test.txt

            if (fp == NULL) {                                       //If it fails it will output an error message
                error("Could not open file for write\n");
                sd.deinit();                                        //And eject the SD card
                returnValue = -1;                                   //Then set the return value to -1
            } else {
               int queueSize = fifo.getSize();                      //Get the size of the queue in question
                for (int i = 0; i < queueSize; i++) {               //Loop through every value in the queue
                    environment dataToWrite = fifo.remFromFIFO();   //Get the data to write to the queue               
                    fprintf(
                    fp, "%.2f, %.2f, %.2f\n", dataToWrite.temperature,
                    dataToWrite.pressure,
                    dataToWrite.light);                             //Print all the data we got from the queue to the file
                }
                fclose(fp);                                         // Closes the file
                printf("SD Write done...\n");
                returnValue = 0;                                    //Set the return value to 0
            }       
        }
        writeLock = false;
    } else{
        returnValue = -1;                                           //This is to ensire that the writeLock works as there needs to be a return for each controll path
    }
    return returnValue;
}

int SD_CLASS::clear_sdcard() {              //Clears the test.txt of all data
  printf("Clear all existing data from file\n");
  int returnValue = 0;                              //Initilise the return value to 0
  if (!is_mounted()) {                              //If it is not mounted
    printf("SD Card not mounted \n");
    returnValue = -1;                               //Sets the return value to -1
  }

  FATFileSystem fs("sd", &sd);                      //Chooses the file system of sd
  FILE *fp = fopen("/sd/test.txt","w");             //The 'w' means write and it essentially just replaces
                                                    //whatever is in that file with the new data
  if (fp == NULL) {                                 //If it fails it will output an error message
    error("Could not open file for clearing\n");
    //sd.deinit();                                    //Ejects the sd card
    returnValue = -1;                               //Sets the return value to -1
  } else {
    fprintf(fp, "Temperature, Pressure, Light\n");  //Writes the headers to a file
    fclose(fp);                                     //Closes the file
    printf("SD clear done...\n");
    sd.deinit();                                    //Ejects the file
    returnValue = 0;                                //Sets return vlaue to 0
  }
  return returnValue;
}

int SD_CLASS::read_sdcard()                 //Read the contents of the sd card
{
    int returnValue= 0;                                             //Initilise the return value
    if (!readLock){                                                 //This makes it safe from race conditions
        readLock = true;                                            //Turn on the writeLock
        printf("Initialise and read from a file\n");
        if (!is_mounted()) {                                        //If not mounted
            printf("It's not mounted \n");
            returnValue= -1;                                        //set the return value to -1
        } else {                                                    // If it is mounted
    
            FATFileSystem fs("sd", &sd);                            //Choose the file system of sd
            FILE *fp = fopen("/sd/test.txt","r");                   //Opens a file called test.txt

            if(fp == NULL) {                                        //If it fails it will output an error message
                error("Could not open or find file for read\n");
                returnValue= -1;                                    //Then set the return value to -1
            } else {
                char buff[64]; buff[63] = 0;                        //Creates a character buffer of size 64
                while (!feof(fp)) {                                 
                    fgets(buff, 63, fp);
                    printf("%s\n", buff);
                }
                fclose(fp);
                printf("SD Write done...\n");
                sd.deinit();
                
                returnValue= 0;
            }
        } 
        readLock = false;  
    } else {
        returnValue= -1;           //This is to ensire that the writeLock works as there needs to be a return for each controll path
    }
    return returnValue; 

}

bool SD_CLASS::is_mounted(){                //Checks if the sd card is mounted

    return mounted;

}

bool SD_CLASS::mount(FIFO &fifo){           //This just checks whether the sd card is mounted or not
    printf("Mounting the sd card \n");
    if (0 != sd.init()){               //If not mounted
        printf("Failed to mount SD card\n");
        mounted = false;                //Mounted is set to false
        //blueLED = 0;                   //Turn LED off

    } else {                            //if mounted
        //running = true;
        mounted = true;                 //Mounted is set to true
        //printf("mounted ahs been set to ture\n");
        //t8.start(blink);            //Blink LED
        write_sdcard(fifo);             //Write current content to sd
        //running = false;                //set running to false to stop the LED
        //t8.join();                  //Combines two threads back together
        //blueLED = 1;                   //Turn LED on
    }
    return mounted;
}


bool SD_CLASS::unmount(FIFO &fifo){         //This just checks whether the sd card is mounted or not

    printf("unmounting the sd card\n");
    write_sdcard(fifo);                     //Write current content to sd
    mounted = false;                        //Mounted is set to false
    sd.deinit();                            //unmount the sd card
    return mounted;
}