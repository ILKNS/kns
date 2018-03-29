#include "../inc/kns/ioctl.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>

void ioctl_set_addr(int File, void *addr){
	int ret;

	if((ret = ioctl(File, PASSING_MEM_ADDR, addr)) < 0){
		printf("ioctl failed.\n");
		exit(-1);
	}
}

void main(){
	int fd, ret;
	void* addr = NULL;

	if((fd = open("/dev/"DEVICE_NAME"_DEV", O_WRONLY)) < 0){
		printf("Unable to open device file /dev/"DEVICE_NAME"_DEV, %d\n", fd);
		exit(-1);
	}

	addr = mmap(0, 8UL<<30, PROT_READ | PROT_WRITE,
	MAP_SHARED | MAP_HUGETLB | MAP_ANONYMOUS, -1, 0);

	ioctl_set_addr(fd, &addr);
	printf("%p\n", addr);
	getchar();
	munmap(addr, 8UL<<20);
}