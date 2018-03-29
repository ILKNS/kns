#pragma once

#include <linux/ioctl.h>

#define FIRST_MINOR 0
#define MINOR_CNT 1
#define MAJOR_NUM 99

#define PASSING_MEM_ADDR _IOW(MAJOR_NUM, 0, void*)

#define DEVICE_NAME "KNS_IOCTL"

extern void* mem_start;

int uio_init(void);
void uio_remove(void);