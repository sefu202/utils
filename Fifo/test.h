#include "fifo.h"
#include <stdio.h>
#include <stdint.h>

#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_RESET   "\x1b[0m"

#define print_debuginfo(return_value)	(printf("%sFile: %s, line: %i, returned %i%s\n", ANSI_COLOR_RED,__FILE__, __LINE__, return_value, ANSI_COLOR_RESET))
#define print_debugs(s)					(printf("%sFile: %s, line: %i, %s%s\n", ANSI_COLOR_RED,__FILE__, __LINE__, s, ANSI_COLOR_RESET))

void test_init(void);
void enterCritical(void);
void leaveCritical(void);
void printCritical(void);
void testSkipWrite(void);
void testSkipRead(void);
void testGetLevel(void);
void testGetEndPtr(void);
void testHasElementsLeft(void);
void testPutAndGet(void);
void testGet(void);
void testFlush(void);
void testPut(void);
void testDeinitFree(void);
void testInitMalloc(void);
void testInit(void);
