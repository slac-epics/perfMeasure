TOP=../..

include $(TOP)/configure/CONFIG

#=============================
# build a library

# for gdb/ddd debugger:
USR_CFLAGS += -O
USR_CFLAGS_solaris-sparc-gnu += -g
USR_CFLAGS_linux-x86 += -g
# for debug print statements and slc-awareness
USR_CFLAGS += -DDEBUG_PRINT
# Use EVR driver routines
USR_CFLAGS_RTEMS += -DEVR_DRIVER_SUPPORT
ifeq ($(EVR_DRIVER_SUPPORT_LINUX), YES)
USR_CFLAGS_Linux += -DEVR_DRIVER_SUPPORT
endif


SRC_DIRS += $(TOP)/perfMeasure/src

INC+= perfMeasure.h

# DBDINC += 

DBD += drvPerfMeasure.dbd

LIBRARY_IOC = perfMeasure

perfMeasure_SRCS += perfMeasure.c
perfMeasure_SRCS += stress.c
perfMeasure_SRCS += drvPerfMeasure.c
perfMeasure_LIBS += $(EPICS_BASE_IOC_LIBS)

#===========================

include $(TOP)/configure/RULES

#----------------------------------------
#  ADD RULES AFTER THIS LINE

