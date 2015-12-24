#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/msg.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <setjmp.h>

#ifndef	TRUE
#define	TRUE	1
#endif

#ifndef	FLASE
#define	FALSE	0
#endif

/*	PUBLIC PARAMETERs	*/
#ifndef IPC_PERM
#define IPC_PERM	S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP
#endif
#define DEF_TIMEOUT	30

/*	SEMAPHORE	*/
#define SEM_RETRY   1           /* Retry times when semop() return EAGAIN */
#define SEM_RETRY_INTERVAL 200  /* microseconds */

/*******************************************
struct semun
{
	int val;
	struct semid_ds *buf;
	unsigned short *array;
};
******************************************/

/* MESSAGE QUEUES */
#define MSGSIZE_MAX	4096
struct msgpack
{
	long msgtype;
	char msgbody[MSGSIZE_MAX];
};

/* process */
#ifndef CMDMAX
#define CMDMAX	128
#endif

struct Process{
	char uname[9];
	long pid;
	long ppid;
	int sc;
	char stime[9];
	char tty[9];
	char time[9];
	char cmd[CMDMAX];
};

void Sem_TimeOut();

int SDKSemInit( key_t semkey, int nsem, int semflag );

int SDKSemDestroy( key_t semkey );

#define	SDKSemGet( semkey )	SDKSemInit( semkey, 0, 0 )

int SDKSemGetVal( int semid, int semitem );

int SDKSemSetVal( int semid, int semitem, int val );

int SDKSemGetHandle( int semid, int semitem, int timeout, int autorele );

int SDKSemReleHandle( int semid, int semitem, int autorele );

/*	SHARED MEMORY	*/
int SDKShmInit( key_t shmkey, size_t nsize, int shmflag );

int SDKShmDestroy( key_t shmkey );

#define	SDKShmGet( shmkey )	SDKShmInit( shmkey, 0, 0 )

void *SDKShmAt( int shmid, int offset, int shmflag );

int SDKShmDt( const void *shmaddr, int offset );

/*	PROCESS	*/
struct Process *GetProcess( char *Uname, char *ProcName, int *n );


