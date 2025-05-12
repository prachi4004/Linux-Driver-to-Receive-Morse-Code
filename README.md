# Linux-Driver-to-Receive-Morse-Code
This project implements a Linux kernel module that receives Morse code input from an ESP32 via serial communication. The user-space program reads the code and sends it to the kernel via ioctl. The module decodes the Morse code asynchronously using a workqueue and writes the output to a file, demonstrating device drivers and deferred execution.
