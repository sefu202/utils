#include "test.h"
#include "fifo.h"
#include <assert.h>

void testSkipWrite(void)
{
    printf("Test of fifo_skip_write() and _n() started\n");
	
		fifo_handle_t *pHandle = fifo_init_malloc(32, sizeof(uint32_t));
		for (uint8_t i = 0; i < 31; i++)
		{
			if (fifo_getLevel(pHandle) != i) print_debuginfo(i);
			if (fifo_skip_write(pHandle) != FIFO_NO_ERROR) print_debugs("");
		}
		if (fifo_skip_write(pHandle) != FIFO_FULL) print_debugs("");
		if (fifo_skip_write_n(pHandle, 1) != FIFO_FULL) print_debugs("");

		pHandle->_lock |= 0x01; // write lock
		if (fifo_skip_write(pHandle) != FIFO_BUISY) print_debugs("");
		if (fifo_skip_write(NULL) != FIFO_WRONG_PARAM) print_debugs("");
		if (fifo_skip_write_n(pHandle, 1) != FIFO_BUISY) print_debugs("");
		if (fifo_skip_write_n(NULL, 1) != FIFO_WRONG_PARAM) print_debugs("");
		pHandle->_lock = 0;
		
		fifo_flush(pHandle);
		if (fifo_skip_write_n(pHandle, (pHandle->size / pHandle->basetype_size) -1) != FIFO_NO_ERROR) print_debugs("");
		fifo_skip_read_n(pHandle, 3);
		if (fifo_skip_write_n(pHandle, 10), fifo_getLevel(pHandle) != (pHandle->size / pHandle->basetype_size) -1) print_debugs("");
		
		fifo_deinit_free(pHandle);
	
	printf("Test of fifo_skip_write() and _n() ended\n");
}

void testSkipRead(void)
{
    uint8_t dummy8;
    printf("Test of fifo_skip_read() and _n() started\n");

    fifo_handle_t *pHandle = fifo_init_malloc(16, sizeof(uint8_t));
    fifo_put(pHandle, "1");
    fifo_put(pHandle, "2");

    if (fifo_skip_read(pHandle) != FIFO_NO_ERROR) print_debugs("");
    if (fifo_get(pHandle, &dummy8), dummy8 != '2') print_debugs("");
    if(fifo_skip_read(pHandle) != FIFO_EMPTY) print_debugs("");

    pHandle->_lock |= 0x02; // read lock

    if (fifo_skip_read(pHandle) != FIFO_BUISY) print_debugs("");
    if (fifo_skip_read(NULL) != FIFO_WRONG_PARAM) print_debugs("");
    if (fifo_skip_read_n(pHandle, 1) != FIFO_BUISY) print_debugs("");
    if (fifo_skip_read_n(NULL, 1) != FIFO_WRONG_PARAM) print_debugs("");
    pHandle->_lock = 0;

    for (uint8_t j = 0; j < 3; j++)
    {
        for (uint8_t i = 0; i < 8; i++)
        {
            fifo_put(pHandle, &i);
        }

        if (fifo_skip_read_n(pHandle, 7) != FIFO_NO_ERROR) print_debugs("");
        if (fifo_get(pHandle, &dummy8), dummy8 != 7) print_debugs("");
        if(fifo_skip_read_n(pHandle, 1) != FIFO_EMPTY) print_debugs("");
    }
    // *** Test limiting of skipped elements to available elements ***
    for (uint8_t i = 0; i < 8; i++)
    {
        fifo_put(pHandle, &i);
    }
    fifo_skip_read_n(pHandle, 16);
    if (fifo_getLevel(pHandle) != 0) print_debugs("");
    fifo_deinit_free(pHandle);

    pHandle = fifo_init_malloc(16, sizeof(uint64_t));
    
    for (uint64_t i = 0; i < 15; i++)
    {
        fifo_put(pHandle, &i);
    }
    if (fifo_skip_read(pHandle), fifo_getLevel(pHandle) != 14) print_debuginfo(fifo_getLevel(pHandle));
    if (fifo_skip_read_n(pHandle, 2), fifo_getLevel(pHandle) != 12) print_debugs("");

	printf("Test of fifo_skip_read() and _n() ended\n");
}

