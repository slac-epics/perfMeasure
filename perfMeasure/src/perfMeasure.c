#include <stdint.h>
#include <stdlib.h>
#include <stddef.h>             /* size_t                      */
#include <string.h>             /* strcmp                      */
#include <stdio.h>              /* printf                      */
#ifdef __rtems__
#include <rtems.h>              /* timer routines              */
#include <rtems/timerdrv.h>     /* timer routines              */
#include <bsp.h>                /* BSP*                        */
#if (__RTEMS_MAJOR__ > 4) || (__RTEMS_MAJOR__ == 4 && __RTEMS_MINOR__ >= 9)
#define Timer_initialize benchmark_timer_initialize
#define Read_timer benchmark_timer_read
#endif
#endif

#include "epicsMutex.h"
#include "epicsThread.h"        /* epicsThreadSleep()          */
#include "ellLib.h"
#include "errlog.h"

#include "perfMeasure.h"


#ifdef __rtems__
/*
 * From Till Straumann:
 * Macro for "Move From Time Base" to get current time in ticks.
 * The PowerPC Time Base is a 64-bit register incrementing usually
 * at 1/4 of the PPC bus frequency (which is CPU/Board dependent.
 * Even the 1/4 divisor is not fixed by the architecture).
 *
 * 'MFTB' just reads the lower 32-bit of the time base.
 */
#ifdef __PPC__
#define MFTB(var) asm volatile("mftb %0":"=r"(var))
#else
#define MFTB(var) (var)=Read_timer()
#endif
#endif

#if defined(i386) || defined(__i386) || defined(__i386__) || defined(_M_IX86) || defined(_X86_)  /*  for 32bits */ \
     || defined(__x86_64) || defined(__x86_64__) || defined(_M_X64)                              /*  for 64bits */
/* __inline__ static unsigned long long int rdtsc(void)
{
        unsigned long long int x;
        __asm__ volatile (".byte 0x0f, 0x31": "=A" (x));
        return x;
} */

__inline__ static uint64_t rdtsc(void)
{
    uint32_t lo, hi;
    
    __asm__ __volatile__("rdtsc" : "=a"(lo), "=d"(hi));

    return (((uint64_t)hi) << 32) | ((uint64_t)lo);
}

#define MFTB(var)  ((var)=(uint32_t) rdtsc())
#endif

static int    initialized = 0;
static double clockTicksPerUsec = 1.5E+3;  /* need to fix it to avoid hardcoding */
static double clockTicksPerSec;
static ELLLIST perfParm_s;


static void  Get_clockTicksPerUsec(void)
{
    uint32_t start, stop;

    do {
      MFTB(start);
      epicsThreadSleep(1.);
      MFTB(stop);
    } while(!(stop>start));

    clockTicksPerSec  = (double)(stop-start);
    clockTicksPerUsec = clockTicksPerSec * 1.E-6;
}


void initPerfMeasure(void)
{
    if(initialized) return;

    ellInit(&perfParm_s);
    Get_clockTicksPerUsec();

    initialized = 1;
}

void resetPerfMeasure(perfParm_ts *p)
{
    if(!p) return ;

    epicsMutexLock(p->lock);

    p->enb = 1;
    p->enb_mutex = 1;

    p->cnt = 0;
    p->diff = 0;
    p->diff_min = 100000000;
    p->diff_max = 0;
    p->elapsed_time = 0.;
    p->elapsed_time_min = 0.;
    p->elapsed_time_max = 0.;

   epicsMutexUnlock(p->lock);
}

void enbPerfMeasure(perfParm_ts *p, unsigned enb)
{
    if(!p) return;

    epicsMutexLock(p->lock);
    p->enb = enb;
    epicsMutexUnlock(p->lock);
}


