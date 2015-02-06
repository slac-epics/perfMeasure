#ifndef INCstressH
#define INCstressH

#include <ellLib.h>
#include <epicsThread.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    ELLNODE               node;
    char                  name[32];
    epicsThreadId         tid;
    volatile unsigned int counter;
    volatile unsigned int suspendFlag;
} stressThread_t;

void initStress_perfMeasure(void);
int  createStressThreads(unsigned int nthread, unsigned int priority);
int  reportStressThreads(int interest);
int  suspendStressThreads(void);
int  resumeStressThreads(void);


#ifdef __cplusplus
}
#endif

#endif     /* INCstressH */