void testGetLevel(void)
{
    printf("Test of fifo_getLevel() started\n");
    fifo_handle_t *pHandle = fifo_init_malloc(32, sizeof(uint32_t));
    if (fifo_getLevel(pHandle) != 0) print_debugs("");
    if (fifo_getEmptySpace(pHandle) != 31) print_debugs("");
    for (uint8_t i = 0, ret; i < 31; i++)
    {
        if ((ret = fifo_getLevel(pHandle)) != i) print_debuginfo(ret);
        if ((ret = fifo_getEmptySpace(pHandle)) != 31 - i) print_debuginfo(ret);
        fifo_skip_write(pHandle);
    }
    for (uint8_t i = 0; i < 31; i++)
    {
        fifo_skip_read(pHandle);
    }
    for (uint8_t i = 0, ret; i < 31; i++)
    {
        if ((ret = fifo_getLevel(pHandle)) != i) print_debuginfo(ret);
        if ((ret = fifo_getEmptySpace(pHandle)) != 31 - i) print_debuginfo(ret);
        fifo_skip_write(pHandle);
    }
    fifo_deinit_free(pHandle);
    printf("Test of fifo_getLevel() ended\n");
}

void testGetEndPtr(void)
{
    printf("Test of fifo_getEndPtr() has started\n");

    // ** Int test **
    fifo_handle_t myHandle;
    uint8_t fifo_buffer[8];
    fifo_init(&myHandle, &fifo_buffer, sizeof(fifo_buffer), sizeof(fifo_buffer[0]));

    if (fifo_getEndPtr(&myHandle) != &fifo_buffer[7]) print_debugs("");

    // ** Other int test **
    fifo_handle_t myHandle2;
    uint64_t fifo_buffer2[8];
    fifo_init(&myHandle2, &fifo_buffer2, sizeof(fifo_buffer2), sizeof(fifo_buffer2[0]));

    if (fifo_getEndPtr(&myHandle2) != &fifo_buffer2[sizeof(fifo_buffer2) / sizeof(fifo_buffer2[0]) -1]) print_debugs("");

    // ** Struct test **
    fifo_handle_t myHandle3;
    fifo_handle_t fifo_buffer3[8];
    fifo_init(&myHandle3, &fifo_buffer3, sizeof(fifo_buffer3), sizeof(fifo_buffer3[0]));

    if (fifo_getEndPtr(&myHandle3) != &fifo_buffer3[7]) print_debugs("");

	printf("Test of fifo_getEndPtr() ended\n");
}

void testHasElementsLeft(void)
{
    #define TEST_TYPE uint32_t
	#define TEST_BUFSIZE 11

	fifo_handle_t *pHandle = fifo_init_malloc(TEST_BUFSIZE, sizeof(TEST_TYPE));
    if (pHandle == NULL)
    {
        assert(0);
    }
    printf("Test of fifo_hasElementsLeft() and fifo_hasSpaceLeft started\n");
	if (fifo_hasElementsLeft(pHandle)) 	print_debugs("");
	if (!fifo_hasSpaceLeft(pHandle)) 	print_debugs("");

	for (uint8_t i = 0; i < (TEST_BUFSIZE -1); i++)
	{
		if (!fifo_hasSpaceLeft(pHandle)) 	print_debugs("");
        fifo_skip_write(pHandle);
		if (!fifo_hasElementsLeft(pHandle)) print_debugs("");
	}

	fifo_skip_write(pHandle);
	if (!fifo_hasElementsLeft(pHandle)) print_debugs("");
	if (fifo_hasSpaceLeft(pHandle)) 	print_debugs("");
	printf("Test of fifo_hasElementsLeft() and fifo_hasSpaceLeft ended\n");

	// *** cleanup ***
	fifo_deinit_free(pHandle);

    #undef TEST_TYPE
    #undef TEST_BUFSIZE
}

