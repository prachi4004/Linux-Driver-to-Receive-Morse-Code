#ifndef _MORSE_DRIVER_H_
#define _MORSE_DRIVER_H_

//#include <linux/types.h>
#include <linux/ioctl.h>
//#include <linux/uaccess.h>

#define MORSE_IOC_MAGIC  'M'
#define MORSE_IOC_RESET  _IO(MORSE_IOC_MAGIC, 1)  // Reset the Morse code input
#define MORSE_IOC_SEND   _IOW(MORSE_IOC_MAGIC, 2, char *)  // Send Morse code to driver

// Morse code timing constants (duration-based classification)
#define DOT_DURATION    200   // milliseconds (short press duration for dot)
#define DASH_DURATION   600   // milliseconds (long press duration for dash)

#endif // _MORSE_DRIVER_H_
