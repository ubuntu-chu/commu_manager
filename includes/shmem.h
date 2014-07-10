#ifndef    _SHMEM_H_
#define    _SHMEM_H_

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <errno.h>
#include <muduo/base/Logging.h>
#include "config.h"

class shmem{
public:
	shmem()
	{
		key_t 			key;

		if ((key = ftok(def_FTOK_PATH, def_FTOK_PROJ_ID)) == (key_t)(-1)){
			LOG_SYSFATAL << "shmem create failed!" << getpid();
		}
		if ((id_ = shmget(key, def_SHMEM_SIZE, 0666|IPC_CREAT)) == (int)(-1)){
			LOG_SYSFATAL << "Failed to obtain shared memory ID";
		}
	}
	~shmem()
	{
		if (id_ != (int)(-1)){
			shmctl(id_, IPC_RMID, NULL);
		}
	}

	void *attach(void)
	{
		void *mem_addr;

		if ((mem_addr = (void *)shmat(id_, NULL, 0) ) == (void *)(-1)){
			LOG_SYSFATAL << "Failed to attach shared memory ID";
		}
		return mem_addr;
	}

	void detach(void *paddr)
	{
		shmdt(paddr);
	}

private:
	int 						id_;
};



















#endif


