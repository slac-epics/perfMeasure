#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>
#ifdef __rtems__
#include <rtems.h>
#include <rtems/timerdrv.h>
#include <bsp.h>
#endif

#include "epicsMutex.h"
#include "epicsThread.h"
#include "epicsPrint.h"
#include "ellLib.h"
#include "errlog.h"
#include "iocsh.h"
#include "epicsExport.h"

#include "stress.h"


static ELLLIST stressParm_s;
static int   initialized = 0;


static void stress_func(void *pArg)
{
   stressThread_t *p = pArg;
   if(!p) return;

   while(1) {
       while(!p->suspendFlag) p->counter++;
       epicsThreadSuspendSelf();
   }  
}


void initStress_perfMeasure(void)
{
    if(initialized) return;

    ellInit(&stressParm_s);
    initialized = 1;
}

int createStressThreads(unsigned int nthread, unsigned int priority)
{
    int i;
    char name[32];
    stressThread_t *p;
    initStress_perfMeasure();

    if(!nthread)   nthread = 1;
    if(!priority)  priority = epicsThreadPriorityLow;

    for(i=0; i<nthread; i++) {
        p = (stressThread_t*) malloc(sizeof(stressThread_t));
        if(!p) return -1;

        sprintf(name, "stress_%d", ellCount(&stressParm_s));

        strcpy(p->name, name);
        p->counter     = 0;
        p->suspendFlag = 0;

        p->tid = epicsThreadCreate(name, priority,
                                   epicsThreadGetStackSize(epicsThreadStackSmall),
                                   (EPICSTHREADFUNC) stress_func,
                                   (void*) p);
        
        ellAdd(&stressParm_s, &p->node);
    }


    return 0;
}

int reportStressThreads(int interest)
{
    stressThread_t *p;

    if(!initialized) {
        epicsPrintf("The stress test never initialized...\n");
        return -1;
    }

    epicsPrintf("Number of Stress test threads: %d\n", ellCount(&stressParm_s));
    if(interest) {
        p = (stressThread_t*) ellFirst(&stressParm_s);
        while(p) {
            if(p->suspendFlag) epicsPrintf("\t%s is suspended\n", p->name);
            else               epicsPrintf("\t%s is active\n",    p->name);
            p = (stressThread_t*) ellNext(&p->node);
        }
    }
    return 0;
}

int suspendStressThreads(void)
{
    stressThread_t *p = NULL;

    if(ellCount(&stressParm_s) == 0) return -1;

    p = (stressThread_t*) ellFirst(&stressParm_s);
    while(p) {
        p->suspendFlag = 1;
        p = (stressThread_t*) ellNext(&p->node);
    }

    return 0;
}

int resumeStressThreads(void)
{
    stressThread_t *p = NULL;
 
    if(ellCount(&stressParm_s) == 0) return -1;

    p = (stressThread_t*) ellFirst(&stressParm_s);
    while(p) {
        if(p->suspendFlag) {
            epicsThreadResume(p->tid);
            p->suspendFlag = 0;
        }
        p = (stressThread_t*) ellNext(&p->node);
    }

    return 0;
}



static const iocshArg         createStressThreadsArg0   = {"number of threads", iocshArgInt};
static const iocshArg         createStressThreadsArg1   = {"thread priority",   iocshArgInt};
static const iocshArg *const  createStressThreadsArgs[] = { &createStressThreadsArg0,
                                                            &createStressThreadsArg1 };
static const iocshFuncDef     createStressThreadsDef    = { "createStressThreads", 2, createStressThreadsArgs };
static const iocshFuncDef     suspendStressThreadsDef   = { "suspendStressThreads", 0, NULL};
static const iocshFuncDef     resumeStressThreadsDef    = { "resumeStressThreads", 0, NULL};

static void  createStressThreadsCall(const iocshArgBuf *args)
{
    createStressThreads(args[0].ival, args[1].ival);
}

static void suspendStressThreadsCall(const iocshArgBuf *args)
{
    suspendStressThreads();
}

static void resumeStressThreadsCall(const iocshArgBuf *args)
{
    resumeStressThreads();
}

static void drvPerfMeasure_stressThreadsRegistrar(void)
{
    iocshRegister(&createStressThreadsDef,   createStressThreadsCall);
    iocshRegister(&suspendStressThreadsDef,  suspendStressThreadsCall);
    iocshRegister(&resumeStressThreadsDef,   resumeStressThreadsCall);
}

epicsExportRegistrar(drvPerfMeasure_stressThreadsRegistrar);



