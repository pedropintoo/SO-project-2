/**
 *  \file semSharedReceptionist.c (implementation file)
 *
 *  \brief Problem name: Restaurant
 *
 *  Synchronization based on semaphores and shared memory.
 *  Implementation with SVIPC.
 *
 *  Definition of the operations carried out by the receptionist:
 *     \li waitForGroup                 -> TOARRIVE (0)
 *     \li provideTableOrWaitingRoom    -> WAIT (1)
 *     \li receivePayment               -> ATTABLE (2)
 *                                      -> DONE (3)
 *
 *  \author Nuno Lau - December 2023
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <math.h>
#include <assert.h>

#include "probConst.h"
#include "probDataStruct.h"
#include "logging.h"
#include "sharedDataSync.h"
#include "semaphore.h"
#include "sharedMemory.h"

/** \brief logging file name */
static char nFic[51];

/** \brief shared memory block access identifier */
static int shmid;

/** \brief semaphore set access identifier */
static int semgid;

/** \brief pointer to shared memory region */
static SHARED_DATA *sh;

/* constants for groupRecord */
#define TOARRIVE 0
#define WAIT 1
#define ATTABLE 2
#define DONE 3

/** \brief receptioninst view on each group evolution (useful to decide table binding) */
static int groupRecord[MAXGROUPS];

/** \brief receptionist waits for next request */
static request waitForGroup();

/** \brief receptionist waits for next request */
static void provideTableOrWaitingRoom(int n);

/** \brief receptionist receives payment */
static void receivePayment(int n);

/**
 *  \brief Main program.
 *
 *  Its role is to generate the life cycle of one of intervening entities in the problem: the receptionist.
 */
int main(int argc, char *argv[])
{
    int key;    /*access key to shared memory and semaphore set */
    char *tinp; /* numerical parameters test flag */

    /* validation of command line parameters */
    if (argc != 4)
    {
        freopen("error_RT", "a", stderr);
        fprintf(stderr, "Number of parameters is incorrect!\n");
        return EXIT_FAILURE;
    }
    else
    {
        freopen(argv[3], "w", stderr);
        setbuf(stderr, NULL);
    }

    strcpy(nFic, argv[1]);
    key = (unsigned int)strtol(argv[2], &tinp, 0);
    if (*tinp != '\0')
    {
        fprintf(stderr, "Error on the access key communication!\n");
        return EXIT_FAILURE;
    }

    /* connection to the semaphore set and the shared memory region and mapping the shared region onto the
       process address space */
    if ((semgid = semConnect(key)) == -1)
    {
        perror("error on connecting to the semaphore set");
        return EXIT_FAILURE;
    }
    if ((shmid = shmemConnect(key)) == -1)
    {
        perror("error on connecting to the shared memory region");
        return EXIT_FAILURE;
    }
    if (shmemAttach(shmid, (void **)&sh) == -1)
    {
        perror("error on mapping the shared region on the process address space");
        return EXIT_FAILURE;
    }

    /* initialize random generator */
    srandom((unsigned int)getpid());

    /* initialize internal receptionist memory */
    int g;
    for (g = 0; g < sh->fSt.nGroups; g++)
    {
        groupRecord[g] = TOARRIVE;
    }

    /* simulation of the life cycle of the receptionist */
    int nReq = 0;
    request req;
    while( nReq < sh->fSt.nGroups*2 ) { // each group must speak with receptionist 2 times!
        req = waitForGroup();
        switch (req.reqType)
        {
        case TABLEREQ:
            provideTableOrWaitingRoom(req.reqGroup); // TODO param should be groupid
            break;
        case BILLREQ:
            receivePayment(req.reqGroup);
            break;
        }
        nReq++;
    }

    /* unmapping the shared region off the process address space */
    if (shmemDettach(sh) == -1)
    {
        perror("error on unmapping the shared region off the process address space");
        return EXIT_FAILURE;
        ;
    }

    return EXIT_SUCCESS;
}

/**
 *  \brief decides table to occupy for group n or if it must wait.
 *
 *  Checks current state of tables and groups in order to decide table or wait.
 *
 *  \return table id or -1 (in case of wait decision)
 */
static int decideTableOrWait(int n)
{
    // TODO insert your code here
    int g;
    for (int table = 0; table < NUMTABLES; table++) {
        for (g = 0; g < sh->fSt.nGroups && sh->fSt.assignedTable[g] != table; g++);
        if (g == sh->fSt.nGroups) return table;
    }
    return -1;
}

/**
 *  \brief called when a table gets vacant and there are waiting groups
 *         to decide which group (if any) should occupy it.
 *
 *  Checks current state of tables and groups in order to decide group.
 *
 *  \return group id or -1 (in case of wait decision)
 */
static int decideNextGroup()
{
    // TODO insert your code here
    if(sh->fSt.groupsWaiting == 0) return -1;

    int r;
    for (int n = 0; n < sh->fSt.nGroups; n++) {
        if (groupRecord[n] == WAIT) {
            r = n; // next group = smallest id that is waiting
            break;
        }
    }

    return r;
}

