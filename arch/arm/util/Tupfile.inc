#####################################################################################################################
##
##  Tupfile -- An alternative to 'make` build system -- build the object files for the arm architecture
##
##        Copyright (c)  2017-2019 -- Adam Clark
##
## -----------------------------------------------------------------------------------------------------------------
##
##     Date      Tracker  Version  Pgmr  Description
##  -----------  -------  -------  ----  ---------------------------------------------------------------------------
##  2019-Mar-03  Initial   0.3.0   ADCL  Initial version
##
#####################################################################################################################


##
## -- These are the commands that will be used to build for that arm architecture
##    ---------------------------------------------------------------------------
AR = armv7-rpi2-linux-gnueabihf-ar
AS = armv7-rpi2-linux-gnueabihf-as
CC = armv7-rpi2-linux-gnueabihf-gcc
LD = armv7-rpi2-linux-gnueabihf-ld
OC = armv7-rpi2-linux-gnueabihf-objcopy



##
## -- Add some options to the build
##    -----------------------------
CFLAGS += -mcpu=cortex-a7
CFLAGS += -marm
CFLAGS += -mfpu=vfpv3
CFLAGS += -mfloat-abi=softfp

