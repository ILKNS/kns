#include "../inc/kns/ioctl.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

/* Only ia64 requires this */
#ifdef __ia64__
#define ADDR (void *)(0x8000000000000000UL)
#define FLAGS (MAP_SHARED | MAP_FIXED)
#else
#define ADDR (void *)(0x0UL)
#define FLAGS (MAP_SHARED)
#endif

void ioctl_set_addr(int File, void *addr){
	int ret;

	if((ret = ioctl(File, PASSING_MEM_ADDR, addr)) < 0){
		printf("ioctl failed.\n");
		exit(-1);
	}
}

void main(){
	int fd, hugefd, ret;
	void* addr;
	extern int errno;

	if((fd = open("/dev/"DEVICE_NAME"_DEV", O_WRONLY)) < 0){
		printf("Unable to open device file /dev/"DEVICE_NAME"_DEV, %d\n", fd);
		exit(-1);
	}

	addr = mmap(0, BUF_SIZE, PROT_READ | PROT_WRITE,
	MAP_SHARED | MAP_HUGETLB | MAP_ANONYMOUS, -1, 0);

/*	if((hugefd = open("/mnt/huge/kns", O_CREAT | O_RDWR, 0755)) < 0){
		printf("Unable to creat file /mnt/huge/kns\n");
		exit(-1);
	}

	if((addr = mmap(ADDR, BUF_SIZE, PROT_READ | PROT_WRITE, FLAGS, hugefd, 0)) == MAP_FAILED){
		printf("%d\n", errno);
		perror("mmap hugepage failed.");
	}*/


	ioctl_set_addr(fd, &addr);
	printf("%p\n", addr);
}