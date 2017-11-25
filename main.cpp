#include <iostream>
#include "semaphore.h"
using namespace std;
/*
Problems:
1. How do we determine which integer the child will work on?
2. How do we spawn more than one child at a time?
3. Queueing the children and activating the next one?
4. Listen for !wq input and terminating everything?
*/
int main(int argc, const char * argv[]) {
    const int u = 827395609;
    const int v = 962094883;
    int children = 0;
    // Spawn 4 children, only two at a time and cannot work on same int
    while(children < 4) { // Spawns only 4 children... hopefully
        if(!fork()) {
            // Child, loop in here and never break out
            int x = u;
            int randNum = rand(); // Generate random numbers
             // Test if number less than 100 or divisible by X
            if(randNum < 100 || randNum % x == 0) {
                // Break out and queue itself
            }
        } else {
            // Parent - listens for !wq

        }
    }
    
}