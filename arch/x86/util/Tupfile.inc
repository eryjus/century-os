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
AR = i686-elf-ar
AS = nasm
CC = i686-elf-gcc
LD = i686-elf-ld
OC = i686-elf-objcopy



##
## -- Add some options to the build
##    -----------------------------
CFLAGS += -mno-red-zone
CFLAGS += -fno-pic
CFLAGS += -march=i586
