#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>
#include <iostream>
#include <string>
#include "semaphore.h"
using namespace std;
enum{PROC1, PROC2, PROC3, PROC4};
/*
Problems:
1. How do we determine which integer the child will work on?
2. How do we spawn more than one child at a time?
3. Queueing the children and activating the next one?
*/
int main(int argc, const char * argv[]) {
    const int u = 827395609;
    const int v = 962094883;
    int children = 0;
    int max = 0;
    SEMAPHORE sem(1);
    sem.V(max);
    pid_t childrenPid[4];
    pid_t pid;
    // Spawn 4 children, only two at a time and cannot work on same int
    
    while(children < 4) { // Spawns only 4 children... hopefully
        if((pid = fork()) == 0) {
            cout << "In child " << getpid() << endl;
            // Child, loop in here and never break out
            int x = u;
            bool resume = true;
            cout << max << endl;
            while(true) {
                sem.P(max);
                if(resume) {
                    cout << getpid() << " running" << endl;
                    resume = false;
                }
                // 
                int randNum = rand(); // Generate random numbers
                // Test if number less than 100 or divisible by X
                if(randNum < 100 || randNum % x == 0) {
                    // Break out and queue itself
                    cout << getpid() << " Leaving, " << randNum << endl;
                    sem.V(max);
                    if(!resume) {
                        resume = true;
                    }
                }
            }
        } else {
            cout << "Recording child # " << children << " pid: " << pid << endl;
            childrenPid[children] = pid;
            // Parent - listens for !wq, when it gets it, abort all children
            children ++;
        }
    }
    cout << getpid() << " Parent finished making children" << endl;
    // After spawning 4 children, parent comes out here 
    while(true) {
        string sig;
        // Listen for !wq
        while(getline(cin, sig)) {
            if(sig == "!wq") break;
        }
        // Got !wq and aborts children
        for(pid_t pid : childrenPid) {
            cout << "Killing: " << pid << endl;
            kill(pid, SIGTERM);
        }
        break;
    }
    cout << "Killed all children, parent leaving" << endl;
    sem.remove();
}