void testPutAndGet(void)
{	
    #define TEST_TYPE uint32_t
	#define TEST_TIMES 100	// this is only accurate when being a multiple of (TEST_BUFSIZE -1)
	#define TEST_BUFSIZE 11
    fifo_handle_t *pHandle = fifo_init_malloc(TEST_BUFSIZE * sizeof(TEST_TYPE), sizeof(TEST_TYPE));
	TEST_TYPE tx, should, rcv = 0xf, dummyTestType;
	if (pHandle == NULL)
	{
		print_debugs("Allocation failed");
		assert(0);
	}
    printf("Test of fifo_put() and fifo_get() started\n");
	/* This test pushes data into the fifo and then reads it out of the fifo, it always fills up the fifo and empties it afterwards */
	for (uint32_t i = 0; i < TEST_TIMES / (TEST_BUFSIZE -1); i++)
	{
		// ** put data into the buffer **
		for (uint32_t j = 0; j < (TEST_BUFSIZE-1); j++)	// load the buffer
		{
			tx = i * (TEST_BUFSIZE -1) + j;
			fifo_put(pHandle, &tx);	
		}
		// ** get the data
		for (uint32_t j = 0; j < (TEST_BUFSIZE-1); j++)
		{
			should = i * (TEST_BUFSIZE -1) + j;
			fifo_get(pHandle, &rcv);
			// ** Check the data **
			if (rcv != should)
			{
				print_debuginfo(should);
			}
		}
	}
	printf("Test of fifo_put() and fifo_get() ended\n");
    #undef TEST_TYPE
    #undef TEST_TIMES
    #undef TEST_BUFSIZE
}

void testGet(void)
{
    #define TESTFIFO_SIZE 8
    #define TESTVALUE 15	// test a specific value
	uint8_t dummy8 = TESTVALUE;
    fifoerror_t return_value_fifoerror;

    fifo_handle_t *pHandle = fifo_init_malloc(TESTFIFO_SIZE, sizeof(uint8_t));

	fifo_put(pHandle, "\0");	// put a value (\0) into the fifo note: this uses a string because a string already is a pointer
	printf("Test of fifo_get() started\n");
	if ((return_value_fifoerror = fifo_get(pHandle, NULL)) 		!= FIFO_WRONG_PARAM) 	print_debugs(""); // bad call
	if ((return_value_fifoerror = fifo_get(NULL, 	&dummy8)) 	!= FIFO_WRONG_PARAM) 	print_debugs(""); // bad call
	pHandle->_lock = 0x2;	// read lock
	if ((return_value_fifoerror = fifo_get(pHandle, &dummy8)) 	!= FIFO_BUISY) print_debuginfo(return_value_fifoerror),			print_debugs(""); // call while locked
	pHandle->_lock = 0;		// unlock
	if (dummy8 != TESTVALUE) print_debugs("dummy8 got written to illegaly by fifo_get()");
	if ((return_value_fifoerror = fifo_get(pHandle, &dummy8)) 	!= FIFO_NO_ERROR)		print_debugs(""); // good call
	if (dummy8 != '\0') print_debugs("fifo_get() doesn't work");
	dummy8 = TESTVALUE;
	if ((return_value_fifoerror = fifo_get(pHandle, &dummy8)) 	!= FIFO_EMPTY)			print_debugs(""); // call while empty
	if (dummy8 != TESTVALUE) print_debugs("dummy8 got written to illegaly by fifo_get()");
	printf("Test of fifo_get() ended\n");
	fifo_deinit_free(pHandle);
    #undef TESTFIFO_SIZE
}

