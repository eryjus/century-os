//===================================================================================================================
//
//  platform-gpio.h -- Definitions and functions for the GPIO hardware for bcm2835
//
//        Copyright (c)  2017-2020 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2019-Feb-24  Initial   0.3.0   ADCL  Initial version
//
//===================================================================================================================


#ifndef __HARDWARE_H__
#   error "Use #include \"hardware.h\" and it will pick up this file; do not #include this file directly."
#endif


//
// -- This is the enumeration of the GPIO pins that are available for use
//    -------------------------------------------------------------------
typedef enum {
    GPIO0,  GPIO1,  GPIO2,  GPIO3,  GPIO4,  GPIO5,  GPIO6,  GPIO7,  GPIO8,  GPIO9,
    GPIO10, GPIO11, GPIO12, GPIO13, GPIO14, GPIO15, GPIO16, GPIO17, GPIO18, GPIO19,
    GPIO20, GPIO21, GPIO22, GPIO23, GPIO24, GPIO25, GPIO26, GPIO27, GPIO28, GPIO29,
    GPIO30, GPIO31, GPIO32, GPIO33, GPIO34, GPIO35, GPIO36, GPIO37, GPIO38, GPIO39,
    GPIO40, GPIO41, GPIO42, GPIO43, GPIO44, GPIO45, GPIO46, GPIO47, GPIO48, GPIO49,
    GPIO50, GPIO51, GPIO52, GPIO53,
} GpioPin_t;


//
// -- These are the class of alternate function that can be used with each GPIO pin
//    -----------------------------------------------------------------------------
typedef enum {
    ALT0, ALT1, ALT2, ALT3, ALT4, ALT5,
} GpioAlt_t;


//
// -- Define a common interface for the GPIO functions that are needed
//    ----------------------------------------------------------------
typedef struct GpioDevice_t {
    archsize_t base;

    void (*GpioSelectAlt)(struct GpioDevice_t *, GpioPin_t, GpioAlt_t);
    void (*GpioEnablePin)(struct GpioDevice_t *, GpioPin_t);
    void (*GpioDisablePin)(struct GpioDevice_t *, GpioPin_t);
} GpioDevice_t;


//
// -- Here, declare the different configurations of the GPIO will use
//    ---------------------------------------------------------------
extern GpioDevice_t loaderGpio;
extern GpioDevice_t kernelGpio;


//
// -- These are the common interface functions we will use to interact with the GPIO.  These functions are
//    not safe in that they will not check for nulls before calling the function.  Therefore, caller beware!
//    -----------------------------------------------------------------------------------------------------------
inline void GpioSelectAlt(GpioDevice_t *dev, GpioPin_t pin, GpioAlt_t alt) { dev->GpioSelectAlt(dev, pin, alt); }
inline void GpioEnablePin(GpioDevice_t *dev, GpioPin_t pin) { dev->GpioEnablePin(dev, pin); }
inline void GpioDisablePin(GpioDevice_t *dev, GpioPin_t pin) { dev->GpioDisablePin(dev, pin); }


//
// -- Here are the function prototypes needed for these operations
//    ------------------------------------------------------------
extern void _GpioSelectAlt(GpioDevice_t *dev, GpioPin_t pin, GpioAlt_t alt);
extern void _GpioEnablePin(GpioDevice_t *dev, GpioPin_t pin);
extern void _GpioDisablePin(GpioDevice_t *dev, GpioPin_t pin);


//
// -- define the base locations for both the loader and the kernel versions
//    ---------------------------------------------------------------------
#define LDR_GPIO_BASE       (MMIO_LOADER_LOC + 0x200000)
#define KRN_GPIO_BASE       (MMIO_VADDR + 0x200000)


//
// -- since there is a lot of duplication, reuse these values
//    -------------------------------------------------------
#define GPIO_PININ          (0b000)                     // Pin is an input
#define GPIO_PINOUT         (0b001)                     // Pin is an output
#define GPIO_AFUNC0         (0b100)                     // Takes alternate function 0
#define GPIO_AFUNC1         (0b101)                     // Takes alternate function 1
#define GPIO_AFUNC2         (0b110)                     // Takes alternate function 2
#define GPIO_AFUNC3         (0b111)                     // Takes alternate function 3
#define GPIO_AFUNC4         (0b011)                     // Takes alternate function 4
#define GPIO_AFUNC5         (0b110)                     // Takes alternate function 5


#define GPIO_FSEL0          (0x0)                       // GPIO Function Select 0
//-------------------------------------------------------------------------------------------------------------------
#define GPIOFSEL0_SEL9      (7<<27)                     // Function Select 9
#define GPIOFSEL0_SEL8      (7<<24)                     // Function Select 8
#define GPIOFSEL0_SEL7      (7<<21)                     // Function Select 7
#define GPIOFSEL0_SEL6      (7<<18)                     // Function Select 6
#define GPIOFSEL0_SEL5      (7<<15)                     // Function Select 5
#define GPIOFSEL0_SEL4      (7<<12)                     // Function Select 4
#define GPIOFSEL0_SEL3      (7<<9)                      // Function Select 3
#define GPIOFSEL0_SEL2      (7<<6)                      // Function Select 2
#define GPIOFSEL0_SEL1      (7<<3)                      // Function Select 1
#define GPIOFSEL0_SEL0      (7<<0)                      // Function Select 0

#define SH_SEL9(x)          (((x)&0x7)<<27)             // Shift to the proper bits
#define SH_SEL8(x)          (((x)&0x7)<<24)             // Shift to the proper bits
#define SH_SEL7(x)          (((x)&0x7)<<21)             // Shift to the proper bits
#define SH_SEL6(x)          (((x)&0x7)<<18)             // Shift to the proper bits
#define SH_SEL5(x)          (((x)&0x7)<<15)             // Shift to the proper bits
#define SH_SEL4(x)          (((x)&0x7)<<12)             // Shift to the proper bits
#define SH_SEL3(x)          (((x)&0x7)<<9)              // Shift to the proper bits
#define SH_SEL2(x)          (((x)&0x7)<<6)              // Shift to the proper bits
#define SH_SEL1(x)          (((x)&0x7)<<3)              // Shift to the proper bits
#define SH_SEL0(x)          (((x)&0x7)<<0)              // Shift to the proper bits


#define GPIO_FSEL1          (4)                         // GPIO Function Select 1
//-------------------------------------------------------------------------------------------------------------------
#define GPIOFSEL1_SEL19     (7<<27)                     // Function Select 19
#define GPIOFSEL1_SEL18     (7<<24)                     // Function Select 18
#define GPIOFSEL1_SEL17     (7<<21)                     // Function Select 17
#define GPIOFSEL1_SEL16     (7<<18)                     // Function Select 16
#define GPIOFSEL1_SEL15     (7<<15)                     // Function Select 15
#define GPIOFSEL1_SEL14     (7<<12)                     // Function Select 14
#define GPIOFSEL1_SEL13     (7<<9)                      // Function Select 13
#define GPIOFSEL1_SEL12     (7<<6)                      // Function Select 12
#define GPIOFSEL1_SEL11     (7<<3)                      // Function Select 11
#define GPIOFSEL1_SEL10     (7<<0)                      // Function Select 10

#define SH_SEL19(x)         (((x)&0x7)<<27)             // Shift to the proper bits
#define SH_SEL18(x)         (((x)&0x7)<<24)             // Shift to the proper bits
#define SH_SEL17(x)         (((x)&0x7)<<21)             // Shift to the proper bits
#define SH_SEL16(x)         (((x)&0x7)<<18)             // Shift to the proper bits
#define SH_SEL15(x)         (((x)&0x7)<<15)             // Shift to the proper bits
#define SH_SEL14(x)         (((x)&0x7)<<12)             // Shift to the proper bits
#define SH_SEL13(x)         (((x)&0x7)<<9)              // Shift to the proper bits
#define SH_SEL12(x)         (((x)&0x7)<<6)              // Shift to the proper bits
#define SH_SEL11(x)         (((x)&0x7)<<3)              // Shift to the proper bits
#define SH_SEL10(x)         (((x)&0x7)<<0)              // Shift to the proper bits


#define GPIO_FSEL2          (8)                         // GPIO Function Select 2
//-------------------------------------------------------------------------------------------------------------------
#define GPIOFSEL2_SEL29     (7<<27)                     // Function Select 29
#define GPIOFSEL2_SEL28     (7<<24)                     // Function Select 28
#define GPIOFSEL2_SEL27     (7<<21)                     // Function Select 27
#define GPIOFSEL2_SEL26     (7<<18)                     // Function Select 26
#define GPIOFSEL2_SEL25     (7<<15)                     // Function Select 25
#define GPIOFSEL2_SEL24     (7<<12)                     // Function Select 24
#define GPIOFSEL2_SEL23     (7<<9)                      // Function Select 23
#define GPIOFSEL2_SEL22     (7<<6)                      // Function Select 22
#define GPIOFSEL2_SEL21     (7<<3)                      // Function Select 21
#define GPIOFSEL2_SEL20     (7<<0)                      // Function Select 20

