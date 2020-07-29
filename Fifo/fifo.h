/**
 * @file fifo.h
 * @brief this library can be used to handle fifo memory buffers
 * The Library consists of the fuctions fifo_init(), fifo_put() and fifo_get()
 * and the types fifoerror_t and fifo_handle_t
 * This library can handle multiple fifos of multiple data types.
 * @note each fifo can only store (size / basetype_size) -1 elements
 * @author Josef Aschwanden
 * @date Jul - 2020
 * @version 1.2
 * Changes:
 * 1.1: Added _hasElementsLeft(), _hasSpaceLeft(), _flush(), bugfixes
 *      Josef Aschwanden
 * 1.2:
 *      Added few functions, bugfixes, better async proof
 */

#ifndef _FIFO_H_
#define _FIFO_H_

#ifdef __cplusplus
extern "C" {
#endif
 
// *** INCLUDES ***
#include <stdint.h>
#include <stdbool.h>

#ifdef TEST_FIFO
#include "test.h"
#endif

// *** DEFINES ***
/**
 * @brief maximum size of a fifo.
 * This Macro is internally needed for the fifo and should be set as small as possible
 */
#define MAX_FIFO_SIZE   128

/**
 * @brief Enable Dynamic allocation of fifos
 */
#define FIFO_ALLOW_MALLOC   true

#if MAX_FIFO_SIZE <= UINT8_MAX
    #define FIFO_INDEX_TYPE uint8_t
#elif MAX_FIFO_SIZE <= UINT16_MAX
    #define FIFO_INDEX_TYPE uint16_t
#elif MAX_FIFO_SIZE <= UINT32_MAX
    #define FIFO_INDEX_TYPE uint32_t
#else
    #error "fifo.c: FIfo size bigger than UINT32_MAX is not supported yet! Change define in fifo.h"
#endif

/**
 * @brief maximum size of the FIFO basetype.
 * This Macro is internally needed for the fifo and should be set as small as possible
 */
#define FIFO_MAX_BASETYPE_SIZE 128

#if FIFO_MAX_BASETYPE_SIZE <= UINT8_MAX
    #define SIZE_FIFO_BASE_TYPE uint8_t
#elif FIFO_MAX_BASETYPE_SIZE <= UINT16_MAX
    #define SIZE_FIFO_BASE_TYPE uint16_t
#elif FIFO_MAX_BASETYPE_SIZE <= UINT32_MAX
    #define SIZE_FIFO_BASE_TYPE uint32_t
#else
    #error "fifo.c: Fifo basetype size bigger than UINT32_MAX is not supported yet! Change define in fifo.h"
#endif


/**
 * @defgroup async_macros Asynchronous Support Macros
 * These Macros are only needed if multiple asynchrounous threads can access the fifo. 
 * They prevent variables from being changed in non atomic Opeartations
 * @note The enter and leave macro are always in the same scope so variables can be created in them
 */
/**
 * @addtogroup async_macros
 * @{
 */
/**
 * This Macro gets called when entering a critical section in the program,
 * it should be defined to disable all other Threads with access to the fifo
 */
#ifdef TEST_FIFO
#define FIFO_ENTER_CRITICAL() enterCritical()
#else
#define FIFO_ENTER_CRITICAL()   /*User definition*/
#endif

/**
 * This Macro gets called when leaving a critical section in the program,
 * it should be defined to revert the changes made with FIFO_ENTER_CRITICAL()
 */
#ifdef TEST_FIFO
#define FIFO_LEAVE_CRITICAL()  leaveCritical()
#else
#define FIFO_LEAVE_CRITICAL()   /*User definition*/
#endif

/**@}*/


// *** TYPEDEF ***
/**
 * @brief fifo error values
 */
typedef enum{
    FIFO_NO_ERROR,    /**< NO Error Ocurred*/
    FIFO_FULL,        /**< FIFO is full, data can not be stored */
    FIFO_EMPTY,       /**< FIFO is empty, no data can be read */
    FIFO_WRONG_PARAM, /**< Wrong Parameters given to a function */
    FIFO_BUISY        /**< FIFO is buisy */
}fifoerror_t;

/**
 * @brief this structure is used as handle for the fifo library
 */
typedef struct{
    SIZE_FIFO_BASE_TYPE basetype_size;      /*!< sizeof the fifo basetype (bytes) */
    FIFO_INDEX_TYPE size;                   /*!< size of the fifo memory (bytes) */
    FIFO_INDEX_TYPE read_idx;               /*!< read index for fifo read access, offset from pFifo in bytes */
    FIFO_INDEX_TYPE write_idx;              /*!< write index for fifo write access, offset from pFifo in bytes */
    void *pFifo;                            /*!< pointer to the first adress of the fifo memory */
	uint8_t _lock;                          /*!< flag to lock the fifo */
}fifo_handle_t;

/** @defgroup fifo_core Core Fifo Functions
 * @brief Functions essential for the use of the fifo
 */

/** 
 * @addtogroup fifo_core
 * @{
 */
/**
 * @brief initializes a fifo handle
 * @note If _DEBUG is defined every Parameter will be checked with assert()
 * @param pHandle pointer to the fifo handle
 * @param pFifo pointer to the fifo memory
 * @param size_fifo size of the fifo in bytes, must be a multiple of bastype size
 * @param basetype_size size of the fifo basetype eg: sizeof(uint8_t)
 * @retval 0 = success
 * @retval -1 = NULL Pointer as Parameter
 * @retval -2 = invalid fifo size
 * @retval -3 = invalid basetype_size
 * @retval -4 = size_fifo is not a multiple of basetype_size
 */
int8_t fifo_init(volatile fifo_handle_t *pHandle, void *pFifo, FIFO_INDEX_TYPE size_fifo, SIZE_FIFO_BASE_TYPE basetype_size);

#if FIFO_ALLOW_MALLOC
/**
 * @brief allocates a fifo handle and the fifo memory, and initializes it
 * @note If _DEBUG is defined every Parameter will be checked with assert()
 * @note memory has to be freed with fifo_deinit_free()
 * @param size_fifo size of the fifo in elements
 * @param basetype_size size of the fifo basetype eg: sizeof(uint8_t)
 * @retval NULL = Allocation failed
 * @return pointer to the fifo handle
 */
fifo_handle_t* fifo_init_malloc(FIFO_INDEX_TYPE size_fifo, SIZE_FIFO_BASE_TYPE basetype_size);

/**
 * @brief deallocates a fifo handle and its buffer
 * @param pHandle pointer to the Fifo handle
 */
void fifo_deinit_free(volatile fifo_handle_t *pHandle);
#endif   /* FIFO_ALLOW_MALLOC */

/**
 * @brief puts an element into the fifo.
 * @note If _DEBUG is defined every Parameter will be checked with assert()
 * @param pHandle pointer to the fifo handle
 * @param [in] pData pointer to the data to be put onto the fifo
 * @return fifoerror_t
 */
fifoerror_t fifo_put(volatile fifo_handle_t *pHandle, const void *pData);

/**
 * @brief puts an element into the fifo.
 * @note If _DEBUG is defined every Parameter will be checked with assert()
 * @param pHandle pointer to the handle of the fifo
 * @param [out] pData pointer to the storage for the data from the fifo
 * @return fifoerror_t
 */
fifoerror_t fifo_get(volatile fifo_handle_t* pHandle, void *pData);
/**
 * @}
 */

/** @defgroup fifo_ext Extended Fifo Functions
 * @brief Functions optional for the use of the fifo
 */

/** 
 * @addtogroup fifo_ext
 * @{
 */

/**
 * @brief checks if a fifo still has elements in it
 * @note pHandle gets checked with assert() when _DEBUG is defined
 * @retval true = has elements left
 */
bool fifo_hasElementsLeft(volatile fifo_handle_t *pHandle);

/**
 * @brief checks if a fifo still free space in it
 * @note pHandle gets checked with assert() when _DEBUG is defined
 * @retval true = has free space left
 */
bool fifo_hasSpaceLeft(volatile fifo_handle_t *pHandle);

/**
 * @brief flushes a FIFO
 * @return FIFO_NO_ERROR    Everything worked
 * @return FIFO_BUISY       FIFO handle is locked
 */
fifoerror_t fifo_flush(volatile fifo_handle_t *pHandle);

/**
 * @brief returns the pointer to the end of the fifo buffer
 * @param pHandle pointer to the fifo handle
 * @retval pointer to the end of the fifo
 */
void *fifo_getEndPtr(fifo_handle_t *pHandle);

/**
 * @brief acts as if the fifo had been read
 * This may be useful to read from the fifo outside of this library, for example if a function gets passed a fifo pointer
 * and it reads multiple times from the fifo without calling a fifo function
 * @note Be careful when reading from the fifo without fifo functions!!
 * @param pHandle pointer to the fifo handle
 * @param n ammount of read cycles to be skipped
 * @return fifoerror_t
 */
fifoerror_t fifo_skip_read(fifo_handle_t *pHandle);

/**
 * @brief acts as if the fifo had been read n times
 * This may be useful to read from the fifo outside of this library, for example if a function gets passed a fifo pointer
 * and it reads multiple times from the fifo without calling a fifo function
 * @note Be careful when reading from the fifo without fifo functions!!
 * @param pHandle pointer to the fifo handle
 * @param n ammount of read cycles to be skipped
 * @return fifoerror_t
 */
fifoerror_t fifo_skip_read_n(fifo_handle_t *pHandle, FIFO_INDEX_TYPE n);

/**
 * @brief acts as if something had been written to the fifo without changing its contents
 * This may be useful to write to the fifo outside of this library, for example if a function gets passed a fifo pointer
 * and it writes multiple times to the fifo without calling a fifo function
 * @note Be careful when writing to the fifo without fifo functions!!
 * @param pHandle pointer to the fifo handle
 * @return fifoerror_t
 */
fifoerror_t fifo_skip_write(fifo_handle_t *pHandle);

/**
 * @brief acts as if the fifo had been written to n times without changing its contents on the physical memory
 * This may be useful to write to the fifo outside of this library, for example if a function gets passed a fifo pointer
 * and it writes multiple times to the fifo without calling a fifo function
 * @note Be careful when writing to the fifo without fifo functions!!
 * @param pHandle pointer to the fifo handle
 * @param n ammount of write cycles to be skipped
 * @return fifoerror_t
 */
fifoerror_t fifo_skip_write_n(fifo_handle_t *pHandle, FIFO_INDEX_TYPE n);

/**
 * @brief returns the fill level of a fifo
 * @param pHandle pointer to a fifo handle
 * @retval fill level of the fifo in elements
 */
FIFO_INDEX_TYPE fifo_getLevel(fifo_handle_t *pHandle);

/**
 * @brief returns the empty space of a fifo
 * @param pHandle pointer to a fifo handle
 * @retval empty space in the fifo in elements
 */
FIFO_INDEX_TYPE fifo_getEmptySpace(fifo_handle_t *pHandle);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif  // _FIFO_H_