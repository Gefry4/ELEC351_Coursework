#ifndef FIFO_HEADER_H
#define FIFO_HEADER_H
#include <cstdint>
#include <cstdio>

/******************************************************************************************************************

This Class handles everything to do with the FIFO.
It creates public functions that:

* Check if the FIFO is full
* Check if the FIFO is empty
* Add data to the FIFO
* Remove data from the FIFO
* Get the current size of the FIFO

It creates private variables:

* Front
* Rear
* Items
* Size
* MaxSize
* ReadLock
* WriteLock

*******************************************************************************************************************
*/


//Enviromental sensors
struct environment {
    float temperature;
    float pressure;
    float light;
} ;



class FIFO {
    
   
    public:

        FIFO(uint32_t maxItems);                        //Constructor method creates the FIFO object of size maxItems
 
        void addToFIFO(environment environment_data);   //float lightLvl, float pressure, float temperature);

        environment remFromFIFO();                      //Removes a struct of type environment from the queue

        int getSize();                                  //Gets the current size of the FIFO

        bool isFull();                                  //Public method that turns true if queue is full

        bool isEmpty();                                 //Public method that turns true if queue is empty

    private:

          int front;                //Front of the Queue
          int rear;                 //Rear of the Queue
         environment* items;        //Creates the Queue as an array of environment measurements osize maxItems
         int size;                  //sets intial amount of data inside the array to 0
         int maxSize;               //Sets the the max amount of data that can be in the array to the maximum size of the array
         bool readLock;
         bool writeLock;
};
#endif