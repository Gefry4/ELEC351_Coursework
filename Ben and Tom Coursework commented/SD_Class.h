#ifndef SD_CLASS_HEADER_H
#define SD_CLASS_HEADER_H

#include <cstdint>
#include <cstdio>
#include "FIFO.h"
#include "Thread.h"

/******************************************************************************************************************

This Class handles everything to do with the SD card.
It creates public functions that:

* Write data to the sd card
* Clear the data on the sd card
* Read the data from the sd card
* Checks if it's mounted or not
* Mounts the sd card
* Unmounts the sd card

It creates private variables:

* WriteLock - set to false
* ReadLock - set to false
* Mounted - set to false

*******************************************************************************************************************
*/

class SD_CLASS {

    public:

    SD_CLASS();

    int write_sdcard(FIFO &fifo);   //Writes to the sd card the content of the queue FIFO

    int clear_sdcard();             //Clears the test.txt of all data

    int read_sdcard();              //Read the contents of the sd card

    bool is_mounted();              //Checks if the sd card is mounted

    bool mount(FIFO &fifo);         // mount and write queue

    bool unmount(FIFO &fifo);       // write queue and unmount

    private:

    bool writeLock = false;     //Initilise writeLock to off
    bool readLock = false;      //Initilise readLock to off
    bool mounted = false;       //Initilise mounted to unmounted    

};
#endif