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
	uint8_t mySecretValues[20];			// to check 2 simultanious fifos
	volatile fifo_handle_t hfifo, my2ndFifo;


    fifo_init(&hfifo, magicValues, sizeof(magicValues), sizeof(*magicValues));
	fifo_init(&my2ndFifo, mySecretValues, sizeof(mySecretValues), sizeof(*mySecretValues));

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
		fifo_put(&my2ndFifo, &j);
		fifo_put(&hfifo, &i);
	}
	for (i = 1, j = 0; i < sizeof(magicValues) / sizeof(*magicValues); i++, j++)
	{
		fifo_get(&my2ndFifo, &secretValue);
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
    
	char dummy;
	scanf("%s", &dummy);
}
