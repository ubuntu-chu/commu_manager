#ifndef    _SHMEM_H_
#define    _SHMEM_H_

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <errno.h>
#include <muduo/base/Logging.h>

//进程间共享内存定义
#define 	def_FTOK_PATH 			("/etc/profile")
#define 	def_FTOK_PROJ_ID 		(0x20)
#define 	def_SHMEM_SIZE			(20*1024)

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
        LOG_INFO << "shmget succ, key = " << muduo::Fmt("0x%x", key) 
                << " shmid = " << id_ << " size = " << def_SHMEM_SIZE;
	}
	~shmem()
	{
        LOG_INFO << "rm shm id = " <<id_;
		if (id_ != (int)(-1)){
			shmctl(id_, IPC_RMID, NULL);
		}
	}

	void *attach(void)
	{
		if ((mem_addr_ = (void *)shmat(id_, NULL, 0) ) == (void *)(-1)){
			LOG_SYSFATAL << "Failed to attach shared memory ID";
		}
		return mem_addr_;
	}

	void detach(void)
	{
		shmdt(mem_addr_);
	}

private:
	int 						id_;
    void                        *mem_addr_;
};



















#endif


