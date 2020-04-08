#####################################################################################################################
##
##  Tupfile -- An alternative to 'make` build system -- build the object files for the arm architecture
##
##        Copyright (c)  2017-2020 -- Adam Clark
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
AR = arm-none-eabi-ar
AS = arm-none-eabi-as
CC = arm-none-eabi-gcc
LD = arm-none-eabi-ld
OC = arm-none-eabi-objcopy



##
## -- Add some options to the build
##    -----------------------------
CFLAGS += -mlittle-endian
CFLAGS += -mcpu=cortex-a7
CFLAGS += -mfpu=neon-vfpv4
CFLAGS += -mfloat-abi=hard


