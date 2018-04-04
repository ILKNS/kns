#pragma once

#include <linux/ioctl.h>
#include <linux/const.h>

#define FIRST_MINOR 0
#define MINOR_CNT 1
#define MAJOR_NUM 99

#define PASSING_MEM_ADDR _IOW(MAJOR_NUM, 0, void*)

#define DEVICE_NAME "KNS_IOCTL"

#define BUF_SIZE		(_AC(1,UL)<<31)
#define PAGE_SIZE_2MB 	(_AC(1,UL)<<21)

extern void* mem_start;

int uio_init(void);
void uio_remove(void);

void* mmap_hp_u2k(void* memaddr);