void testFlush(void)
{
    #define TESTFIFO_SIZE 8
    fifoerror_t return_value_fifoerror;

    fifo_handle_t *pHandle = fifo_init_malloc(TESTFIFO_SIZE, sizeof(uint8_t));

	printf("Test of fifo_flush() started\n");
	while(fifo_put(pHandle, "E") != FIFO_FULL);	// fill up the fifo
	if ((return_value_fifoerror = fifo_flush(NULL)) != FIFO_WRONG_PARAM) 	print_debugs("");	// bad call
	pHandle->_lock = 0x1;	// write lock
	if ((return_value_fifoerror = fifo_flush(pHandle)) != FIFO_BUISY) 		print_debugs("");	// call while locked
	pHandle->_lock = 0x2;	// read lock
	if ((return_value_fifoerror = fifo_flush(pHandle)) != FIFO_BUISY) 		print_debugs("");	// call while locked
	pHandle->_lock = 0;		// unlock
	if ((return_value_fifoerror = fifo_flush(pHandle)) != FIFO_NO_ERROR) 	print_debugs("");	// good call
	if (pHandle->read_idx != pHandle->write_idx) 							print_debugs("fifo_flush() does not work correctly");
	printf("Test of fifo_flush() ended\n");
    #undef TESTFIFO_SIZE
}

void testPut(void)
{
    const char string_test[] = "Hello World";
	// *** fifo_put() ***
	#define TESTFIFO_SIZE 8
	fifoerror_t return_value_fifoerror;
	char dummy;
	printf("Test of fifo_put() started\n");
	fifo_handle_t *pHandle = fifo_init_malloc(TESTFIFO_SIZE, sizeof(uint8_t));
	//if ((return_value_fifoerror = fifo_put(pHandle, &string_test[0])) 	!= FIFO_NO_ERROR) 		print_debugs("");	// good call
	if ((return_value_fifoerror = fifo_put(NULL, 	&string_test[1])) 	!= FIFO_WRONG_PARAM) 	print_debugs("");	// null pointer handle
	if ((return_value_fifoerror = fifo_put(pHandle, NULL)) 				!= FIFO_WRONG_PARAM) 	print_debugs("");	// null pointer data
	
	pHandle->_lock |= 0x01;	// write lock
	if ((return_value_fifoerror = fifo_put(pHandle, &string_test[1])) 	!= FIFO_BUISY) 			print_debugs("");	// handle locked
	pHandle->_lock &= ~(0x01); 	// disable write lock

	for (uint8_t i = 0; i < (TESTFIFO_SIZE -1); i++)	// the fifo can only store size -1 elements
	{
		if ((return_value_fifoerror = fifo_put(pHandle, &string_test[i])) != FIFO_NO_ERROR) 	print_debugs(""); // fill up the fifo
	}

	if ((return_value_fifoerror = fifo_put(pHandle, &string_test[7])) != FIFO_FULL) 	print_debugs("");	// now the fifo is full
	fifo_get(pHandle, &dummy);	// empty the fifo by one
	if ((return_value_fifoerror = fifo_put(pHandle, &string_test[7])) != FIFO_NO_ERROR) print_debugs("");	// now write again
	printf("Test of fifo_put() ended\n");
}

void testDeinitFree(void)
{
    fifo_handle_t *pHandle;
	// *** fifo_deinit_free() ***
	printf("Note Memory usage of the test_fifo task\npress enter to start");
	getc(stdin);
	printf("Test of fifo_deinit_free() started\n");
	// ** Memory leek test **
	for (uint32_t i = 0; i < 1000000; i++)
	{
		pHandle = fifo_init_malloc(MAX_FIFO_SIZE, FIFO_MAX_BASETYPE_SIZE);
		fifo_deinit_free(pHandle);
	}
	printf("Check memory usage of the test_fifo task\nPress enter to end the test\n");
	printf("if memory usage didnt change by much, fifo_deinit_free() works");
	getc(stdin);
	printf("Test of fifo_deinit_free() ended\n");

}

