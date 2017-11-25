// This software may be used in your CECS326 programs

// simple implementation of SEMAPHORE class with some error 
// and signal handling

#include "semaphore.h"

SEMAPHORE::SEMAPHORE(int size) {

	this->size = size;
	semid = semget(IPC_PRIVATE, size, PERMS); // Perms = 0600
	init();
	}

int SEMAPHORE::remove() {
	semun dummy;
	return semctl(semid, 0 /*not used*/, IPC_RMID, dummy);
	}

int SEMAPHORE::P(int id){
	int retval;
	struct sembuf *p = &((pset+id)->sb); // Arrow is same as dot function (*user).name = user -> name
	while(((retval=semop(semid,p,1))==-1)&&(errno==EINTR)); // the one after the p means applies to one object
    // Idea: process is blocked or it's sleeping - forcing it - will be events in system that resumes process
    // Something that happens: kill signal - process knows to go and finish up
    // To safe guard against that random awake, then we put it in a loop so if this wakes up and the error number is the interrupt then we know it woke up at the wrong time, then force back to go to sleep with loop
	return retval;
}

int SEMAPHORE::V(int id){
	int retval;
	struct sembuf *v = &((vset+id)->sb);
	while(((retval=semop(semid,v,1))==-1)&&(errno==EINTR));
	return retval;
}	

void SEMAPHORE::set_sembuf_p(int k, int op, int flg){
	(pset+k)->sb.sem_num = (short) k;
	(pset+k)->sb.sem_op = op;
	(pset+k)->sb.sem_flg = flg;
}

void SEMAPHORE::set_sembuf_v(int k, int op, int flg){
	(vset+k)->sb.sem_num = (short) k;
	(vset+k)->sb.sem_op = op;
	(vset+k)->sb.sem_flg = flg;
}

int SEMAPHORE::init() {
	pset = new mysembuf[size];
	vset = new mysembuf[size];
	for(int k=0; k<size; k++) {
		set_sembuf_p(k, -1, 0 /*suspend*/);
		set_sembuf_v(k, 1, 0 /*suspend*/);
	}
	
	// initialize all to zero
	semun arg;
	ushort initv[size];
	for(int k=0; k<size; k++) {
		initv[k]=0;
	}
	arg.array = initv; // array of all values, initialized to zero
	return semctl(semid, size, SETALL, arg);
}

SEMAPHORE::~SEMAPHORE() {
	delete pset;
	delete vset;
}

