/**
    Crystal Chun 012680952
    File names associated with this assignment: 
        main.cpp, semaphore.cpp, semaphore.h
    Description:
        A parent process generates four child processes.
        Only two child processes can run at a given time.
        Each child works on a specified integer
        (u = 827395609 or v = 962094883), but they cannot
        work on the same integer. A child will continuously 
        generate random numbers until it hits the stopping 
        condition, which is when the number generated is less than
        100 or a factor of the integer the child is work on 
        (either u or v). When the child hits the stopping condition,
        it will surrender its turn at the cpu, add itself to the queue, and allow
        the next child to run.
*/

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

// Semaphore IDs: 
// EXE is for executing a child process
// BUF is for entering critical area (shared memory)
enum {EXE, BUF};

// Function headers
int getModNum(char *);
void childProc(SEMAPHORE &, char *);
void quit(SEMAPHORE &, pid_t [], int);

int main(int argc, const char * argv[]) {
    // Maximum size of shared memory = 2 characters
    const int BUFFSIZE = 2; 

    // Keeps track of the number of children created
    int children = 0;

    // Keeps track of the process IDs of all the children
    pid_t pid;
    pid_t childrenPid[4];

    // Initialize semaphores
    SEMAPHORE sem(2);
    
    // Initialize EXE semaphore to 2
    sem.V(EXE); 
    sem.V(EXE); 

    // Initialize BUF semaphore to 1
    sem.V(BUF); 

    // Initialize shared memory
	int shmid = shmget(IPC_PRIVATE, BUFFSIZE * sizeof(char), PERMS); // allocated shared memory (2 characters)
	char * shmBUF = (char *) shmat(shmid, 0, SHM_RND); // attaching shared memory

    // Setup flags in shared memory, '1' = available, '0' = not available
    *(shmBUF + 0) = '1'; // Flag that u is available
    *(shmBUF + 1) = '1'; // Flag that v is available

    // Spawning 4 children
    while(children < 4) { 
        if((pid = fork()) == 0) { // In child
            childProc(sem, shmBUF);
        } else { // In parent
            // Adding child's process ID into list
            childrenPid[children] = pid;
            children ++;
        }
    }

    // Initialize string to listen for end signal
    string endSignal;

    // Listens for "!wq"
    while(getline(cin, endSignal)) {
        if(endSignal == "!wq") break; // When we get "!wq" we break out of loop
    }

    // Got !wq so aborts children and exits
    quit(sem, childrenPid, shmid);
}

/**
Gets the mod number (u or v) for the child process by checking the shared memory. 
The mod number must not be used by another child process.
    @param shmbuf - The shared memory used to determine which integer to use.
    @return An index (0 for u, 1 for v) that determines which integer is free
        to be used by the child.
*/
int getModNum(char * shmbuf) {
    // Checks shared memory to see which integer is not being in use
    if(*(shmbuf + 0) == '1') { // Integer u is not in use, use u.
        // Flag to prevent use of integer u
        *(shmbuf + 0) = '0'; 
        return 0;
    } else if (*(shmbuf + 1) == '1') { // Integer v is not in use, use v.
        // Flag to prevent use of v integer
        *(shmbuf + 1) = '0';
        return 1;
    } 
}

/**
Where a child process lives. The child process will
continually generate random numbers until the number is either
less than 100 or the number is a factor of the number the 
child process is assigned to work on. 
    @param sem - The semaphore object.
    @param shmbuf - The shared memory object.
*/
void childProc(SEMAPHORE & sem, char * shmbuf) {
    // Initialize index of shared memory this process will use
    int index = 0; // 0 = u, 1 = v

    while(true) {
        // Initialize the mod number
        int modNum = 1;

        // Queue itself if at least two child processes already running
        sem.P(EXE);

        // Accessing critical area (shared memory)
        sem.P(BUF); 
        index = getModNum(shmbuf);
        sem.V(BUF);

        srand((unsigned)time(0));

        // Determines the mod number. Index = 0, use u (827395609), else use v (962094883).
        modNum = index == 0 ? 827395609 : 962094883;
        
        // Generates random numbers until number is less than 100 or divisible by its modNum
        while(true) {
            int randNum = rand(); // Generate random numbers

            // Test if number less than 100 or divisible by modNum
            if(randNum < 100 || randNum % modNum == 0) {
                // If it is, queue itself and allow next child to execute
    
                // Update flag in critical area to allow next process to use this modNum
                sem.P(BUF);
                *(shmbuf + index) = '1';
                sem.V(BUF); 

                // Resumes next process in queue
                sem.V(EXE); 
                break;
            }
        }
    }
}

/**
Cleans up the program by killing all 4 children process 
and removing both the shared memory and the semaphore.
    @param sem - The semaphore object.
    @param children - The process IDs of all four children.
    @param shmid - The ID of the shared memory. Used for deallocating shared memory. 
*/
void quit(SEMAPHORE & sem, pid_t children[], int shmid) {
    // Goes through each child's process ID and kills it.
    for(int i = 0; i < sizeof(* children); i++) {
        kill(children[i], SIGTERM);
    }
    // Cleans up shared memory
    shmctl(shmid, IPC_RMID, NULL);

    // Removes semaphore
    sem.remove();
}