#define SH_SEL29(x)         (((x)&0x7)<<27)             // Shift to the proper bits
#define SH_SEL28(x)         (((x)&0x7)<<24)             // Shift to the proper bits
#define SH_SEL27(x)         (((x)&0x7)<<21)             // Shift to the proper bits
#define SH_SEL26(x)         (((x)&0x7)<<18)             // Shift to the proper bits
#define SH_SEL25(x)         (((x)&0x7)<<15)             // Shift to the proper bits
#define SH_SEL24(x)         (((x)&0x7)<<12)             // Shift to the proper bits
#define SH_SEL23(x)         (((x)&0x7)<<9)              // Shift to the proper bits
#define SH_SEL22(x)         (((x)&0x7)<<6)              // Shift to the proper bits
#define SH_SEL21(x)         (((x)&0x7)<<3)              // Shift to the proper bits
#define SH_SEL20(x)         (((x)&0x7)<<0)              // Shift to the proper bits


#define GPIO_FSEL3          (0xc)                       // GPIO Function Select 3
//-------------------------------------------------------------------------------------------------------------------
#define GPIOFSEL3_SEL39     (7<<27)                     // Function Select 39
#define GPIOFSEL3_SEL38     (7<<24)                     // Function Select 38
#define GPIOFSEL3_SEL37     (7<<21)                     // Function Select 37
#define GPIOFSEL3_SEL36     (7<<18)                     // Function Select 36
#define GPIOFSEL3_SEL35     (7<<15)                     // Function Select 35
#define GPIOFSEL3_SEL34     (7<<12)                     // Function Select 34
#define GPIOFSEL3_SEL33     (7<<9)                      // Function Select 33
#define GPIOFSEL3_SEL32     (7<<6)                      // Function Select 32
#define GPIOFSEL3_SEL31     (7<<3)                      // Function Select 31
#define GPIOFSEL3_SEL30     (7<<0)                      // Function Select 30

#define SH_SEL39(x)         (((x)&0x7)<<27)             // Shift to the proper bits
#define SH_SEL38(x)         (((x)&0x7)<<24)             // Shift to the proper bits
#define SH_SEL37(x)         (((x)&0x7)<<21)             // Shift to the proper bits
#define SH_SEL36(x)         (((x)&0x7)<<18)             // Shift to the proper bits
#define SH_SEL35(x)         (((x)&0x7)<<15)             // Shift to the proper bits
#define SH_SEL34(x)         (((x)&0x7)<<12)             // Shift to the proper bits
#define SH_SEL33(x)         (((x)&0x7)<<9)              // Shift to the proper bits
#define SH_SEL32(x)         (((x)&0x7)<<6)              // Shift to the proper bits
#define SH_SEL31(x)         (((x)&0x7)<<3)              // Shift to the proper bits
#define SH_SEL30(x)         (((x)&0x7)<<0)              // Shift to the proper bits


#define GPIO_FSEL4          (0x10)                      // GPIO Function Select 4
//-------------------------------------------------------------------------------------------------------------------
#define GPIOFSEL4_SEL49     (7<<27)                     // Function Select 49
#define GPIOFSEL4_SEL48     (7<<24)                     // Function Select 48
#define GPIOFSEL4_SEL47     (7<<21)                     // Function Select 47
#define GPIOFSEL4_SEL46     (7<<18)                     // Function Select 46
#define GPIOFSEL4_SEL45     (7<<15)                     // Function Select 45
#define GPIOFSEL4_SEL44     (7<<12)                     // Function Select 44
#define GPIOFSEL4_SEL43     (7<<9)                      // Function Select 43
#define GPIOFSEL4_SEL42     (7<<6)                      // Function Select 42
#define GPIOFSEL4_SEL41     (7<<3)                      // Function Select 41
#define GPIOFSEL4_SEL40     (7<<0)                      // Function Select 40

#define SH_SEL49(x)         (((x)&0x7)<<27)             // Shift to the proper bits
#define SH_SEL48(x)         (((x)&0x7)<<24)             // Shift to the proper bits
#define SH_SEL47(x)         (((x)&0x7)<<21)             // Shift to the proper bits
#define SH_SEL46(x)         (((x)&0x7)<<18)             // Shift to the proper bits
#define SH_SEL45(x)         (((x)&0x7)<<15)             // Shift to the proper bits
#define SH_SEL44(x)         (((x)&0x7)<<12)             // Shift to the proper bits
#define SH_SEL43(x)         (((x)&0x7)<<9)              // Shift to the proper bits
#define SH_SEL42(x)         (((x)&0x7)<<6)              // Shift to the proper bits
#define SH_SEL41(x)         (((x)&0x7)<<3)              // Shift to the proper bits
#define SH_SEL40(x)         (((x)&0x7)<<0)              // Shift to the proper bits


#define GPIO_FSEL5          (0x14)                      // GPIO Function Select 5
//-------------------------------------------------------------------------------------------------------------------
#define GPIOFSEL5_SEL53     (7<<9)                      // Function Select 53
#define GPIOFSEL5_SEL52     (7<<6)                      // Function Select 52
#define GPIOFSEL5_SEL51     (7<<3)                      // Function Select 51
#define GPIOFSEL5_SEL50     (7<<0)                      // Function Select 50

#define SH_SEL53(x)         (((x)&0x7)<<9)              // Shift to the proper bits
#define SH_SEL52(x)         (((x)&0x7)<<6)              // Shift to the proper bits
#define SH_SEL51(x)         (((x)&0x7)<<3)              // Shift to the proper bits
#define SH_SEL50(x)         (((x)&0x7)<<0)              // Shift to the proper bits



#define GPIO_SET0           (0x1c)                      // GPIO Pin Output Set 0
//-------------------------------------------------------------------------------------------------------------------
#define GPIOSET0_31         (1<<31)                     // Set GPIO pin 31
#define GPIOSET0_30         (1<<30)                     // Set GPIO pin 30
#define GPIOSET0_29         (1<<29)                     // Set GPIO pin 29
#define GPIOSET0_28         (1<<28)                     // Set GPIO pin 28
#define GPIOSET0_27         (1<<27)                     // Set GPIO pin 27
#define GPIOSET0_26         (1<<26)                     // Set GPIO pin 26
#define GPIOSET0_25         (1<<25)                     // Set GPIO pin 25
#define GPIOSET0_24         (1<<24)                     // Set GPIO pin 24
#define GPIOSET0_23         (1<<23)                     // Set GPIO pin 23
#define GPIOSET0_22         (1<<22)                     // Set GPIO pin 22
#define GPIOSET0_21         (1<<21)                     // Set GPIO pin 21
#define GPIOSET0_20         (1<<20)                     // Set GPIO pin 20
#define GPIOSET0_19         (1<<19)                     // Set GPIO pin 19
#define GPIOSET0_18         (1<<18)                     // Set GPIO pin 18
#define GPIOSET0_17         (1<<17)                     // Set GPIO pin 17
#define GPIOSET0_16         (1<<16)                     // Set GPIO pin 16
#define GPIOSET0_15         (1<<15)                     // Set GPIO pin 15
#define GPIOSET0_14         (1<<14)                     // Set GPIO pin 14
#define GPIOSET0_13         (1<<13)                     // Set GPIO pin 13
#define GPIOSET0_12         (1<<12)                     // Set GPIO pin 12
#define GPIOSET0_11         (1<<11)                     // Set GPIO pin 11
#define GPIOSET0_10         (1<<10)                     // Set GPIO pin 10
#define GPIOSET0_9          (1<<9)                      // Set GPIO pin 9
#define GPIOSET0_8          (1<<8)                      // Set GPIO pin 8
#define GPIOSET0_7          (1<<7)                      // Set GPIO pin 7
#define GPIOSET0_6          (1<<6)                      // Set GPIO pin 6
#define GPIOSET0_5          (1<<5)                      // Set GPIO pin 5
#define GPIOSET0_4          (1<<4)                      // Set GPIO pin 4
#define GPIOSET0_3          (1<<3)                      // Set GPIO pin 3
#define GPIOSET0_2          (1<<2)                      // Set GPIO pin 2
#define GPIOSET0_1          (1<<1)                      // Set GPIO pin 1
#define GPIOSET0_0          (1<<0)                      // Set GPIO pin 0


#define GPIO_SET1           (0x20)                      // GPIO Pin Output Set 1
//-------------------------------------------------------------------------------------------------------------------
#define GPIOSET1_53         (1<<21)                     // Set GPIO pin 53
#define GPIOSET1_52         (1<<20)                     // Set GPIO pin 52
#define GPIOSET1_51         (1<<19)                     // Set GPIO pin 51
#define GPIOSET1_50         (1<<18)                     // Set GPIO pin 50
#define GPIOSET1_49         (1<<17)                     // Set GPIO pin 49
#define GPIOSET1_48         (1<<16)                     // Set GPIO pin 48
#define GPIOSET1_47         (1<<15)                     // Set GPIO pin 47
#define GPIOSET1_46         (1<<14)                     // Set GPIO pin 46
#define GPIOSET1_45         (1<<13)                     // Set GPIO pin 45
#define GPIOSET1_44         (1<<12)                     // Set GPIO pin 44
#define GPIOSET1_43         (1<<11)                     // Set GPIO pin 43
#define GPIOSET1_42         (1<<10)                     // Set GPIO pin 42
#define GPIOSET1_41         (1<<9)                      // Set GPIO pin 41
#define GPIOSET1_40         (1<<8)                      // Set GPIO pin 40
#define GPIOSET1_39         (1<<7)                      // Set GPIO pin 39
#define GPIOSET1_38         (1<<6)                      // Set GPIO pin 38
#define GPIOSET1_37         (1<<5)                      // Set GPIO pin 37
#define GPIOSET1_36         (1<<4)                      // Set GPIO pin 36
#define GPIOSET1_35         (1<<3)                      // Set GPIO pin 35
#define GPIOSET1_34         (1<<2)                      // Set GPIO pin 34
#define GPIOSET1_33         (1<<1)                      // Set GPIO pin 33
#define GPIOSET1_32         (1<<0)                      // Set GPIO pin 32


