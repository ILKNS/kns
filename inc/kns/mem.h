#pragma once

#include <linux/const.h>

extern void* mem_phy_base;

enum {
	PAGE_SHIFT_4KB = 12,
	PAGE_SHIFT_2MB = 21,
	PAGE_SHIFT_1GB = 30,
};

#define BUF_SIZE		(_AC(8, UL) << PAGE_SHIFT_1GB)

enum {
	PAGE_SIZE_4KB = (_AC(1, UL) << PAGE_SHIFT_4KB), /* 4096 bytes */
	PAGE_SIZE_2MB = (_AC(1, UL) << PAGE_SHIFT_2MB), /* 2097152 bytes */
	PAGE_SIZE_1GB = (_AC(1, UL) << PAGE_SHIFT_1GB), /* 1073741824 bytes */
};

#define PGMASK_4KB	(PAGE_SIZE_4KB - 1)
#define PGMASK_2MB	(PAGE_SIZE_2MB - 1)
#define PGMASK_1GB	(PAGE_SIZE_1GB - 1)

/* page numbers */
#define PGN_4KB(la)	(((uintptr_t) (la)) >> PAGE_SHIFT_4KB)
#define PGN_2MB(la)	(((uintptr_t) (la)) >> PAGE_SHIFT_2MB)
#define PGN_1GB(la)	(((uintptr_t) (la)) >> PAGE_SHIFT_1GB)

#define PGOFF_4KB(la)	(((uintptr_t) (la)) & PGMASK_4KB)
#define PGOFF_2MB(la)	(((uintptr_t) (la)) & PGMASK_2MB)
#define PGOFF_1GB(la)	(((uintptr_t) (la)) & PGMASK_1GB)

#define PGADDR_4KB(la)	(((uintptr_t) (la)) & ~((uintptr_t) PGMASK_4KB))
#define PGADDR_2MB(la)	(((uintptr_t) (la)) & ~((uintptr_t) PGMASK_2MB))
#define PGADDR_1GB(la)	(((uintptr_t) (la)) & ~((uintptr_t) PGMASK_1GB))

#define MPOL_DEFAULT	 0
#define MPOL_BIND 		 1
#define MPOL_PREFERRED   2
#define MPOL_INTERLEAVE  3

#define MEM_USER_START			MEM_USER_DIRECT_BASE_ADDR
#define MEM_USER_END			MEM_USER_IOMAPM_END_ADDR

#define MEM_ZC_USER_START		MEM_USER_IOMAPM_BASE_ADDR
#define MEM_ZC_USER_END			MEM_USER_IOMAPK_END_ADDR

#ifndef MAP_FAILED
#define MAP_FAILED	((void *) -1)
#endif

//TODO Redesign these addresses
#define MEM_VIRTUAL_BASE_ADDR		0x0 		 /* memory is allocated here (2MB going up, 1GB going down) */
#define MEM_VIRTUAL_END_ADDR		BUF_SIZE
#define MEM_USER_DIRECT_BASE_ADDR	0x7000000000 /* start of direct user mappings (P = V) */
#define MEM_USER_DIRECT_END_ADDR	0x7F00000000 /* end of direct user mappings (P = V) */
#define MEM_USER_IOMAPM_BASE_ADDR	0x8000000000 /* user mappings controlled by IX */
#define MEM_USER_IOMAPM_END_ADDR	0x100000000000 /* end of user mappings controlled by IX */
#define MEM_USER_IOMAPK_BASE_ADDR	0x100000000000 /* batched system calls and network mbuf's */
#define MEM_USER_IOMAPK_END_ADDR	0x101000000000 /* end of batched system calls and network mbuf's */

void* mmap_hp_u2k(void* memaddr);