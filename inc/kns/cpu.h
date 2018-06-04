/*
 * cpu.h - definitions for x86_64 CPUs
 */

#pragma once

/*
 * Endianness
 */

#define __LITTLE_ENDIAN	1234
#define __BIG_ENDIAN	4321

#define __BYTE_ORDER	__LITTLE_ENDIAN

/* Function will be replaced by linux API*/
extern int cpu_run_on_one(void(*func), void*, unsigned int);