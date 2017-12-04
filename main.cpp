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
enum {EXE, BUF}; // Semaphore ids, EXE is for executing a child process, BUF is for entering the critical area (aka shared memory)
void childProc(SEMAPHORE &, char*);
void quit(SEMAPHORE &, pid_t [], int);

int main(int argc, const char * argv[]) {
    const int BUFFSIZE = 2; // Shared memory will only have two characters in it
    int children = 0;

    // Keeps track of the process IDs of all the children
    pid_t pid;
    pid_t childrenPid[4];

    // Initialize semaphores
    SEMAPHORE sem(2);
    
    sem.V(EXE); 
    sem.V(EXE); // EXE = 2
    sem.V(BUF); // BUF = 1

    // Initialize shared memory
	int shmid = shmget(IPC_PRIVATE, BUFFSIZE * sizeof(char), PERMS); // allocated shared memory
	char * shmBUF = (char *)shmat(shmid, 0, SHM_RND); // attaching shared memory

    // Setup flags in shared memory, 1 = available, 0 = not available
    *(shmBUF + 0) = '1'; // Flag that u is available
    *(shmBUF + 1) = '1'; // Flag that v is available

    // Spawning 4 children
    while(children < 4) { 
        if((pid = fork()) == 0) { // In child
            cout << "In child " << getpid() << endl;
            childProc(sem, shmBUF);
        } else { // In parent
            cout << "Recording child # " << children << " pid: " << pid << endl;
            // Adding child's process ID into list
            childrenPid[children] = pid;
            children ++;
        }
    }
    cout << getpid() << " Parent finished making children" << endl;

    string endSignal;

    // Listens for "!wq"
    while(getline(cin, endSignal)) {
        if(endSignal == "!wq") break;
    }

    // Got !wq and aborts children
    quit(sem, childrenPid, shmid);
}

int getModNum(char * shmbuf) {
    // Choosing the mod number
    if(*(shmbuf + 0) == '1') { // Using u
        // Flag to prevent use of integer u
        *(shmbuf + 0) = '0'; 
        return 0;
    } else if (*(shmbuf + 1) == '1') { // Using v
        // Flag to prevent use of v integer
        *(shmbuf + 1) = '0';
        return 1;
    } 
}
void childProc(SEMAPHORE & sem, char * shmbuf) {
// Get rid when turning in
    bool resume = true; 
//
    int index = 0; // Index of shared memory this process is using, 0 = u, 1 = v

    while(true) {
        int modNum = 1;
        sem.P(EXE);

        // Accessing critical area (shared memory)
        sem.P(BUF); 
        index = getModNum(shmbuf);
        sem.V(BUF);
// Get rid
        if(resume) {
            cout << "NEW CHILD: " << getpid() << " running, using: " << modNum << endl;
            resume = false;
        }
//
        modNum = index == 0 ? 827395609 : 962094883;
        // Generates random numbers until number is less than 100 or divisible by its modNum
        while(true) {
            int randNum = rand(); // Generate random numbers

            // Test if number less than 100 or divisible by X
            if(randNum < 100 || randNum % modNum == 0) {
                // Breaks out of loop (no longer running)
                cout << getpid() << " Leaving: " << randNum << " index: " << index<< endl;
                
                // Update flag in critical area to allow next process to use this modNum
                sem.P(BUF);
                *(shmbuf + index) = '1';
                sem.V(BUF); 

                sem.V(EXE); // Resumes next process in queue
// Get rid
                if(!resume) {
                    resume = true;
                }
//
                break;
            }
        }
    }
}
/**
* Cleans up the program by killing all 4 children process 
* and removing both the shared memory and the semaphores.
* 
*/
void quit(SEMAPHORE & sem, pid_t children[], int shmid) {
    // Goes through all children process IDs and kills them.
    for(int i = 0; i < sizeof(* children); i++) {
        cout << "Killing: " << children[i] << endl;
        kill(children[i], SIGTERM);
    }
    // Cleans up shared memory
    shmctl(shmid, IPC_RMID, NULL);
    cout << "Killed all children, parent leaving" << endl;

    // Removes semaphore
    sem.remove();
}