/**
 *  \brief receptionist waits for next request
 *
 *  Receptionist updates state and waits for request from group, then reads request,
 *  and signals availability for new request.
 *  The internal state should be saved.
 *
 *  \return request submitted by group
 */
static request waitForGroup()
{
    request ret;

    if (semDown(semgid, sh->mutex) == -1)
    { /* enter critical region */
        perror("error on the down operation for semaphore access (RT)");
        exit(EXIT_FAILURE);
    }
    
    ////////////////////////////////////////////
    // TODO insert your code here       
    sh->fSt.st.receptionistStat = WAIT_REQUEST;
    saveState(nFic, &sh->fSt);
    ////////////////////////////////////////////

    if (semUp(semgid, sh->mutex) == -1)
    { /* exit critical region */
        perror("error on the up operation for semaphore access (RT)");
        exit(EXIT_FAILURE);
    }

    ////////////////////////////////////////////
    // TODO insert your code here
    if (semDown(semgid, sh->receptionistReq) == -1)
    {
        perror("error on the down operation for semaphore access (RT)");
        exit(EXIT_FAILURE);
    }
    ////////////////////////////////////////////

    if (semDown(semgid, sh->mutex) == -1)
    { /* enter critical region */
        perror("error on the down operation for semaphore access (RT)");
        exit(EXIT_FAILURE);
    }

    ////////////////////////////////////////////
    // TODO insert your code here
    ret = sh->fSt.receptionistRequest;
    ////////////////////////////////////////////

    if (semUp(semgid, sh->mutex) == -1)
    { /* exit critical region */
        perror("error on the up operation for semaphore access (RT)");
        exit(EXIT_FAILURE);
    }

    ////////////////////////////////////////////
    // TODO insert your code here
    if (semUp(semgid, sh->receptionistRequestPossible) == -1)
    { 
        perror("error on the up operation for semaphore access (RT)");
        exit(EXIT_FAILURE);
    }
    ////////////////////////////////////////////


    return ret;
}

/**
 *  \brief receptionist decides if group should occupy table or wait
 *
 *  Receptionist updates state and then decides if group occupies table
 *  or waits. Shared (and internal) memory may need to be updated.
 *  If group occupies table, it must be informed that it may proceed.
 *  The internal state should be saved.
 *
 */
static void provideTableOrWaitingRoom(int n)
{
    if (semDown(semgid, sh->mutex) == -1)
    { /* enter critical region */
        perror("error on the down operation for semaphore access (RT)");
        exit(EXIT_FAILURE);
    }

    ////////////////////////////////////////////
    // TODO insert your code here
    sh->fSt.st.receptionistStat = ASSIGNTABLE;
    saveState(nFic, &sh->fSt);

    switch(sh->fSt.assignedTable[n] = decideTableOrWait(n)) {
        case -1:
            sh->fSt.groupsWaiting++;
            groupRecord[n] = WAIT;
            break;
        default:
            groupRecord[n] = ATTABLE;
            if (semUp(semgid, sh->waitForTable[n] ) == -1)
            {
                perror("error on the up operation for semaphore access (RT)");
                exit(EXIT_FAILURE);
            }    
    }
    ////////////////////////////////////////////

    if (semUp(semgid, sh->mutex) == -1)
    { /* exit critical region */
        perror("error on the up operation for semaphore access (RT)");
        exit(EXIT_FAILURE);
    }
}

/**
 *  \brief receptionist receives payment
 *
 *  Receptionist updates its state and receives payment.
 *  If there are waiting groups, receptionist should check if table that just became
 *  vacant should be occupied. Shared (and internal) memory should be updated.
 *  The internal state should be saved.
 *
 */

static void receivePayment(int n)
{   
    
    if (semDown(semgid, sh->mutex) == -1)
    { /* enter critical region */
        perror("error on the down operation for semaphore access (RT)");
        exit(EXIT_FAILURE);
    }
    
    ////////////////////////////////////////////
    // TODO insert your code here
    sh->fSt.st.receptionistStat = RECVPAY;
    saveState(nFic, &sh->fSt);

    groupRecord[n] = DONE;

    int vacant_table = sh->fSt.assignedTable[n];
    sh->fSt.assignedTable[n] = -1;

    int next_group = decideNextGroup();
    if(next_group != -1) {
        sh->fSt.assignedTable[next_group] = vacant_table; // !!
        if (semUp(semgid, sh->waitForTable[next_group] ) == -1)
        { 
            perror("error on the up operation for semaphore access (RT)");
            exit(EXIT_FAILURE);
        } 
        sh->fSt.groupsWaiting--;  
        groupRecord[next_group] = ATTABLE;
    }
    
    ////////////////////////////////////////////

    if (semUp(semgid, sh->mutex) == -1)
    { /* exit critical region */
        perror("error on the up operation for semaphore access (RT)");
        exit(EXIT_FAILURE);
    }

    ////////////////////////////////////////////
    // TODO insert your code here
    if (semUp(semgid, sh->tableDone[vacant_table]) == -1)
    { 
        perror("error on the up operation for semaphore access (RT)");
        exit(EXIT_FAILURE);
    }
    ////////////////////////////////////////////

}