perfParm_ts* makePerfMeasure(char *name, char *description)
{
    perfParm_ts *p;

    if(!initialized) initPerfMeasure();

    p = (perfParm_ts*) malloc(sizeof(perfParm_ts));
    if(!p) {
        errlogPrintf("%s@%d(malloc): fail for (%s-%s)\n", __func__, __LINE__, name, description);
        return NULL;
    }

    strncpy(p->name, name, 32);
    strncpy(p->description, description, 80);
    p->lock = epicsMutexMustCreate();

    resetPerfMeasure(p);


    ellAdd(&perfParm_s, &p->node);

    return  p;
}

perfParm_ts* findPerfMeasure(char *name)
{
    perfParm_ts *p = NULL;

    if(ellCount(&perfParm_s)==0) return p;

    p = (perfParm_ts*) ellFirst(&perfParm_s);
    while(p) {
        if(!strncmp(name, p->name, 32)) break;
        p = (perfParm_ts*) ellNext(&p->node);
    }
    
    return p;
}


void startPerfMeasure(perfParm_ts* p)
{
    if(!p || !p->enb) return;

     epicsMutexLock(p->lock);
     MFTB(p->scratch_pad[0]);
     epicsMutexUnlock(p->lock);
}

void endPerfMeasure(perfParm_ts* p)
{
    if(!p || !p->enb) return;

    MFTB(p->scratch_pad[1]);

    if(p->scratch_pad[0] > p->scratch_pad[1]) return;


    epicsMutexLock(p->lock);
    p->cnt++;
    p->diff = p->scratch_pad[1] - p->scratch_pad[0];
    if(p->diff < p->diff_min) p->diff_min = p->diff;
    if(p->diff > p->diff_max) p->diff_max = p->diff;
    epicsMutexUnlock(p->lock);
}


void calcPerfMeasure(perfParm_ts *p)
{
    if(!p || !p->enb) return;

    epicsMutexLock(p->lock);
    
    p->elapsed_time     = (double) p->diff     / clockTicksPerUsec;
    p->elapsed_time_min = (double) p->diff_min / clockTicksPerUsec;
    p->elapsed_time_max = (double) p->diff_max / clockTicksPerUsec;

    epicsMutexUnlock(p->lock);

}

static void listNodesPerfMeasure(void)
{
    perfParm_ts *p;
    p = (perfParm_ts *) ellFirst(&perfParm_s);
         /* 0        1         2         3         4         5         6         7         8         9 */
         /* 123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890 */
    printf("-----------------------   --------------------------------------------\n");
    printf("       Node name                         Description\n");
    printf("-----------------------   --------------------------------------------\n");
    while(p) {
        printf("%16s           %s\n", p->name, p->description);
        p = (perfParm_ts*) ellNext(&p->node);
    }
    printf("-----------------------   --------------------------------------------\n");

}

static void displayNodePerfMeasure(perfParm_ts *p)
{
    if(!p) return;

    calcPerfMeasure(p);
    printf("%16s %3d %12u %12.8lf %12.8lf %12.8lf %s\n", p->name, p->enb, p->cnt, p->elapsed_time, p->elapsed_time_min, p->elapsed_time_max, p->description);

}


void reportPerfMeasure(int interest)
{
    if(!initialized) {
        printf("Driver never been initialized....\n");
        return ;
    }
    printf("Estimated Clock Speed: %lf MHz\n", clockTicksPerUsec);
    printf("Driver has %d measurement point(s) now...\n", ellCount(&perfParm_s));
    
    if(interest == 1) listNodesPerfMeasure();
    if(interest>1) {
        perfParm_ts *p;

         /* 0        1         2         3         4         5         6         7         8         9 */
         /* 123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890 */
    printf("--------------------------------------------------------------------------------------------\n");
    printf("    Node name     Enb         Counter   Time(usec)    Minimum      Maximum          Description\n");
    printf("--------------------------------------------------------------------------------------------\n");
        p = (perfParm_ts*) ellFirst(&perfParm_s);
        while(p) {
            displayNodePerfMeasure(p);
            p = (perfParm_ts*) ellNext(&p->node);
        }
    printf("--------------------------------------------------------------------------------------------\n");

    }
}


