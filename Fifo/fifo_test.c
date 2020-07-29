/**
 * @file fifo_test.c
 * @brief this Programm is used to test the fifo library
 * @author Josef Aschwanden
 * @date Jul - 2020
 * @version 2.1
 */

// *** INCLUDES ***
#include <stdio.h>
#include "fifo.h"
#include <stdint.h>
#include "test.h"

/**
 * @brief this function is a test for the FIFO Library
 */
int main(void)
{
	test_init();
	printCritical();

	testInit();
	printCritical();
	
	testInitMalloc();
	printCritical();
	
	testDeinitFree();
	printCritical();
		
	testPut();
	printCritical();
	
	testFlush();
	printCritical();
	
	testGet();
	printCritical();
	
	testPutAndGet();
	printCritical();
	
	testHasElementsLeft();
	printCritical();
	
	testGetEndPtr();
	printCritical();
	
	testGetLevel();
	printCritical();

	testSkipRead();
	printCritical();

	testSkipWrite();
	printCritical();
}

