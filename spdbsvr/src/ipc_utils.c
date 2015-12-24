#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <errno.h>
#include "utils.h"

#define IPC_KEY 128

union semun {
    int val;
    struct semid_ds *buf;
    ushort *array;
};

int _nShmId, _nSemId;

//�����ź���
static int sem(key_t key)
{
    union semun sem;
    int semid;

    sem.val = 0;

    semid = semget( key, 1, IPC_CREAT|0666 );
    if (semid == -1)
    {
        err_log("semget( CREAT ) ʧ��.");
        return -1;
    }

    //��ʼ���ź���
    semctl( semid, 0, SETVAL, sem );
    return semid;
}

//ɾ���ź���
void d_sem(int semid)
{
    union semun sem;
    sem.val=0;
    semctl(semid,0,IPC_RMID,0);
}

//��������ڴ�
void d_shm( key_t key )
{
    struct shmid_ds buf;
    int shmid, semid;

    shmid=shmget( key, 0, 0 );
    semid=sem(key);
    shmctl(shmid, IPC_RMID, &buf);
    d_sem(semid);

    return;
}

static int set_p(int semid)
{
    struct sembuf sops={0, +1,IPC_NOWAIT};
    return(semop(semid, &sops,1));
}

static int set_v(int semid)
{
    struct sembuf sops={0, -1,IPC_NOWAIT};
    return(semop(semid, &sops,1));
}

//�ȴ��ź������ 0
static void waitv(int semid)
{
    struct sembuf sops={0,0,0};
    semop(semid, &sops,1);
}

int InitProcManage(const char *pidfile)
{
    char *shm;
    char sharesize = 10;
    key_t key;

    if ((key = ftok(pidfile, IPC_KEY)) == -1)
    {
        err_log( "ftok() ʧ��.");
        return -1;
    }
    if ((_nShmId = shmget(key, sharesize, 0666|IPC_CREAT)) == -1)
    {
        err_log( "%s|%d shmget( CREAT ) ʧ��, errinfo=[%s].", 
                __FILE__, __LINE__, strerror(errno) );
        return -1;
    }
    if ((_nSemId = sem(key)) == -1)
    {
        err_log( "%s|%d sem() ʧ��.", __FILE__, __LINE__ );
        return -1;
    }
    shm = (char *)shmat(_nShmId, 0, 0);
    if((int)shm == -1)
    {
        err_log( "%s|%d shmat() ʧ��.", __FILE__, __LINE__ );
        return -1;
    }

    memset(shm, 0, sharesize);
    shm[0] = '0';
    shmdt(shm);

    return 0;
}

int UpdateProcNum( int step)
{
    char *shm;
    char buf[11];
    int  procnum;

    shm = (char *)shmat(_nShmId, 0, 0);
    if ((int)shm == -1)
    {
        err_log( "����: shmat SHM error, %s.", strerror(errno));
        return -1;
    }

    waitv(_nSemId);

    memset(buf, 0, sizeof(buf));
    memcpy(buf, shm, 10);
    procnum = atoi(buf) + step;

    memset(buf, 0, sizeof(buf));
    sprintf(buf, "%d", procnum);
    set_p(_nSemId);
    memcpy(shm, buf, 10);
    set_v(_nSemId);
    shmdt(shm);

    return procnum;
}
