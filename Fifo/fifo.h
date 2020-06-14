/**
 * @file fifo.h
 * @brief this library can be used to handle fifo memory buffers
 * The Library consists of the fuctions fifo_init(), fifo_put() and fifo_get()
 * and the types fifoerror_t and fifo_handle_t
 * This library can handle multiple fifos of multiple data types.
 * @author Josef Aschwanden
 * @date Jun - 2020
 * @version 1.0
 */

 #ifndef _FIFO_H_
 #define _FIFO_H_
 
// *** INCLUDES ***
#include <stdint.h>

// *** DEFINES ***
/**
 * @brief maximum size of a fifo.
 * This Macro is internally needed for the fifo and should be set as small as possible
 */
#define MAX_FIFO_SIZE   255

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
#define FIFO_MAX_BASETYPE_SIZE 255

#if FIFO_MAX_BASETYPE_SIZE <= UINT8_MAX
    #define SIZE_FIFO_BASE_TYPE uint8_t
#elif FIFO_MAX_BASETYPE_SIZE <= UINT16_MAX
    #define SIZE_FIFO_BASE_TYPE uint16_t
#elif FIFO_MAX_BASETYPE_SIZE <= UINT32_MAX
    #define SIZE_FIFO_BASE_TYPE uint32_t
#else
    #error "fifo.c: Fifo basetype size bigger than UINT32_MAX is not supported yet! Change define in fifo.h"
#endif

// *** Critical Definitions ***
/**
 * This Macro gets called when entering a critical section in the program,
 * it should be defined to disable all other Threads with access to the fifo
 * @note only needed if multiple asynchrounous threads can access the fifo
 */
#define FIFO_ENTER_CRITICAL()
/**
 * This Macro gets called when leaving a critical section in the program,
 * it should be defined to revert the changes made with FIFO_ENTER_CRITICAL()
 * @note only needed if multiple asynchrounous threads can access the fifo
 */
#define FIFO_LEAVE_CRITICAL()

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
    SIZE_FIFO_BASE_TYPE basetype_size;      /*!< sizeof the fifo basetype */
    FIFO_INDEX_TYPE size;                   /*!< size of the fifo memory in bytes */
    FIFO_INDEX_TYPE read_idx;               /*!< read index for fifo read access */
    FIFO_INDEX_TYPE write_idx;              /*!< write index for fifo write access */
    void *pFifo;                            /*!< pointer to the first adress of the fifo memory */
	uint8_t _lock;                          /*!< flag to lock the fifo */
}fifo_handle_t;

/**
 * @brief initializes a fifo handle
 * @note If _DEBUG is defined every Parameter will be checked with assert()
 * @param pHandle pointer to the fifo handle
 * @param pFifo pointer to the fifo memory
 * @param size_fifo size of the fifo in bytes
 * @param basetype_size size of the fifo basetype eg: sizeof(uint8_t)
 * @retval 0 = success
 * @retval -1 = NULL Pointer as Parameter
 * @retval -2 = invalid fifo size
 * @retval -3 = invalid basetype_size
 */
int8_t fifo_init(volatile fifo_handle_t *pHandle, void *pFifo, FIFO_INDEX_TYPE size_fifo, uint8_t basetype_size);

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

#endif  // _FIFO_H_