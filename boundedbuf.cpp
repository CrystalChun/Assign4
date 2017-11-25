#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include "semaphore.h"
using namespace std;
// Shared memory is some sort of array of some sorts - array of shared elements
const int MAXCHAR = 10;
const int BUFFSIZE = 3;
enum {PUT_ITEM, TAKE_ITEM}; // set up names of my 2 semaphores - first will be put item and second is take item, this is an array, makes put item a value of zero and the take item a value of one

void producer_proc(SEMAPHORE &, char *);
void parent_cleanup(SEMAPHORE &, int);
void consumer_proc(SEMAPHORE &, char *);

int main(){
	int shmid;
	char *shmBUF;

	SEMAPHORE sem(2);
	sem.V(PUT_ITEM); // V(ID = 0) --> 1
	sem.V(PUT_ITEM); // V(ID = 0) --> 2
	sem.V(PUT_ITEM); // V(ID = 0) --> 3, we start off with three parent process allowances?

    // IPC_PRIVATE is replacement for ftok
	shmid = shmget(IPC_PRIVATE, BUFFSIZE*sizeof(char), PERMS); // allocated shared memory (not attached yet)
	shmBUF = (char *)shmat(shmid, 0, SHM_RND); // attaching to that shared memory - pointer that points to shared memory

	if(fork()){ /* parent process */

		producer_proc(sem, shmBUF);
		parent_cleanup(sem, shmid);

	} else { // child process consumer, shared not copied
		consumer_proc(sem, shmBUF);
	}

	exit(0);
} // main

void consumer_proc(SEMAPHORE &sem, char *shmBUF) {
	char tmp;

	for(int k=0; k<MAXCHAR; k++){
		sem.P(TAKE_ITEM); // P(ID = 1) --> decrement id = 1
		tmp = *(shmBUF+k%BUFFSIZE);
		sem.V(PUT_ITEM); // V(ID = 0) --> increment id = 0
		cout << "(" << getpid() << ")  " 
				<< "buf[" << k%BUFFSIZE << "] "
				<< tmp << endl;
	}
} // child_proc

void producer_proc(SEMAPHORE &sem, char *shmBUF) {

	char data[128];
	cout << "(" << getpid() << ")  Please enter a string --> ";
	cin.getline(data, 127);

	char input;
	for(int k=0; k<MAXCHAR; k++){
		input = data[k];
		sem.P(PUT_ITEM); // P(ID = 0) --> decrements id = 0
		*(shmBUF+(k%BUFFSIZE)) = input;
		sem.V(TAKE_ITEM); // Increments ID = 1 (V(1))
	}
} // parent_proc

void parent_cleanup (SEMAPHORE &sem, int shmid) {

	int status;			/* child status */
	wait(0);	/* wait for child to exit */
	shmctl(shmid, IPC_RMID, NULL);	/* cleaning up */
	sem.remove();
} // parent_cleanup