#define GPIO_CLR0           (0x28)                      // GPIO Pin Output Clear 0
//-------------------------------------------------------------------------------------------------------------------
#define GPIOCLR0_31         (1<<31)                     // Clear GPIO pin 31
#define GPIOCLR0_30         (1<<30)                     // Clear GPIO pin 30
#define GPIOCLR0_29         (1<<29)                     // Clear GPIO pin 29
#define GPIOCLR0_28         (1<<28)                     // Clear GPIO pin 28
#define GPIOCLR0_27         (1<<27)                     // Clear GPIO pin 27
#define GPIOCLR0_26         (1<<26)                     // Clear GPIO pin 26
#define GPIOCLR0_25         (1<<25)                     // Clear GPIO pin 25
#define GPIOCLR0_24         (1<<24)                     // Clear GPIO pin 24
#define GPIOCLR0_23         (1<<23)                     // Clear GPIO pin 23
#define GPIOCLR0_22         (1<<22)                     // Clear GPIO pin 22
#define GPIOCLR0_21         (1<<21)                     // Clear GPIO pin 21
#define GPIOCLR0_20         (1<<20)                     // Clear GPIO pin 20
#define GPIOCLR0_19         (1<<19)                     // Clear GPIO pin 19
#define GPIOCLR0_18         (1<<18)                     // Clear GPIO pin 18
#define GPIOCLR0_17         (1<<17)                     // Clear GPIO pin 17
#define GPIOCLR0_16         (1<<16)                     // Clear GPIO pin 16
#define GPIOCLR0_15         (1<<15)                     // Clear GPIO pin 15
#define GPIOCLR0_14         (1<<14)                     // Clear GPIO pin 14
#define GPIOCLR0_13         (1<<13)                     // Clear GPIO pin 13
#define GPIOCLR0_12         (1<<12)                     // Clear GPIO pin 12
#define GPIOCLR0_11         (1<<11)                     // Clear GPIO pin 11
#define GPIOCLR0_10         (1<<10)                     // Clear GPIO pin 10
#define GPIOCLR0_9          (1<<9)                      // Clear GPIO pin 9
#define GPIOCLR0_8          (1<<8)                      // Clear GPIO pin 8
#define GPIOCLR0_7          (1<<7)                      // Clear GPIO pin 7
#define GPIOCLR0_6          (1<<6)                      // Clear GPIO pin 6
#define GPIOCLR0_5          (1<<5)                      // Clear GPIO pin 5
#define GPIOCLR0_4          (1<<4)                      // Clear GPIO pin 4
#define GPIOCLR0_3          (1<<3)                      // Clear GPIO pin 3
#define GPIOCLR0_2          (1<<2)                      // Clear GPIO pin 2
#define GPIOCLR0_1          (1<<1)                      // Clear GPIO pin 1
#define GPIOCLR0_0          (1<<0)                      // Clear GPIO pin 0


#define GPIO_CLR1           (0x2c)                      // GPIO Pin Output Clear 1
//-------------------------------------------------------------------------------------------------------------------
#define GPIOCLR1_53         (1<<21)                     // Clear GPIO pin 53
#define GPIOCLR1_52         (1<<20)                     // Clear GPIO pin 52
#define GPIOCLR1_51         (1<<19)                     // Clear GPIO pin 51
#define GPIOCLR1_50         (1<<18)                     // Clear GPIO pin 50
#define GPIOCLR1_49         (1<<17)                     // Clear GPIO pin 49
#define GPIOCLR1_48         (1<<16)                     // Clear GPIO pin 48
#define GPIOCLR1_47         (1<<15)                     // Clear GPIO pin 47
#define GPIOCLR1_46         (1<<14)                     // Clear GPIO pin 46
#define GPIOCLR1_45         (1<<13)                     // Clear GPIO pin 45
#define GPIOCLR1_44         (1<<12)                     // Clear GPIO pin 44
#define GPIOCLR1_43         (1<<11)                     // Clear GPIO pin 43
#define GPIOCLR1_42         (1<<10)                     // Clear GPIO pin 42
#define GPIOCLR1_41         (1<<9)                      // Clear GPIO pin 41
#define GPIOCLR1_40         (1<<8)                      // Clear GPIO pin 40
#define GPIOCLR1_39         (1<<7)                      // Clear GPIO pin 39
#define GPIOCLR1_38         (1<<6)                      // Clear GPIO pin 38
#define GPIOCLR1_37         (1<<5)                      // Clear GPIO pin 37
#define GPIOCLR1_36         (1<<4)                      // Clear GPIO pin 36
#define GPIOCLR1_35         (1<<3)                      // Clear GPIO pin 35
#define GPIOCLR1_34         (1<<2)                      // Clear GPIO pin 34
#define GPIOCLR1_33         (1<<1)                      // Clear GPIO pin 33
#define GPIOCLR1_32         (1<<0)                      // Clear GPIO pin 32


#define GPIO_LEV0           (0x34)                      // GPIO Pin Level 0
//-------------------------------------------------------------------------------------------------------------------
#define GPIOLVL0_31         (1<<31)                     // Level GPIO pin 31
#define GPIOLVL0_30         (1<<30)                     // Level GPIO pin 30
#define GPIOLVL0_29         (1<<29)                     // Level GPIO pin 29
#define GPIOLVL0_28         (1<<28)                     // Level GPIO pin 28
#define GPIOLVL0_27         (1<<27)                     // Level GPIO pin 27
#define GPIOLVL0_26         (1<<26)                     // Level GPIO pin 26
#define GPIOLVL0_25         (1<<25)                     // Level GPIO pin 25
#define GPIOLVL0_24         (1<<24)                     // Level GPIO pin 24
#define GPIOLVL0_23         (1<<23)                     // Level GPIO pin 23
#define GPIOLVL0_22         (1<<22)                     // Level GPIO pin 22
#define GPIOLVL0_21         (1<<21)                     // Level GPIO pin 21
#define GPIOLVL0_20         (1<<20)                     // Level GPIO pin 20
#define GPIOLVL0_19         (1<<19)                     // Level GPIO pin 19
#define GPIOLVL0_18         (1<<18)                     // Level GPIO pin 18
#define GPIOLVL0_17         (1<<17)                     // Level GPIO pin 17
#define GPIOLVL0_16         (1<<16)                     // Level GPIO pin 16
#define GPIOLVL0_15         (1<<15)                     // Level GPIO pin 15
#define GPIOLVL0_14         (1<<14)                     // Level GPIO pin 14
#define GPIOLVL0_13         (1<<13)                     // Level GPIO pin 13
#define GPIOLVL0_12         (1<<12)                     // Level GPIO pin 12
#define GPIOLVL0_11         (1<<11)                     // Level GPIO pin 11
#define GPIOLVL0_10         (1<<10)                     // Level GPIO pin 10
#define GPIOLVL0_9          (1<<9)                      // Level GPIO pin 9
#define GPIOLVL0_8          (1<<8)                      // Level GPIO pin 8
#define GPIOLVL0_7          (1<<7)                      // Level GPIO pin 7
#define GPIOLVL0_6          (1<<6)                      // Level GPIO pin 6
#define GPIOLVL0_5          (1<<5)                      // Level GPIO pin 5
#define GPIOLVL0_4          (1<<4)                      // Level GPIO pin 4
#define GPIOLVL0_3          (1<<3)                      // Level GPIO pin 3
#define GPIOLVL0_2          (1<<2)                      // Level GPIO pin 2
#define GPIOLVL0_1          (1<<1)                      // Level GPIO pin 1
#define GPIOLVL0_0          (1<<0)                      // Level GPIO pin 0


#define GPIO_LEV1           (0x38)                      // GPIO Pin Level 1
//-------------------------------------------------------------------------------------------------------------------
#define GPIOLVL1_53         (1<<21)                     // Level GPIO pin 53
#define GPIOLVL1_52         (1<<20)                     // Level GPIO pin 52
#define GPIOLVL1_51         (1<<19)                     // Level GPIO pin 51
#define GPIOLVL1_50         (1<<18)                     // Level GPIO pin 50
#define GPIOLVL1_49         (1<<17)                     // Level GPIO pin 49
#define GPIOLVL1_48         (1<<16)                     // Level GPIO pin 48
#define GPIOLVL1_47         (1<<15)                     // Level GPIO pin 47
#define GPIOLVL1_46         (1<<14)                     // Level GPIO pin 46
#define GPIOLVL1_45         (1<<13)                     // Level GPIO pin 45
#define GPIOLVL1_44         (1<<12)                     // Level GPIO pin 44
#define GPIOLVL1_43         (1<<11)                     // Level GPIO pin 43
#define GPIOLVL1_42         (1<<10)                     // Level GPIO pin 42
#define GPIOLVL1_41         (1<<9)                      // Level GPIO pin 41
#define GPIOLVL1_40         (1<<8)                      // Level GPIO pin 40
#define GPIOLVL1_39         (1<<7)                      // Level GPIO pin 39
#define GPIOLVL1_38         (1<<6)                      // Level GPIO pin 38
#define GPIOLVL1_37         (1<<5)                      // Level GPIO pin 37
#define GPIOLVL1_36         (1<<4)                      // Level GPIO pin 36
#define GPIOLVL1_35         (1<<3)                      // Level GPIO pin 35
#define GPIOLVL1_34         (1<<2)                      // Level GPIO pin 34
#define GPIOLVL1_33         (1<<1)                      // Level GPIO pin 33
#define GPIOLVL1_32         (1<<0)                      // Level GPIO pin 32


