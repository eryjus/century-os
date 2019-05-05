#!/bin/bash


##
## -- This is the version of crosstool-ng to install; change if you require a different version
##    -----------------------------------------------------------------------------------------
VER=1.24.0
CWD=`pwd`


##
## -- If we are running as root, we cannot continue
##    ---------------------------------------------
if [[ $EUID -eq 0 ]]; then
   echo "This script cannot be run as root; it will ask for your password if needed" 
   exit 1
fi


##
## -- at some point here, I will need to check to see if all the required compnents are installed
##    -------------------------------------------------------------------------------------------



##
## -- Make the destination folders
##    ----------------------------
mkdir -p ~/opt/bin ~/opt/ct-ng ~/opt/cross ~/Downloads


##
## -- Now, go get the crosstool-ng source and extract it to the opt folder
##    --------------------------------------------------------------------
cd ~/Downloads
rm -f crosstool-ng*.tar.bz2
wget http://crosstool-ng.org/download/crosstool-ng/crosstool-ng-$VER.tar.bz2
cd ~/opt/ct-ng
rm -fR *
tar -xaf ~/Downloads/crosstool-ng-$VER.tar.bz2
mv crosstool-ng-$VER/* .
rm -fR crosstool-ng-$VER


##
## -- Now, build ct-ng
##    ----------------
./configure --enable-local
make
PATH=~/opt/ct-ng:$PATH
ct-ng --version


##
## -- now, prepare to build the i686-elf cross compiler
##    -------------------------------------------------
cd ~/opt/cross
cp $CWD/.config.i686-elf ./.config
ct-ng upgradeconfig
ct-ng build
cd ~/opt/bin
rm -f i686-elf-*
ln -s ~/opt/cross/i686-elf/bin/* .


##
## -- now, prepare to build the arm-eabi cross compiler
##    -------------------------------------------------
cd ~/opt/cross
cp $CWD/.config.arm-eabi ./.config
ct-ng upgradeconfig
ct-ng build
cd ~/opt/bin
rm -f arm-eabi-*
ln -s ~/opt/cross/arm-eabi/bin/* .


##
## -- now, prepare to build the i686-elf cross compiler
##    -------------------------------------------------
cd ~/opt/cross
cp $CWD/.config.arm-none-eabi ./.config
ct-ng upgradeconfig
ct-ng build
cd ~/opt/bin
rm -f arm-none-eabi-*
ln -s ~/opt/cross/arm-none-eabi/bin/* .


##
## -- now, prepare to build the i686-elf cross compiler
##    -------------------------------------------------
cd ~/opt/cross
cp $CWD/.config.armv7-rpi2-linux-gnueabihf ./.config
ct-ng upgradeconfig
ct-ng build
cd ~/opt/bin
rm -f armv7-rpi2-linux-gnueabihf-*
ln -s ~/opt/cross/armv7-rpi2-linux-gnueabihf/bin/* .


