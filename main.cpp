#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <iostream>
#include <string>
#include "semaphore.h"
using namespace std;

void childProc(SEMAPHORE &, int);
void quit(SEMAPHORE &, pid_t []);
/*
Problems:
1. Determining whether to work on u or v. Idea: shared memory, put in whichever is available. 
*/
int main(int argc, const char * argv[]) {
    const int BUFFSIZE = 2;
    const int u = 827395609;
    const int v = 962094883;
    int children = 0;
    int execute = 0;
    pid_t pid;
    pid_t childrenPid[4];
    SEMAPHORE sem(1);

    // IPC_PRIVATE is replacement for ftok
	int shmid = shmget(IPC_PRIVATE, BUFFSIZE * sizeof(int), PERMS); // allocated shared memory (not attached yet)
	int * shmBUF = (int *)shmat(shmid, 0, SHM_RND); // attaching to that shared memory - pointer that points to shared memor

    sem.V(execute);
    sem.V(execute);
    
    *(shmBUF + 0) = u;
    *(shmBUF + 1) = v;
    cout << "u: " << *(shmBUF + 0) << endl << "v: " << *(shmBuf + 0) << endl;
    // Spawn 4 children, only two at a time and cannot work on same int
    
    while(children < 4) { // Spawns only 4 children... hopefully
        if((pid = fork()) == 0) {
            cout << "In child " << getpid() << endl;
            // Child, loop in here and never break out
            childProc(sem, execute);
        } else {
            cout << "Recording child # " << children << " pid: " << pid << endl;
            childrenPid[children] = pid;
            // Parent - listens for !wq, when it gets it, abort all children
            children ++;
        }
    }
    cout << getpid() << " Parent finished making children" << endl;

    // After spawning 4 children, parent comes out here 

    string sig;

    // Listen for !wq
    while(getline(cin, sig)) {
        if(sig == "!wq") break;
    }

    // Got !wq and aborts children
    quit(sem, childrenPid);
}

void childProc(SEMAPHORE & sem, int execute) {
    int x = 827395609;
    bool resume = true;
    while(true) {
        sem.P(execute);
        // Read queue for number and assign here
        if(resume) {
            cout << getpid() << " running" << endl;
            resume = false;
        }
        while(true) {
            int randNum = rand(); // Generate random numbers
            // Test if number less than 100 or divisible by X
            if(randNum < 100 || randNum % x == 0) {
                // Break out and queue itself
                cout << getpid() << " Leaving, " << randNum << endl;
                sem.V(execute);
                if(!resume) {
                    resume = true;
                }
                // Put number in queue
                break;
            }
        }
    }
}
/**

*/
void quit(SEMAPHORE & sem, pid_t children[]) {
    // Goes through all children and kills them.
    for(int i = 0; i < sizeof(*children); i++) {
        cout << "Killing: " << children[i] << endl;
        kill(children[i], SIGTERM);
    }

    cout << "Killed all children, parent leaving" << endl;

    // Removes semaphore
    sem.remove();
}