#define GPIO_EDS0           (0x40)                      // GPIO Pin Event Detect Status 0
//-------------------------------------------------------------------------------------------------------------------
#define GPIOEDS0_31         (1<<31)                     // Event Detected on GPIO pin 31
#define GPIOEDS0_30         (1<<30)                     // Event Detected on GPIO pin 30
#define GPIOEDS0_29         (1<<29)                     // Event Detected on GPIO pin 29
#define GPIOEDS0_28         (1<<28)                     // Event Detected on GPIO pin 28
#define GPIOEDS0_27         (1<<27)                     // Event Detected on GPIO pin 27
#define GPIOEDS0_26         (1<<26)                     // Event Detected on GPIO pin 26
#define GPIOEDS0_25         (1<<25)                     // Event Detected on GPIO pin 25
#define GPIOEDS0_24         (1<<24)                     // Event Detected on GPIO pin 24
#define GPIOEDS0_23         (1<<23)                     // Event Detected on GPIO pin 23
#define GPIOEDS0_22         (1<<22)                     // Event Detected on GPIO pin 22
#define GPIOEDS0_21         (1<<21)                     // Event Detected on GPIO pin 21
#define GPIOEDS0_20         (1<<20)                     // Event Detected on GPIO pin 20
#define GPIOEDS0_19         (1<<19)                     // Event Detected on GPIO pin 19
#define GPIOEDS0_18         (1<<18)                     // Event Detected on GPIO pin 18
#define GPIOEDS0_17         (1<<17)                     // Event Detected on GPIO pin 17
#define GPIOEDS0_16         (1<<16)                     // Event Detected on GPIO pin 16
#define GPIOEDS0_15         (1<<15)                     // Event Detected on GPIO pin 15
#define GPIOEDS0_14         (1<<14)                     // Event Detected on GPIO pin 14
#define GPIOEDS0_13         (1<<13)                     // Event Detected on GPIO pin 13
#define GPIOEDS0_12         (1<<12)                     // Event Detected on GPIO pin 12
#define GPIOEDS0_11         (1<<11)                     // Event Detected on GPIO pin 11
#define GPIOEDS0_10         (1<<10)                     // Event Detected on GPIO pin 10
#define GPIOEDS0_9          (1<<9)                      // Event Detected on GPIO pin 9
#define GPIOEDS0_8          (1<<8)                      // Event Detected on GPIO pin 8
#define GPIOEDS0_7          (1<<7)                      // Event Detected on GPIO pin 7
#define GPIOEDS0_6          (1<<6)                      // Event Detected on GPIO pin 6
#define GPIOEDS0_5          (1<<5)                      // Event Detected on GPIO pin 5
#define GPIOEDS0_4          (1<<4)                      // Event Detected on GPIO pin 4
#define GPIOEDS0_3          (1<<3)                      // Event Detected on GPIO pin 3
#define GPIOEDS0_2          (1<<2)                      // Event Detected on GPIO pin 2
#define GPIOEDS0_1          (1<<1)                      // Event Detected on GPIO pin 1
#define GPIOEDS0_0          (1<<0)                      // Event Detected on GPIO pin 0


#define GPIO_EDS1           (0x44)                      // GPIO Pin Event Detect Status 1
//-------------------------------------------------------------------------------------------------------------------
#define GPIOEDS1_53         (1<<21)                     // Event Detected on GPIO pin 53
#define GPIOEDS1_52         (1<<20)                     // Event Detected on GPIO pin 52
#define GPIOEDS1_51         (1<<19)                     // Event Detected on GPIO pin 51
#define GPIOEDS1_50         (1<<18)                     // Event Detected on GPIO pin 50
#define GPIOEDS1_49         (1<<17)                     // Event Detected on GPIO pin 49
#define GPIOEDS1_48         (1<<16)                     // Event Detected on GPIO pin 48
#define GPIOEDS1_47         (1<<15)                     // Event Detected on GPIO pin 47
#define GPIOEDS1_46         (1<<14)                     // Event Detected on GPIO pin 46
#define GPIOEDS1_45         (1<<13)                     // Event Detected on GPIO pin 45
#define GPIOEDS1_44         (1<<12)                     // Event Detected on GPIO pin 44
#define GPIOEDS1_43         (1<<11)                     // Event Detected on GPIO pin 43
#define GPIOEDS1_42         (1<<10)                     // Event Detected on GPIO pin 42
#define GPIOEDS1_41         (1<<9)                      // Event Detected on GPIO pin 41
#define GPIOEDS1_40         (1<<8)                      // Event Detected on GPIO pin 40
#define GPIOEDS1_39         (1<<7)                      // Event Detected on GPIO pin 39
#define GPIOEDS1_38         (1<<6)                      // Event Detected on GPIO pin 38
#define GPIOEDS1_37         (1<<5)                      // Event Detected on GPIO pin 37
#define GPIOEDS1_36         (1<<4)                      // Event Detected on GPIO pin 36
#define GPIOEDS1_35         (1<<3)                      // Event Detected on GPIO pin 35
#define GPIOEDS1_34         (1<<2)                      // Event Detected on GPIO pin 34
#define GPIOEDS1_33         (1<<1)                      // Event Detected on GPIO pin 33
#define GPIOEDS1_32         (1<<0)                      // Event Detected on GPIO pin 32


#define GPIO_REN0           (0x4c)                      // GPIO Pin Rising Edge Detect Status 0
//-------------------------------------------------------------------------------------------------------------------
#define GPIOREN0_31         (1<<31)                     // Set Rising Edge Detect on GPIO pin 31
#define GPIOREN0_30         (1<<30)                     // Set Rising Edge Detect on GPIO pin 30
#define GPIOREN0_29         (1<<29)                     // Set Rising Edge Detect on GPIO pin 29
#define GPIOREN0_28         (1<<28)                     // Set Rising Edge Detect on GPIO pin 28
#define GPIOREN0_27         (1<<27)                     // Set Rising Edge Detect on GPIO pin 27
#define GPIOREN0_26         (1<<26)                     // Set Rising Edge Detect on GPIO pin 26
#define GPIOREN0_25         (1<<25)                     // Set Rising Edge Detect on GPIO pin 25
#define GPIOREN0_24         (1<<24)                     // Set Rising Edge Detect on GPIO pin 24
#define GPIOREN0_23         (1<<23)                     // Set Rising Edge Detect on GPIO pin 23
#define GPIOREN0_22         (1<<22)                     // Set Rising Edge Detect on GPIO pin 22
#define GPIOREN0_21         (1<<21)                     // Set Rising Edge Detect on GPIO pin 21
#define GPIOREN0_20         (1<<20)                     // Set Rising Edge Detect on GPIO pin 20
#define GPIOREN0_19         (1<<19)                     // Set Rising Edge Detect on GPIO pin 19
#define GPIOREN0_18         (1<<18)                     // Set Rising Edge Detect on GPIO pin 18
#define GPIOREN0_17         (1<<17)                     // Set Rising Edge Detect on GPIO pin 17
#define GPIOREN0_16         (1<<16)                     // Set Rising Edge Detect on GPIO pin 16
#define GPIOREN0_15         (1<<15)                     // Set Rising Edge Detect on GPIO pin 15
#define GPIOREN0_14         (1<<14)                     // Set Rising Edge Detect on GPIO pin 14
#define GPIOREN0_13         (1<<13)                     // Set Rising Edge Detect on GPIO pin 13
#define GPIOREN0_12         (1<<12)                     // Set Rising Edge Detect on GPIO pin 12
#define GPIOREN0_11         (1<<11)                     // Set Rising Edge Detect on GPIO pin 11
#define GPIOREN0_10         (1<<10)                     // Set Rising Edge Detect on GPIO pin 10
#define GPIOREN0_9          (1<<9)                      // Set Rising Edge Detect on GPIO pin 9
#define GPIOREN0_8          (1<<8)                      // Set Rising Edge Detect on GPIO pin 8
#define GPIOREN0_7          (1<<7)                      // Set Rising Edge Detect on GPIO pin 7
#define GPIOREN0_6          (1<<6)                      // Set Rising Edge Detect on GPIO pin 6
#define GPIOREN0_5          (1<<5)                      // Set Rising Edge Detect on GPIO pin 5
#define GPIOREN0_4          (1<<4)                      // Set Rising Edge Detect on GPIO pin 4
#define GPIOREN0_3          (1<<3)                      // Set Rising Edge Detect on GPIO pin 3
#define GPIOREN0_2          (1<<2)                      // Set Rising Edge Detect on GPIO pin 2
#define GPIOREN0_1          (1<<1)                      // Set Rising Edge Detect on GPIO pin 1
#define GPIOREN0_0          (1<<0)                      // Set Rising Edge Detect on GPIO pin 0


