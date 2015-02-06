#include <stdlib.h>
#include <drvSup.h>
#include <errlog.h>
#include <epicsExport.h>

#include "perfMeasure.h"
#include "stress.h"


static int perfMeasureReport(int interest);
static int perfMeasureInitialize(void);

struct drvet drvPerfMeasure = {
    2,
    (DRVSUPFUN) perfMeasureReport,
    (DRVSUPFUN) perfMeasureInitialize
};

epicsExportAddress(drvet, drvPerfMeasure);



static int perfMeasureReport(int interest)
{
    reportPerfMeasure(interest);
    reportStressThreads(interest);

    return 0;
}


static int perfMeasureInitialize(void)
{
    initPerfMeasure();
    initStress_perfMeasure();

    return 0;
}


