#include "FIFO.h"
#include "mbed.h"
#include <cstdio>


/******************************************************************************************************************

This Class handles everything to do with the FIFO.
It creates the functions to:

* Check if the FIFO is full
* Check if the FIFO is empty
* Add data to the FIFO
* Remove data from the FIFO
* Get the current size of the FIFO

*******************************************************************************************************************
*/

DigitalOut errorled(LED3);

FIFO::FIFO(uint32_t maxItems){                      //Constructor method creates the FIFO object of size maxItems

    front = 0;                          //Front of the Queue
    rear = -1;                          //Rear of the Queue
    items = new environment[maxItems];  //Creates the Queue as an array of environment measurements of size maxItems
    size = 0;                           //sets intial amount of data inside the array to 0
    maxSize = maxItems;                 //Sets the the max amount of data that can be in the array to the maximum size of the array
    readLock=false;                     //Sets the readlock, that stops race conditions, to false
    writeLock=false;                    //Sets the writeLock, that stops race conditions, to false

};

bool FIFO::isFull(){                                //Public method that turns true if queue is full 

    if (size == maxSize){                   //If it's full...
        return true;                        //Return true
    } else{                                 //else...
        return false;                       //Return false
    }


};

bool FIFO::isEmpty() {                              //Public method that turns true if queue is empty

     if (size == 0){                        //If it's empty...
        return true;                        //Return true
    } else {                                //else...
        return false;                       //return false
    }
};


void FIFO::addToFIFO(environment environment_data){ //Add a struct of type environment to the queue
   
   if (isFull()){                           //handle error codition 
        printf("The FIFO is full!\n");
        errorled = 1;                       //turns on red LED
   } else {
       errorled = 0;
       //Here I make it thread save by elimiating race conditions
        if (!writeLock){                    //If the writeLock is fals, which it is intially
            writeLock = true;               //Set it to true, this means that if another thread tries to 
                                            //use this function it will be stopped by the if statement
            rear = (rear+1) % maxSize;      //Update the rear position to be the next rear position
            size++;                         //increment the size of the fifo
            items[rear] = environment_data; //add the environment data to the end of the fifo 
            writeLock = false;              //Set the writeLock back to false, this means that other threads can
                                            //now access this function
            printf("add to queue size is: %d\n", size);
        }

   }
};

environment FIFO::remFromFIFO(){                    //Removes a struct of type environment from the queue

    environment blank;                      //Instantiates envrionment as blank
    blank.light = NULL;                     //Sets light to null using constructor
    blank.pressure = NULL;                  //Sets pressure to null using constructor
    blank.temperature = NULL;               //Sets temperature to null using constructor

    if(isEmpty()){                          //If the Queue is empty

        printf("Queue is empty\n");         
       
        return blank;                       //return the blank constructor

    } else if (!readLock){
        
        readLock = true;                    //Set it to true, this means that if another thread tries to 
                                            //use this function it will be stopped by the if statement
        environment first = items[front];   //remove first value from array items
        size--;                             //Decrement the size of the fifo
        front = (front+1)%(maxSize);        //Update the front position to the next front position
        readLock = false;                   //Set the readLock back to false, this means that other threads can
                                            //now access this function  
        printf("remove from queue size is: %d\n", size);
        return first;                       //return the first value in the FIFO
    }
    else{
        printf("Hit readlock trying again");
         return remFromFIFO();              //Try again
    }

};



int FIFO::getSize() {                               //Gets the current size of the FIFO
    printf("GetSize: %d\n",size);
    return size;
};