#define GPIO_REN1           (0x50)                      // GPIO Pin Rising Edge Detect Status 1
//-------------------------------------------------------------------------------------------------------------------
#define GPIOREN1_53         (1<<21)                     // Set Rising Edge Detect on GPIO pin 53
#define GPIOREN1_52         (1<<20)                     // Set Rising Edge Detect on GPIO pin 52
#define GPIOREN1_51         (1<<19)                     // Set Rising Edge Detect on GPIO pin 51
#define GPIOREN1_50         (1<<18)                     // Set Rising Edge Detect on GPIO pin 50
#define GPIOREN1_49         (1<<17)                     // Set Rising Edge Detect on GPIO pin 49
#define GPIOREN1_48         (1<<16)                     // Set Rising Edge Detect on GPIO pin 48
#define GPIOREN1_47         (1<<15)                     // Set Rising Edge Detect on GPIO pin 47
#define GPIOREN1_46         (1<<14)                     // Set Rising Edge Detect on GPIO pin 46
#define GPIOREN1_45         (1<<13)                     // Set Rising Edge Detect on GPIO pin 45
#define GPIOREN1_44         (1<<12)                     // Set Rising Edge Detect on GPIO pin 44
#define GPIOREN1_43         (1<<11)                     // Set Rising Edge Detect on GPIO pin 43
#define GPIOREN1_42         (1<<10)                     // Set Rising Edge Detect on GPIO pin 42
#define GPIOREN1_41         (1<<9)                      // Set Rising Edge Detect on GPIO pin 41
#define GPIOREN1_40         (1<<8)                      // Set Rising Edge Detect on GPIO pin 40
#define GPIOREN1_39         (1<<7)                      // Set Rising Edge Detect on GPIO pin 39
#define GPIOREN1_38         (1<<6)                      // Set Rising Edge Detect on GPIO pin 38
#define GPIOREN1_37         (1<<5)                      // Set Rising Edge Detect on GPIO pin 37
#define GPIOREN1_36         (1<<4)                      // Set Rising Edge Detect on GPIO pin 36
#define GPIOREN1_35         (1<<3)                      // Set Rising Edge Detect on GPIO pin 35
#define GPIOREN1_34         (1<<2)                      // Set Rising Edge Detect on GPIO pin 34
#define GPIOREN1_33         (1<<1)                      // Set Rising Edge Detect on GPIO pin 33
#define GPIOREN1_32         (1<<0)                      // Set Rising Edge Detect on GPIO pin 32


#define GPIO_FEN0           (0x58)                      // GPIO Pin Falling Edge Detect Status 0
//-------------------------------------------------------------------------------------------------------------------
#define GPIOFEN0_31         (1<<31)                     // Set Falling Edge Detect on GPIO pin 31
#define GPIOFEN0_30         (1<<30)                     // Set Falling Edge Detect on GPIO pin 30
#define GPIOFEN0_29         (1<<29)                     // Set Falling Edge Detect on GPIO pin 29
#define GPIOFEN0_28         (1<<28)                     // Set Falling Edge Detect on GPIO pin 28
#define GPIOFEN0_27         (1<<27)                     // Set Falling Edge Detect on GPIO pin 27
#define GPIOFEN0_26         (1<<26)                     // Set Falling Edge Detect on GPIO pin 26
#define GPIOFEN0_25         (1<<25)                     // Set Falling Edge Detect on GPIO pin 25
#define GPIOFEN0_24         (1<<24)                     // Set Falling Edge Detect on GPIO pin 24
#define GPIOFEN0_23         (1<<23)                     // Set Falling Edge Detect on GPIO pin 23
#define GPIOFEN0_22         (1<<22)                     // Set Falling Edge Detect on GPIO pin 22
#define GPIOFEN0_21         (1<<21)                     // Set Falling Edge Detect on GPIO pin 21
#define GPIOFEN0_20         (1<<20)                     // Set Falling Edge Detect on GPIO pin 20
#define GPIOFEN0_19         (1<<19)                     // Set Falling Edge Detect on GPIO pin 19
#define GPIOFEN0_18         (1<<18)                     // Set Falling Edge Detect on GPIO pin 18
#define GPIOFEN0_17         (1<<17)                     // Set Falling Edge Detect on GPIO pin 17
#define GPIOFEN0_16         (1<<16)                     // Set Falling Edge Detect on GPIO pin 16
#define GPIOFEN0_15         (1<<15)                     // Set Falling Edge Detect on GPIO pin 15
#define GPIOFEN0_14         (1<<14)                     // Set Falling Edge Detect on GPIO pin 14
#define GPIOFEN0_13         (1<<13)                     // Set Falling Edge Detect on GPIO pin 13
#define GPIOFEN0_12         (1<<12)                     // Set Falling Edge Detect on GPIO pin 12
#define GPIOFEN0_11         (1<<11)                     // Set Falling Edge Detect on GPIO pin 11
#define GPIOFEN0_10         (1<<10)                     // Set Falling Edge Detect on GPIO pin 10
#define GPIOFEN0_9          (1<<9)                      // Set Falling Edge Detect on GPIO pin 9
#define GPIOFEN0_8          (1<<8)                      // Set Falling Edge Detect on GPIO pin 8
#define GPIOFEN0_7          (1<<7)                      // Set Falling Edge Detect on GPIO pin 7
#define GPIOFEN0_6          (1<<6)                      // Set Falling Edge Detect on GPIO pin 6
#define GPIOFEN0_5          (1<<5)                      // Set Falling Edge Detect on GPIO pin 5
#define GPIOFEN0_4          (1<<4)                      // Set Falling Edge Detect on GPIO pin 4
#define GPIOFEN0_3          (1<<3)                      // Set Falling Edge Detect on GPIO pin 3
#define GPIOFEN0_2          (1<<2)                      // Set Falling Edge Detect on GPIO pin 2
#define GPIOFEN0_1          (1<<1)                      // Set Falling Edge Detect on GPIO pin 1
#define GPIOFEN0_0          (1<<0)                      // Set Falling Edge Detect on GPIO pin 0


#define GPIO_FEN1           (0x5c)                      // GPIO Pin Falling Edge Detect Status 1
//-------------------------------------------------------------------------------------------------------------------
#define GPIOFEN1_53         (1<<21)                     // Set Falling Edge Detect on GPIO pin 53
#define GPIOFEN1_52         (1<<20)                     // Set Falling Edge Detect on GPIO pin 52
#define GPIOFEN1_51         (1<<19)                     // Set Falling Edge Detect on GPIO pin 51
#define GPIOFEN1_50         (1<<18)                     // Set Falling Edge Detect on GPIO pin 50
#define GPIOFEN1_49         (1<<17)                     // Set Falling Edge Detect on GPIO pin 49
#define GPIOFEN1_48         (1<<16)                     // Set Falling Edge Detect on GPIO pin 48
#define GPIOFEN1_47         (1<<15)                     // Set Falling Edge Detect on GPIO pin 47
#define GPIOFEN1_46         (1<<14)                     // Set Falling Edge Detect on GPIO pin 46
#define GPIOFEN1_45         (1<<13)                     // Set Falling Edge Detect on GPIO pin 45
#define GPIOFEN1_44         (1<<12)                     // Set Falling Edge Detect on GPIO pin 44
#define GPIOFEN1_43         (1<<11)                     // Set Falling Edge Detect on GPIO pin 43
#define GPIOFEN1_42         (1<<10)                     // Set Falling Edge Detect on GPIO pin 42
#define GPIOFEN1_41         (1<<9)                      // Set Falling Edge Detect on GPIO pin 41
#define GPIOFEN1_40         (1<<8)                      // Set Falling Edge Detect on GPIO pin 40
#define GPIOFEN1_39         (1<<7)                      // Set Falling Edge Detect on GPIO pin 39
#define GPIOFEN1_38         (1<<6)                      // Set Falling Edge Detect on GPIO pin 38
#define GPIOFEN1_37         (1<<5)                      // Set Falling Edge Detect on GPIO pin 37
#define GPIOFEN1_36         (1<<4)                      // Set Falling Edge Detect on GPIO pin 36
#define GPIOFEN1_35         (1<<3)                      // Set Falling Edge Detect on GPIO pin 35
#define GPIOFEN1_34         (1<<2)                      // Set Falling Edge Detect on GPIO pin 34
#define GPIOFEN1_33         (1<<1)                      // Set Falling Edge Detect on GPIO pin 33
#define GPIOFEN1_32         (1<<0)                      // Set Falling Edge Detect on GPIO pin 32


