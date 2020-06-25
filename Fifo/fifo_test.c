/**
 * @file fifo_test.c
 * @brief this Programm is used to test the fifo library
 * @author Josef Aschwanden
 * @date Jun - 2020
 * @version 1.0
 */

// *** INCLUDES ***
#include <stdio.h>
#include "fifo.h"
#include <stdint.h>

/**
 * @brief this function is a small test for the FIFO Library
 */
int main(void)
{
	uint64_t magicValues[10];
	volatile fifo_handle_t hfifo;
	fifo_handle_t *hFifo2 = NULL;


    fifo_init(&hfifo, magicValues, sizeof(magicValues), sizeof(*magicValues));
	hFifo2 = fifo_init_malloc(10, sizeof(uint64_t));

	if (hFifo2 == NULL)
	{
		printf("Allocation failed!!!");
		return 0;
	}
	else
	{
		printf("Allocation_success!!!");
	}
	

    printf("Size Basetype: %i", hfifo.basetype_size);
    printf("\nSize: %i", hfifo.size);
    printf("\nRead Index: %i", hfifo.read_idx);
    printf("\nRead Index: %i", hfifo.write_idx);
	if (hfifo.pFifo != magicValues)
    {
        printf("pFifo is wrong");
    }
    else
    {
        printf("\npFifo Success");
    }
	uint64_t i;
	uint8_t j;
	uint64_t magicValue;
	uint8_t secretValue;
	for (i = 1, j = 0; i < sizeof(magicValues) / sizeof(*magicValues); i++, j++)
	{
		fifo_put(hFifo2, &j);
		fifo_put(&hfifo, &i);
	}
	if (fifo_hasSpaceLeft(hFifo2))
	{
		printf("function fifo_hasSpace_left() is shitty");
	}
	else
	{
		printf("\nOK!!");
	}
	if (!fifo_hasElementsLeft(hFifo2))
	{
		printf("function fifo_hasElements_left() is shitty");
	}
	else
	{
		printf("\nOK!!");
	}
	for (i = 1, j = 0; i < sizeof(magicValues) / sizeof(*magicValues); i++, j++)
	{
		fifo_get(hFifo2, &secretValue);
		fifo_get(&hfifo, &magicValue);

		if (secretValue != j)
			printf("TF!!!!");
		else
			printf("\nSecret Value: %i, wich is correct", secretValue);

		if (magicValue != i)
			printf("TF!!!!");
		else
			printf("\nMagic Value: %i, wich is correct", magicValue);
	}
	if (!fifo_hasSpaceLeft(hFifo2))
	{
		printf("function fifo_hasSpace_left() is shitty");
	}
	else
	{
		printf("\nOK!!");
	}
	if (fifo_hasElementsLeft(hFifo2))
	{
		printf("function fifo_hasElements_left() is shitty");
	}
	else
	{
		printf("\nOK!!");
	}
	fifo_deinit_free(hFifo2);

	char dummy;
	scanf("%s", &dummy);
}