void testInitMalloc(void)
{
    fifo_handle_t *pHandle = NULL;

	printf("Test of fifo_init_malloc() started\n");
	if ((pHandle = fifo_init_malloc(0, 					sizeof(uint32_t))) 	!= NULL) 			print_debugs("pHandle != NULL");	// fifo size of 0
	if ((pHandle = fifo_init_malloc(MAX_FIFO_SIZE +1, 	sizeof(uint32_t))) 	!= NULL) 			print_debugs("pHandle != NULL");	// fifo size bigger than maximum
	if ((pHandle = fifo_init_malloc(16, 				0))					!= NULL) 			print_debugs("pHandle != NULL");	// basetype size of 0
	if ((pHandle = fifo_init_malloc(16,					FIFO_MAX_BASETYPE_SIZE + 1)) != NULL) 	print_debugs("pHandle != NULL");	// basetype size bigger than FIFO_MAX_BASETYEPE_SIZE
	if ((pHandle = fifo_init_malloc(16, 				sizeof(uint32_t))) 	== NULL) 			print_debugs("pHandle == NULL");	// good call
	if (pHandle->basetype_size != sizeof(uint32_t)) print_debugs("");
	if (pHandle->pFifo == NULL) 					print_debugs("");
	if (pHandle->read_idx != 0) 					print_debugs("");
	if (pHandle->size != 16 * sizeof(uint32_t)) 	print_debugs("");
	if (pHandle->write_idx != 0) 					print_debugs("");
	if (pHandle->_lock != 0) 						print_debugs("");
	fifo_deinit_free(pHandle);	// cleanup
	printf("Test of fifo_init_malloc() ended\n");    
}

void testInit(void)
{
    fifo_handle_t myHandle;
	uint32_t myFifo[8];

	int8_t return_value;

	// *** fifo_init() ***
	printf("Test of fifo_init() started\n");
	if ((return_value = fifo_init(&myHandle, myFifo, sizeof(myFifo),		sizeof(myFifo[0])))			!= 0)	print_debuginfo(return_value);	// good call
	if ((return_value = fifo_init(NULL, 	myFifo,  sizeof(myFifo), 		sizeof(myFifo[0]))) 		!= -1) 	print_debuginfo(return_value);	// null pointer at param 1
	if ((return_value = fifo_init(&myHandle, NULL, 	 sizeof(myFifo), 		sizeof(myFifo[0]))) 		!= -1) 	print_debuginfo(return_value);	// null pointer at param 2
	if ((return_value = fifo_init(&myHandle, myFifo, 0, 					sizeof(myFifo[0]))) 		!= -2) 	print_debuginfo(return_value);	// fifo size of 0
	if ((return_value = fifo_init(&myHandle, myFifo, MAX_FIFO_SIZE + 1, 	sizeof(myFifo[0]))) 		!= -2) 	print_debuginfo(return_value);	// fifo bigger than maximum
	if ((return_value = fifo_init(&myHandle, myFifo, sizeof(myFifo), 		0)) 						!= -3) 	print_debuginfo(return_value);	// basetype size of 0
	if ((return_value = fifo_init(&myHandle, myFifo, sizeof(myFifo), 		FIFO_MAX_BASETYPE_SIZE +1)) != -3) 	print_debuginfo(return_value);	// basetype size bigger than allowed
	if (myHandle.basetype_size != sizeof(myFifo[0]))	print_debugs("");
	if (myHandle.pFifo != &myFifo) 						print_debugs("");
	if (myHandle.read_idx != 0) 						print_debugs("");
	if (myHandle.size != sizeof(myFifo)) 				print_debugs("");
	if (myHandle.write_idx != 0) 						print_debugs("");
	if (myHandle._lock != 0) 							print_debugs("");
	printf("Test of fifo_init() ended\n");
}

static int criticalCounter;	// to test the calling of critical macros, should be 0 at the end of the program
static int criticalCalls;

void test_init(void)
{
    criticalCounter = criticalCalls = 0;
}

void enterCritical(void)
{
	criticalCalls++;
	criticalCounter++;
}

void leaveCritical(void)
{
	criticalCalls++;
	criticalCounter--;
}

void printCritical(void)
{
	printf("Counter: %i, Calls: %i\n", criticalCounter, criticalCalls);
}