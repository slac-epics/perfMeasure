#Makefile at top of application tree
TOP = .
include $(TOP)/configure/CONFIG

DIRS += configure
DIRS += perfMeasure

include $(TOP)/configure/RULES_TOP
