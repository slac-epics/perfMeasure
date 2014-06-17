#ifndef INCperfMeasureH
#define INCperfMeasureH

#include <ellLib.h>
#include <epicsMutex.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    ELLNODE   node;              /* node information for the linked list */
    char      name[32];          /* node name for the indexing */
    char      description[80];   /* description */

    epicsMutexId lock;

    unsigned  scratch_pad[2];
    unsigned  enb;
    unsigned  enb_mutex;

    unsigned  cnt;               /* counter */
    unsigned  diff;              /* tick counter difference between starting and ending */
    unsigned  diff_min;
    unsigned  diff_max;
    double    elapsed_time;      /* elapsed time in usec */
    double    elapsed_time_min;
    double    elapsed_time_max;
} perfParm_ts;



void  initPerfMeasure(void);
void resetPerfMeasure(perfParm_ts *p);
void enbPerfMeasure(perfParm_ts *p, unsigned enb);
perfParm_ts* makePerfMeasure(char *name, char *description);
perfParm_ts* findPerfMeasure(char *name);
void startPerfMeasure(perfParm_ts* p);
void endPerfMeasure(perfParm_ts* p);
void calcPerfMeasure(perfParm_ts *p);
void reportPerfMeasure(int interest);


#ifdef __cplusplus
}
#endif

#endif     /* INCperfMeasureH */