#define GPIO_HEN0           (0x64)                      // GPIO Pin High Detect Status 0
//-------------------------------------------------------------------------------------------------------------------
#define GPIOHEN0_31         (1<<31)                     // Set High Detect on GPIO pin 31
#define GPIOHEN0_30         (1<<30)                     // Set High Detect on GPIO pin 30
#define GPIOHEN0_29         (1<<29)                     // Set High Detect on GPIO pin 29
#define GPIOHEN0_28         (1<<28)                     // Set High Detect on GPIO pin 28
#define GPIOHEN0_27         (1<<27)                     // Set High Detect on GPIO pin 27
#define GPIOHEN0_26         (1<<26)                     // Set High Detect on GPIO pin 26
#define GPIOHEN0_25         (1<<25)                     // Set High Detect on GPIO pin 25
#define GPIOHEN0_24         (1<<24)                     // Set High Detect on GPIO pin 24
#define GPIOHEN0_23         (1<<23)                     // Set High Detect on GPIO pin 23
#define GPIOHEN0_22         (1<<22)                     // Set High Detect on GPIO pin 22
#define GPIOHEN0_21         (1<<21)                     // Set High Detect on GPIO pin 21
#define GPIOHEN0_20         (1<<20)                     // Set High Detect on GPIO pin 20
#define GPIOHEN0_19         (1<<19)                     // Set High Detect on GPIO pin 19
#define GPIOHEN0_18         (1<<18)                     // Set High Detect on GPIO pin 18
#define GPIOHEN0_17         (1<<17)                     // Set High Detect on GPIO pin 17
#define GPIOHEN0_16         (1<<16)                     // Set High Detect on GPIO pin 16
#define GPIOHEN0_15         (1<<15)                     // Set High Detect on GPIO pin 15
#define GPIOHEN0_14         (1<<14)                     // Set High Detect on GPIO pin 14
#define GPIOHEN0_13         (1<<13)                     // Set High Detect on GPIO pin 13
#define GPIOHEN0_12         (1<<12)                     // Set High Detect on GPIO pin 12
#define GPIOHEN0_11         (1<<11)                     // Set High Detect on GPIO pin 11
#define GPIOHEN0_10         (1<<10)                     // Set High Detect on GPIO pin 10
#define GPIOHEN0_9          (1<<9)                      // Set High Detect on GPIO pin 9
#define GPIOHEN0_8          (1<<8)                      // Set High Detect on GPIO pin 8
#define GPIOHEN0_7          (1<<7)                      // Set High Detect on GPIO pin 7
#define GPIOHEN0_6          (1<<6)                      // Set High Detect on GPIO pin 6
#define GPIOHEN0_5          (1<<5)                      // Set High Detect on GPIO pin 5
#define GPIOHEN0_4          (1<<4)                      // Set High Detect on GPIO pin 4
#define GPIOHEN0_3          (1<<3)                      // Set High Detect on GPIO pin 3
#define GPIOHEN0_2          (1<<2)                      // Set High Detect on GPIO pin 2
#define GPIOHEN0_1          (1<<1)                      // Set High Detect on GPIO pin 1
#define GPIOHEN0_0          (1<<0)                      // Set High Detect on GPIO pin 0


#define GPIO_HEN1           (0x68)                      // GPIO Pin High Detect Status 1
//-------------------------------------------------------------------------------------------------------------------
#define GPIOHEN1_53         (1<<21)                     // Set High Detect on GPIO pin 53
#define GPIOHEN1_52         (1<<20)                     // Set High Detect on GPIO pin 52
#define GPIOHEN1_51         (1<<19)                     // Set High Detect on GPIO pin 51
#define GPIOHEN1_50         (1<<18)                     // Set High Detect on GPIO pin 50
#define GPIOHEN1_49         (1<<17)                     // Set High Detect on GPIO pin 49
#define GPIOHEN1_48         (1<<16)                     // Set High Detect on GPIO pin 48
#define GPIOHEN1_47         (1<<15)                     // Set High Detect on GPIO pin 47
#define GPIOHEN1_46         (1<<14)                     // Set High Detect on GPIO pin 46
#define GPIOHEN1_45         (1<<13)                     // Set High Detect on GPIO pin 45
#define GPIOHEN1_44         (1<<12)                     // Set High Detect on GPIO pin 44
#define GPIOHEN1_43         (1<<11)                     // Set High Detect on GPIO pin 43
#define GPIOHEN1_42         (1<<10)                     // Set High Detect on GPIO pin 42
#define GPIOHEN1_41         (1<<9)                      // Set High Detect on GPIO pin 41
#define GPIOHEN1_40         (1<<8)                      // Set High Detect on GPIO pin 40
#define GPIOHEN1_39         (1<<7)                      // Set High Detect on GPIO pin 39
#define GPIOHEN1_38         (1<<6)                      // Set High Detect on GPIO pin 38
#define GPIOHEN1_37         (1<<5)                      // Set High Detect on GPIO pin 37
#define GPIOHEN1_36         (1<<4)                      // Set High Detect on GPIO pin 36
#define GPIOHEN1_35         (1<<3)                      // Set High Detect on GPIO pin 35
#define GPIOHEN1_34         (1<<2)                      // Set High Detect on GPIO pin 34
#define GPIOHEN1_33         (1<<1)                      // Set High Detect on GPIO pin 33
#define GPIOHEN1_32         (1<<0)                      // Set High Detect on GPIO pin 32


#define GPIO_LEN0           (0x70)                      // GPIO Pin Low Detect Status 0
//-------------------------------------------------------------------------------------------------------------------
#define GPIOLEN0_31         (1<<31)                     // Set Low Detect on GPIO pin 31
#define GPIOLEN0_30         (1<<30)                     // Set Low Detect on GPIO pin 30
#define GPIOLEN0_29         (1<<29)                     // Set Low Detect on GPIO pin 29
#define GPIOLEN0_28         (1<<28)                     // Set Low Detect on GPIO pin 28
#define GPIOLEN0_27         (1<<27)                     // Set Low Detect on GPIO pin 27
#define GPIOLEN0_26         (1<<26)                     // Set Low Detect on GPIO pin 26
#define GPIOLEN0_25         (1<<25)                     // Set Low Detect on GPIO pin 25
#define GPIOLEN0_24         (1<<24)                     // Set Low Detect on GPIO pin 24
#define GPIOLEN0_23         (1<<23)                     // Set Low Detect on GPIO pin 23
#define GPIOLEN0_22         (1<<22)                     // Set Low Detect on GPIO pin 22
#define GPIOLEN0_21         (1<<21)                     // Set Low Detect on GPIO pin 21
#define GPIOLEN0_20         (1<<20)                     // Set Low Detect on GPIO pin 20
#define GPIOLEN0_19         (1<<19)                     // Set Low Detect on GPIO pin 19
#define GPIOLEN0_18         (1<<18)                     // Set Low Detect on GPIO pin 18
#define GPIOLEN0_17         (1<<17)                     // Set Low Detect on GPIO pin 17
#define GPIOLEN0_16         (1<<16)                     // Set Low Detect on GPIO pin 16
#define GPIOLEN0_15         (1<<15)                     // Set Low Detect on GPIO pin 15
#define GPIOLEN0_14         (1<<14)                     // Set Low Detect on GPIO pin 14
#define GPIOLEN0_13         (1<<13)                     // Set Low Detect on GPIO pin 13
#define GPIOLEN0_12         (1<<12)                     // Set Low Detect on GPIO pin 12
#define GPIOLEN0_11         (1<<11)                     // Set Low Detect on GPIO pin 11
#define GPIOLEN0_10         (1<<10)                     // Set Low Detect on GPIO pin 10
#define GPIOLEN0_9          (1<<9)                      // Set Low Detect on GPIO pin 9
#define GPIOLEN0_8          (1<<8)                      // Set Low Detect on GPIO pin 8
#define GPIOLEN0_7          (1<<7)                      // Set Low Detect on GPIO pin 7
#define GPIOLEN0_6          (1<<6)                      // Set Low Detect on GPIO pin 6
#define GPIOLEN0_5          (1<<5)                      // Set Low Detect on GPIO pin 5
#define GPIOLEN0_4          (1<<4)                      // Set Low Detect on GPIO pin 4
#define GPIOLEN0_3          (1<<3)                      // Set Low Detect on GPIO pin 3
#define GPIOLEN0_2          (1<<2)                      // Set Low Detect on GPIO pin 2
#define GPIOLEN0_1          (1<<1)                      // Set Low Detect on GPIO pin 1
#define GPIOLEN0_0          (1<<0)                      // Set Low Detect on GPIO pin 0


#define GPIO_LEN1           (0x74)                      // GPIO Pin Low Detect Status 1
//-------------------------------------------------------------------------------------------------------------------
#define GPIOLEN1_53         (1<<21)                     // Set Low Detect on GPIO pin 53
#define GPIOLEN1_52         (1<<20)                     // Set Low Detect on GPIO pin 52
#define GPIOLEN1_51         (1<<19)                     // Set Low Detect on GPIO pin 51
#define GPIOLEN1_50         (1<<18)                     // Set Low Detect on GPIO pin 50
#define GPIOLEN1_49         (1<<17)                     // Set Low Detect on GPIO pin 49
#define GPIOLEN1_48         (1<<16)                     // Set Low Detect on GPIO pin 48
#define GPIOLEN1_47         (1<<15)                     // Set Low Detect on GPIO pin 47
#define GPIOLEN1_46         (1<<14)                     // Set Low Detect on GPIO pin 46
#define GPIOLEN1_45         (1<<13)                     // Set Low Detect on GPIO pin 45
#define GPIOLEN1_44         (1<<12)                     // Set Low Detect on GPIO pin 44
#define GPIOLEN1_43         (1<<11)                     // Set Low Detect on GPIO pin 43
#define GPIOLEN1_42         (1<<10)                     // Set Low Detect on GPIO pin 42
#define GPIOLEN1_41         (1<<9)                      // Set Low Detect on GPIO pin 41
#define GPIOLEN1_40         (1<<8)                      // Set Low Detect on GPIO pin 40
#define GPIOLEN1_39         (1<<7)                      // Set Low Detect on GPIO pin 39
#define GPIOLEN1_38         (1<<6)                      // Set Low Detect on GPIO pin 38
#define GPIOLEN1_37         (1<<5)                      // Set Low Detect on GPIO pin 37
#define GPIOLEN1_36         (1<<4)                      // Set Low Detect on GPIO pin 36
#define GPIOLEN1_35         (1<<3)                      // Set Low Detect on GPIO pin 35
#define GPIOLEN1_34         (1<<2)                      // Set Low Detect on GPIO pin 34
#define GPIOLEN1_33         (1<<1)                      // Set Low Detect on GPIO pin 33
#define GPIOLEN1_32         (1<<0)                      // Set Low Detect on GPIO pin 32


#define GPIO_AREN0          (0x7c)                      // GPIO Pin Async Rising Edge Detect Status 0
//-------------------------------------------------------------------------------------------------------------------
#define GPIOAREN0_31        (1<<31)                     // Set Async Rising Edge Detect on GPIO pin 31
#define GPIOAREN0_30        (1<<30)                     // Set Async Rising Edge Detect on GPIO pin 30
#define GPIOAREN0_29        (1<<29)                     // Set Async Rising Edge Detect on GPIO pin 29
#define GPIOAREN0_28        (1<<28)                     // Set Async Rising Edge Detect on GPIO pin 28
#define GPIOAREN0_27        (1<<27)                     // Set Async Rising Edge Detect on GPIO pin 27
#define GPIOAREN0_26        (1<<26)                     // Set Async Rising Edge Detect on GPIO pin 26
#define GPIOAREN0_25        (1<<25)                     // Set Async Rising Edge Detect on GPIO pin 25
#define GPIOAREN0_24        (1<<24)                     // Set Async Rising Edge Detect on GPIO pin 24
#define GPIOAREN0_23        (1<<23)                     // Set Async Rising Edge Detect on GPIO pin 23
#define GPIOAREN0_22        (1<<22)                     // Set Async Rising Edge Detect on GPIO pin 22
#define GPIOAREN0_21        (1<<21)                     // Set Async Rising Edge Detect on GPIO pin 21
#define GPIOAREN0_20        (1<<20)                     // Set Async Rising Edge Detect on GPIO pin 20
#define GPIOAREN0_19        (1<<19)                     // Set Async Rising Edge Detect on GPIO pin 19
#define GPIOAREN0_18        (1<<18)                     // Set Async Rising Edge Detect on GPIO pin 18
#define GPIOAREN0_17        (1<<17)                     // Set Async Rising Edge Detect on GPIO pin 17
#define GPIOAREN0_16        (1<<16)                     // Set Async Rising Edge Detect on GPIO pin 16
#define GPIOAREN0_15        (1<<15)                     // Set Async Rising Edge Detect on GPIO pin 15
#define GPIOAREN0_14        (1<<14)                     // Set Async Rising Edge Detect on GPIO pin 14
#define GPIOAREN0_13        (1<<13)                     // Set Async Rising Edge Detect on GPIO pin 13
#define GPIOAREN0_12        (1<<12)                     // Set Async Rising Edge Detect on GPIO pin 12
#define GPIOAREN0_11        (1<<11)                     // Set Async Rising Edge Detect on GPIO pin 11
#define GPIOAREN0_10        (1<<10)                     // Set Async Rising Edge Detect on GPIO pin 10
#define GPIOAREN0_9         (1<<9)                      // Set Async Rising Edge Detect on GPIO pin 9
#define GPIOAREN0_8         (1<<8)                      // Set Async Rising Edge Detect on GPIO pin 8
#define GPIOAREN0_7         (1<<7)                      // Set Async Rising Edge Detect on GPIO pin 7
#define GPIOAREN0_6         (1<<6)                      // Set Async Rising Edge Detect on GPIO pin 6
#define GPIOAREN0_5         (1<<5)                      // Set Async Rising Edge Detect on GPIO pin 5
#define GPIOAREN0_4         (1<<4)                      // Set Async Rising Edge Detect on GPIO pin 4
#define GPIOAREN0_3         (1<<3)                      // Set Async Rising Edge Detect on GPIO pin 3
#define GPIOAREN0_2         (1<<2)                      // Set Async Rising Edge Detect on GPIO pin 2
#define GPIOAREN0_1         (1<<1)                      // Set Async Rising Edge Detect on GPIO pin 1
#define GPIOAREN0_0         (1<<0)                      // Set Async Rising Edge Detect on GPIO pin 0


#define GPIO_AREN1          (0x80)                      // GPIO Pin Async Rising Edge Detect Status 1
//-------------------------------------------------------------------------------------------------------------------
#define GPIOAREN1_53        (1<<21)                     // Set Async Rising Edge Detect on GPIO pin 53
#define GPIOAREN1_52        (1<<20)                     // Set Async Rising Edge Detect on GPIO pin 52
#define GPIOAREN1_51        (1<<19)                     // Set Async Rising Edge Detect on GPIO pin 51
#define GPIOAREN1_50        (1<<18)                     // Set Async Rising Edge Detect on GPIO pin 50
#define GPIOAREN1_49        (1<<17)                     // Set Async Rising Edge Detect on GPIO pin 49
#define GPIOAREN1_48        (1<<16)                     // Set Async Rising Edge Detect on GPIO pin 48
#define GPIOAREN1_47        (1<<15)                     // Set Async Rising Edge Detect on GPIO pin 47
#define GPIOAREN1_46        (1<<14)                     // Set Async Rising Edge Detect on GPIO pin 46
#define GPIOAREN1_45        (1<<13)                     // Set Async Rising Edge Detect on GPIO pin 45
#define GPIOAREN1_44        (1<<12)                     // Set Async Rising Edge Detect on GPIO pin 44
#define GPIOAREN1_43        (1<<11)                     // Set Async Rising Edge Detect on GPIO pin 43
#define GPIOAREN1_42        (1<<10)                     // Set Async Rising Edge Detect on GPIO pin 42
#define GPIOAREN1_41        (1<<9)                      // Set Async Rising Edge Detect on GPIO pin 41
#define GPIOAREN1_40        (1<<8)                      // Set Async Rising Edge Detect on GPIO pin 40
#define GPIOAREN1_39        (1<<7)                      // Set Async Rising Edge Detect on GPIO pin 39
#define GPIOAREN1_38        (1<<6)                      // Set Async Rising Edge Detect on GPIO pin 38
#define GPIOAREN1_37        (1<<5)                      // Set Async Rising Edge Detect on GPIO pin 37
#define GPIOAREN1_36        (1<<4)                      // Set Async Rising Edge Detect on GPIO pin 36
#define GPIOAREN1_35        (1<<3)                      // Set Async Rising Edge Detect on GPIO pin 35
#define GPIOAREN1_34        (1<<2)                      // Set Async Rising Edge Detect on GPIO pin 34
#define GPIOAREN1_33        (1<<1)                      // Set Async Rising Edge Detect on GPIO pin 33
#define GPIOAREN1_32        (1<<0)                      // Set Async Rising Edge Detect on GPIO pin 32


#define GPIO_AFEN0          (0x88)                      // GPIO Pin Async Falling Edge Detect Status 0
//-------------------------------------------------------------------------------------------------------------------
#define GPIOAFEN0_31        (1<<31)                     // Set Async Falling Edge Detect on GPIO pin 31
#define GPIOAFEN0_30        (1<<30)                     // Set Async Falling Edge Detect on GPIO pin 30
#define GPIOAFEN0_29        (1<<29)                     // Set Async Falling Edge Detect on GPIO pin 29
#define GPIOAFEN0_28        (1<<28)                     // Set Async Falling Edge Detect on GPIO pin 28
#define GPIOAFEN0_27        (1<<27)                     // Set Async Falling Edge Detect on GPIO pin 27
#define GPIOAFEN0_26        (1<<26)                     // Set Async Falling Edge Detect on GPIO pin 26
#define GPIOAFEN0_25        (1<<25)                     // Set Async Falling Edge Detect on GPIO pin 25
#define GPIOAFEN0_24        (1<<24)                     // Set Async Falling Edge Detect on GPIO pin 24
#define GPIOAFEN0_23        (1<<23)                     // Set Async Falling Edge Detect on GPIO pin 23
#define GPIOAFEN0_22        (1<<22)                     // Set Async Falling Edge Detect on GPIO pin 22
#define GPIOAFEN0_21        (1<<21)                     // Set Async Falling Edge Detect on GPIO pin 21
#define GPIOAFEN0_20        (1<<20)                     // Set Async Falling Edge Detect on GPIO pin 20
#define GPIOAFEN0_19        (1<<19)                     // Set Async Falling Edge Detect on GPIO pin 19
#define GPIOAFEN0_18        (1<<18)                     // Set Async Falling Edge Detect on GPIO pin 18
#define GPIOAFEN0_17        (1<<17)                     // Set Async Falling Edge Detect on GPIO pin 17
#define GPIOAFEN0_16        (1<<16)                     // Set Async Falling Edge Detect on GPIO pin 16
#define GPIOAFEN0_15        (1<<15)                     // Set Async Falling Edge Detect on GPIO pin 15
#define GPIOAFEN0_14        (1<<14)                     // Set Async Falling Edge Detect on GPIO pin 14
#define GPIOAFEN0_13        (1<<13)                     // Set Async Falling Edge Detect on GPIO pin 13
#define GPIOAFEN0_12        (1<<12)                     // Set Async Falling Edge Detect on GPIO pin 12
#define GPIOAFEN0_11        (1<<11)                     // Set Async Falling Edge Detect on GPIO pin 11
#define GPIOAFEN0_10        (1<<10)                     // Set Async Falling Edge Detect on GPIO pin 10
#define GPIOAFEN0_9         (1<<9)                      // Set Async Falling Edge Detect on GPIO pin 9
#define GPIOAFEN0_8         (1<<8)                      // Set Async Falling Edge Detect on GPIO pin 8
#define GPIOAFEN0_7         (1<<7)                      // Set Async Falling Edge Detect on GPIO pin 7
#define GPIOAFEN0_6         (1<<6)                      // Set Async Falling Edge Detect on GPIO pin 6
#define GPIOAFEN0_5         (1<<5)                      // Set Async Falling Edge Detect on GPIO pin 5
#define GPIOAFEN0_4         (1<<4)                      // Set Async Falling Edge Detect on GPIO pin 4
#define GPIOAFEN0_3         (1<<3)                      // Set Async Falling Edge Detect on GPIO pin 3
#define GPIOAFEN0_2         (1<<2)                      // Set Async Falling Edge Detect on GPIO pin 2
#define GPIOAFEN0_1         (1<<1)                      // Set Async Falling Edge Detect on GPIO pin 1
#define GPIOAFEN0_0         (1<<0)                      // Set Async Falling Edge Detect on GPIO pin 0


#define GPIO_AFEN1          (0x8c)                      // GPIO Pin Async Falling Edge Detect Status 1
//-------------------------------------------------------------------------------------------------------------------
#define GPIOAFEN1_53        (1<<21)                     // Set Async Falling Edge Detect on GPIO pin 53
#define GPIOAFEN1_52        (1<<20)                     // Set Async Falling Edge Detect on GPIO pin 52
#define GPIOAFEN1_51        (1<<19)                     // Set Async Falling Edge Detect on GPIO pin 51
#define GPIOAFEN1_50        (1<<18)                     // Set Async Falling Edge Detect on GPIO pin 50
#define GPIOAFEN1_49        (1<<17)                     // Set Async Falling Edge Detect on GPIO pin 49
#define GPIOAFEN1_48        (1<<16)                     // Set Async Falling Edge Detect on GPIO pin 48
#define GPIOAFEN1_47        (1<<15)                     // Set Async Falling Edge Detect on GPIO pin 47
#define GPIOAFEN1_46        (1<<14)                     // Set Async Falling Edge Detect on GPIO pin 46
#define GPIOAFEN1_45        (1<<13)                     // Set Async Falling Edge Detect on GPIO pin 45
#define GPIOAFEN1_44        (1<<12)                     // Set Async Falling Edge Detect on GPIO pin 44
#define GPIOAFEN1_43        (1<<11)                     // Set Async Falling Edge Detect on GPIO pin 43
#define GPIOAFEN1_42        (1<<10)                     // Set Async Falling Edge Detect on GPIO pin 42
#define GPIOAFEN1_41        (1<<9)                      // Set Async Falling Edge Detect on GPIO pin 41
#define GPIOAFEN1_40        (1<<8)                      // Set Async Falling Edge Detect on GPIO pin 40
#define GPIOAFEN1_39        (1<<7)                      // Set Async Falling Edge Detect on GPIO pin 39
#define GPIOAFEN1_38        (1<<6)                      // Set Async Falling Edge Detect on GPIO pin 38
#define GPIOAFEN1_37        (1<<5)                      // Set Async Falling Edge Detect on GPIO pin 37
#define GPIOAFEN1_36        (1<<4)                      // Set Async Falling Edge Detect on GPIO pin 36
#define GPIOAFEN1_35        (1<<3)                      // Set Async Falling Edge Detect on GPIO pin 35
#define GPIOAFEN1_34        (1<<2)                      // Set Async Falling Edge Detect on GPIO pin 34
#define GPIOAFEN1_33        (1<<1)                      // Set Async Falling Edge Detect on GPIO pin 33
#define GPIOAFEN1_32        (1<<0)                      // Set Async Falling Edge Detect on GPIO pin 32


#define GPIO_GPPUD          (0x94)                      // GPIO Pin Pull Up/Down Enable
//-------------------------------------------------------------------------------------------------------------------
#define GPIOPUD_OFF         (0b00)                      // Disable Pull Up/Down Control
#define GPIOPUD_DOWN        (0b01)                      // Enable Pull Down
#define GPIOPUD_UP          (0b10)                      // Enable Pull Up


#define GPIO_GPPUDCLK1      (0x98)                      // GPIO Pin Pull Up/Down Enable Clock 0
//-------------------------------------------------------------------------------------------------------------------
#define GPIOCLK1_31         (1<<31)                     // Assert Clock on GPIO pin 31
#define GPIOCLK1_30         (1<<30)                     // Assert Clock on GPIO pin 30
#define GPIOCLK1_29         (1<<29)                     // Assert Clock on GPIO pin 29
#define GPIOCLK1_28         (1<<28)                     // Assert Clock on GPIO pin 28
#define GPIOCLK1_27         (1<<27)                     // Assert Clock on GPIO pin 27
#define GPIOCLK1_26         (1<<26)                     // Assert Clock on GPIO pin 26
#define GPIOCLK1_25         (1<<25)                     // Assert Clock on GPIO pin 25
#define GPIOCLK1_24         (1<<24)                     // Assert Clock on GPIO pin 24
#define GPIOCLK1_23         (1<<23)                     // Assert Clock on GPIO pin 23
#define GPIOCLK1_22         (1<<22)                     // Assert Clock on GPIO pin 22
#define GPIOCLK1_21         (1<<21)                     // Assert Clock on GPIO pin 21
#define GPIOCLK1_20         (1<<20)                     // Assert Clock on GPIO pin 20
#define GPIOCLK1_19         (1<<19)                     // Assert Clock on GPIO pin 19
#define GPIOCLK1_18         (1<<18)                     // Assert Clock on GPIO pin 18
#define GPIOCLK1_17         (1<<17)                     // Assert Clock on GPIO pin 17
#define GPIOCLK1_16         (1<<16)                     // Assert Clock on GPIO pin 16
#define GPIOCLK1_15         (1<<15)                     // Assert Clock on GPIO pin 15
#define GPIOCLK1_14         (1<<14)                     // Assert Clock on GPIO pin 14
#define GPIOCLK1_13         (1<<13)                     // Assert Clock on GPIO pin 13
#define GPIOCLK1_12         (1<<12)                     // Assert Clock on GPIO pin 12
#define GPIOCLK1_11         (1<<11)                     // Assert Clock on GPIO pin 11
#define GPIOCLK1_10         (1<<10)                     // Assert Clock on GPIO pin 10
#define GPIOCLK1_9          (1<<9)                      // Assert Clock on GPIO pin 9
#define GPIOCLK1_8          (1<<8)                      // Assert Clock on GPIO pin 8
#define GPIOCLK1_7          (1<<7)                      // Assert Clock on GPIO pin 7
#define GPIOCLK1_6          (1<<6)                      // Assert Clock on GPIO pin 6
#define GPIOCLK1_5          (1<<5)                      // Assert Clock on GPIO pin 5
#define GPIOCLK1_4          (1<<4)                      // Assert Clock on GPIO pin 4
#define GPIOCLK1_3          (1<<3)                      // Assert Clock on GPIO pin 3
#define GPIOCLK1_2          (1<<2)                      // Assert Clock on GPIO pin 2
#define GPIOCLK1_1          (1<<1)                      // Assert Clock on GPIO pin 1
#define GPIOCLK1_0          (1<<0)                      // Assert Clock on GPIO pin 0


#define GPIO_GPPUDCLK2      (0x9c)                      // GPIO Pin Pull Up/Down Enable CLock 0
//-------------------------------------------------------------------------------------------------------------------
#define GPIOCLK2_53         (1<<21)                     // Assert Clock on GPIO pin 53
#define GPIOCLK2_52         (1<<20)                     // Assert Clock on GPIO pin 52
#define GPIOCLK2_51         (1<<19)                     // Assert Clock on GPIO pin 51
#define GPIOCLK2_50         (1<<18)                     // Assert Clock on GPIO pin 50
#define GPIOCLK2_49         (1<<17)                     // Assert Clock on GPIO pin 49
#define GPIOCLK2_48         (1<<16)                     // Assert Clock on GPIO pin 48
#define GPIOCLK2_47         (1<<15)                     // Assert Clock on GPIO pin 47
#define GPIOCLK2_46         (1<<14)                     // Assert Clock on GPIO pin 46
#define GPIOCLK2_45         (1<<13)                     // Assert Clock on GPIO pin 45
#define GPIOCLK2_44         (1<<12)                     // Assert Clock on GPIO pin 44
#define GPIOCLK2_43         (1<<11)                     // Assert Clock on GPIO pin 43
#define GPIOCLK2_42         (1<<10)                     // Assert Clock on GPIO pin 42
#define GPIOCLK2_41         (1<<9)                      // Assert Clock on GPIO pin 41
#define GPIOCLK2_40         (1<<8)                      // Assert Clock on GPIO pin 40
#define GPIOCLK2_39         (1<<7)                      // Assert Clock on GPIO pin 39
#define GPIOCLK2_38         (1<<6)                      // Assert Clock on GPIO pin 38
#define GPIOCLK2_37         (1<<5)                      // Assert Clock on GPIO pin 37
#define GPIOCLK2_36         (1<<4)                      // Assert Clock on GPIO pin 36
#define GPIOCLK2_35         (1<<3)                      // Assert Clock on GPIO pin 35
#define GPIOCLK2_34         (1<<2)                      // Assert Clock on GPIO pin 34
#define GPIOCLK2_33         (1<<1)                      // Assert Clock on GPIO pin 33
#define GPIOCLK2_32         (1<<0)                      // Assert Clock on GPIO pin 32


