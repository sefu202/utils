// *** INCLUDES ***
#include "fifo.h"
#include <string.h> // memcpy 
#ifdef _DEBUG
    #include <assert.h>
#endif
#if FIFO_ALLOW_MALLOC == true
    #include <malloc.h>
#endif /* FIFO_ALLOW_MALLOC */

// *** DEFINES ***
#define _WRITE_LOCK 0x01
#define _READ_LOCK  0x02

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
int8_t fifo_init(volatile fifo_handle_t *pHandle, void *pFifo, FIFO_INDEX_TYPE size_fifo, SIZE_FIFO_BASE_TYPE basetype_size)
{
#ifdef _DEBUG
    assert(pHandle != NULL);
    assert(pFifo != NULL);
    assert(size_fifo <= MAX_FIFO_SIZE && size_fifo > 0);
    assert(basetype_size > 0 && basetype_size <= FIFO_MAX_BASETYPE_SIZE);
    assert(size_fifo % basetype_size == 0);
#endif

    // *** Checking Parameters ***
    if (pHandle == NULL || pFifo == NULL)
        return -1;
    if (size_fifo > MAX_FIFO_SIZE || size_fifo == 0)
        return -2;
    if (basetype_size == 0 || basetype_size > FIFO_MAX_BASETYPE_SIZE)
        return -3;
    if (size_fifo % basetype_size != 0)
        return -4;

    // *** Initialize Handle ***
    pHandle->size = size_fifo;
    pHandle->basetype_size = basetype_size;
    pHandle->pFifo = pFifo;
    pHandle->read_idx = 0;
    pHandle->write_idx = 0;
    pHandle->_lock = 0;
    return 0;
}

#if FIFO_ALLOW_MALLOC
/**
 * @brief allocates a fifo handle and the fifo memory, and initializes it
 * @note If _DEBUG is defined every Parameter will be checked with assert()
 * @note memory has to be freed with fifo_deinit_free()
 * @param size_fifo size of the fifo in elements
 * @param basetype_size size of the fifo basetype eg: sizeof(uint8_t)
 * @retval NULL = failed, may be a size_fifo 0 or bigger than MAX_FIFO_SIZE or basetype_size 0 or bigger than FIFO_MAX_BASETYPE_SIZE
 * @return pointer to the fifo handle
 */
fifo_handle_t* fifo_init_malloc(FIFO_INDEX_TYPE size_fifo, SIZE_FIFO_BASE_TYPE basetype_size)
{
 #ifdef _DEBUG
    assert(size_fifo <= MAX_FIFO_SIZE && size_fifo > 0);
    assert(basetype_size > 0 && basetype_size <= FIFO_MAX_BASETYPE_SIZE);
#endif
    // *** Checking Parameters ***
    if (size_fifo > MAX_FIFO_SIZE || size_fifo == 0)
        return NULL;
    if (basetype_size == 0 || basetype_size > FIFO_MAX_BASETYPE_SIZE)
        return NULL;
    
    fifo_handle_t *myHandle = NULL;

    // *** Allocate Handle ***
    myHandle = (fifo_handle_t *)malloc(sizeof(*myHandle));

    if (myHandle != NULL)
    {
        // *** Allocate Buffer ***
        myHandle->pFifo = malloc(size_fifo * basetype_size);
        if (myHandle->pFifo != NULL)
        {
            // *** Initialize Handle ***
            myHandle->size = size_fifo * basetype_size; // size fifo is in elements and size is in bytes
            myHandle->basetype_size = basetype_size;
            myHandle->read_idx = 0;
            myHandle->write_idx = 0;
            myHandle->_lock = 0;
        }
        else     // buffer allocation failed
        {
            free((void *)myHandle);     // therefore free previously allocated memory
            myHandle = NULL;
        }
    }
    return myHandle;
}

/**
 * @brief deallocates a fifo handle and its buffer
 * @note if _DEBUG is defined pHandle gets checked with assert()
 * @param pHandle pointer to the Fifo handle
 */
void fifo_deinit_free(volatile fifo_handle_t *pHandle)
{
#ifdef _DEBUG
    assert(pHandle != NULL);
#endif
    if (pHandle->pFifo != NULL)
    {
        free(pHandle->pFifo);
    }
    if (pHandle != NULL)
    {
        free((void *)pHandle);
    }
}
#endif  /* FIFO_ALLOW_MALLOC */

/**
 * @brief puts an element into the fifo.
 * @note If _DEBUG is defined every Parameter will be checked with assert()
 * @param pHandle pointer to the fifo handle
 * @param [in] pData pointer to the data to be put onto the fifo
 * @return fifoerror_t
 */
fifoerror_t fifo_put(volatile fifo_handle_t *pHandle, const void *pData)
{
    fifoerror_t ret = FIFO_BUISY;
#ifdef _DEBUG
    assert(pHandle != NULL);
    assert(pData != NULL);
#endif
    // *** Checking Parameters ***
    if (pHandle == NULL)
        return FIFO_WRONG_PARAM;
    if (pData == NULL)
        return FIFO_WRONG_PARAM;


    if (!(pHandle->_lock & _WRITE_LOCK))   // fifo is write-locked
    {
    FIFO_ENTER_CRITICAL();  // writing to lock may not be a atomic operation
        pHandle->_lock |= _WRITE_LOCK;      // lock the handle

        // *** Ring ***
        FIFO_INDEX_TYPE idx_temp = pHandle->write_idx + pHandle->basetype_size, read_idx = pHandle->read_idx;
    FIFO_LEAVE_CRITICAL();

        if (idx_temp >= pHandle->size)
        {
            idx_temp = 0;
        }

        // *** Check if space available ***
        if (idx_temp == read_idx)  // No space
        {
            ret = FIFO_FULL;
        }
        else        // space available
        {
            // *** Write to the fifo ***
            memcpy(((uint8_t *)(pHandle->pFifo) + (pHandle->write_idx = idx_temp)), pData, pHandle->basetype_size);
            ret = FIFO_NO_ERROR;
        }
    FIFO_ENTER_CRITICAL();
        pHandle->_lock &= ~_WRITE_LOCK;     // unlock the handle
    FIFO_LEAVE_CRITICAL();
    }
    return ret;
}

/**
 * @brief puts an element into the fifo.
 * @note If _DEBUG is defined every Parameter will be checked with assert()
 * @param pHandle pointer to the handle of the fifo
 * @param [out] pData pointer to the storage for the data from the fifo
 * @return fifoerror_t
 */
fifoerror_t fifo_get(volatile fifo_handle_t *pHandle, void *pData)
{
#ifdef _DEBUG
    assert(pHandle != NULL);
    assert(pData != NULL);
#endif
    // *** Checking Parameters ***
    if (pHandle == NULL)
        return FIFO_WRONG_PARAM;
    if (pData == NULL)
        return FIFO_WRONG_PARAM;

    FIFO_INDEX_TYPE idx_temp, write_idx;
    fifoerror_t ret = FIFO_BUISY;

    if (!(pHandle->_lock & _READ_LOCK))   // fifo is read-locked
    {
    FIFO_ENTER_CRITICAL();
        pHandle->_lock |= _READ_LOCK;      // lock the handle
        write_idx = pHandle->write_idx;     // looking at write_idx may not be a atomic operation
    FIFO_LEAVE_CRITICAL();
        // *** Check if data available ***
        if (write_idx != pHandle->read_idx)  // no data in fifo
        {
            ret = FIFO_NO_ERROR;
            // *** Ring ***
            idx_temp = pHandle->read_idx + pHandle->basetype_size; 

            if (idx_temp >= pHandle->size) 
            {
                idx_temp = 0;
            }

            // *** Copty the data ***
            memcpy(pData, ((uint8_t *)(pHandle->pFifo)) + (pHandle->read_idx = idx_temp), pHandle->basetype_size);  
        }
        else
        {
            ret = FIFO_EMPTY;
        }
    FIFO_ENTER_CRITICAL();
        pHandle->_lock &= ~_READ_LOCK;     // unlock the handle
    FIFO_LEAVE_CRITICAL();
    }

    return ret;
}


/**
 * @brief checks if a fifo still has elements in it
 * @note pHandle gets checked with assert() when _DEBUG is defined
 * @retval true = has elements left
 */
bool fifo_hasElementsLeft(volatile fifo_handle_t *pHandle)
{
#ifdef _DEBUG
    assert(pHandle != NULL);
#endif
    bool ret = false;
    // *** Check if data available ***
FIFO_ENTER_CRITICAL();
    if (!(pHandle == NULL || pHandle->write_idx == pHandle->read_idx))  // no data in fifo
    {
        ret = true;
    }
FIFO_LEAVE_CRITICAL();
    return ret;
}

/**
 * @brief checks if a fifo still free space in it
 * @note pHandle gets checked with assert() when _DEBUG is defined
 * @retval true = has free space left
 */
bool fifo_hasSpaceLeft(volatile fifo_handle_t *pHandle)
{
#ifdef _DEBUG
    assert(pHandle != NULL);
#endif
    bool ret = false;
    // *** Checking Parameters ***
    if (pHandle != NULL)
    {
    FIFO_ENTER_CRITICAL();    
        FIFO_INDEX_TYPE idx_temp = pHandle->write_idx + pHandle->basetype_size;
        if (idx_temp >= pHandle->size)
        {
            idx_temp = 0;
        }

        // *** Check if space available ***
        if (idx_temp != pHandle->read_idx)  // No space
        {
            ret = true;
        }
    FIFO_LEAVE_CRITICAL();
    }
    return ret;
}

/**
 * @brief flushes a FIFO by setting write- and read-index to the same value
 * @return FIFO_NO_ERROR    Everything worked
 * @return FIFO_BUISY       FIFO handle is locked
 */
fifoerror_t fifo_flush(volatile fifo_handle_t *pHandle)
{
#ifdef _DEBUG
    assert(pHandle != NULL);
#endif
    if (pHandle == NULL)
    {
        return FIFO_WRONG_PARAM;
    }
    fifoerror_t ret = FIFO_NO_ERROR;

FIFO_ENTER_CRITICAL();
    if (pHandle->_lock & (_READ_LOCK | _WRITE_LOCK))  // fifo is locked
    {
        ret = FIFO_BUISY;
    }
    pHandle->read_idx = pHandle->write_idx; // flush
FIFO_LEAVE_CRITICAL();
    return ret;
}

/**
 * @brief returns the pointer to the end of the fifo buffer
 * @param pHandle pointer to the fifo handle
 * @retval pointer to the end of the fifo
 */
void *fifo_getEndPtr(fifo_handle_t *pHandle)
{
    return (uint8_t *)pHandle->pFifo + (pHandle->size - pHandle->basetype_size);
}

/**
 * @brief acts as if the fifo had been read
 * This may be useful to read from the fifo outside of this library, for example if a function gets passed a fifo pointer
 * and it reads multiple times from the fifo without calling a fifo function
 * @note Be careful when reading from the fifo without fifo functions!!
 * @param pHandle pointer to the fifo handle
 * @param n ammount of read cycles to be skipped
 * @return fifoerror_t
 */
fifoerror_t fifo_skip_read(fifo_handle_t *pHandle)
{
    fifo_skip_read_n(pHandle, 1);
}

/**
 * @brief acts as if the fifo had been read n times
 * This may be useful to read from the fifo outside of this library, for example if a function gets passed a fifo pointer
 * and it reads multiple times from the fifo without calling a fifo function
 * @note Be careful when reading from the fifo without fifo functions!!
 * @param pHandle pointer to the fifo handle
 * @param n ammount of read cycles to be skipped
 * @return fifoerror_t
 */
fifoerror_t fifo_skip_read_n(fifo_handle_t *pHandle, FIFO_INDEX_TYPE n)
{
#ifdef _DEBUG
    assert(pHandle != NULL);
#endif
    // *** Checking Parameters ***
    if (pHandle == NULL)
        return FIFO_WRONG_PARAM;

    FIFO_INDEX_TYPE idx_temp;

FIFO_ENTER_CRITICAL();
    if (pHandle->_lock & _READ_LOCK)   // fifo is read-locked
    {
        FIFO_LEAVE_CRITICAL();
        return FIFO_BUISY;
    }

    // *** Check if data available ***
    if (pHandle->write_idx == pHandle->read_idx)  // no data in fifo
    {
        FIFO_LEAVE_CRITICAL();
        return FIFO_EMPTY;
    }

    // *** Limit skipped elements to available elements ***
    FIFO_INDEX_TYPE level = fifo_getLevel(pHandle);
    if (n > level)
    {
        n = level;
    }

    // *** Ring ***
    idx_temp = pHandle->read_idx + pHandle->basetype_size * n;

    if (idx_temp >= pHandle->size) 
    {
        idx_temp -= pHandle->size;
    }
    pHandle->read_idx = idx_temp;
FIFO_LEAVE_CRITICAL();

    return FIFO_NO_ERROR;
}

/**
 * @brief acts as if something had been written to the fifo without changing its contents
 * This may be useful to write to the fifo outside of this library, for example if a function gets passed a fifo pointer
 * and it writes multiple times to the fifo without calling a fifo function
 * @note Be careful when writing to the fifo without fifo functions!!
 * @param pHandle pointer to the fifo handle
 * @return fifoerror_t
 */
fifoerror_t fifo_skip_write(fifo_handle_t *pHandle)
{
    return fifo_skip_write_n(pHandle, 1);
}

/**
 * @brief acts as if the fifo had been written to n times without changing its contents on the physical memory
 * This may be useful to write to the fifo outside of this library, for example if a function gets passed a fifo pointer
 * and it writes multiple times to the fifo without calling a fifo function
 * @note Be careful when writing to the fifo without fifo functions!!
 * @param pHandle pointer to the fifo handle
 * @param n ammount of write cycles to be skipped
 * @return fifoerror_t
 */
fifoerror_t fifo_skip_write_n(fifo_handle_t *pHandle, FIFO_INDEX_TYPE n)
{
        fifoerror_t ret;
#ifdef _DEBUG
    assert(pHandle != NULL);
#endif
    // *** Checking Parameters ***
    if (pHandle == NULL)
        return FIFO_WRONG_PARAM;
    if (pHandle == 0)
        return FIFO_WRONG_PARAM;

FIFO_ENTER_CRITICAL();
    if (pHandle->_lock & _WRITE_LOCK)   // fifo is write-locked
    {
        FIFO_LEAVE_CRITICAL();
        return FIFO_BUISY;
    }

    // *** Limit skipped elements to available elements ***
    FIFO_INDEX_TYPE space = fifo_getEmptySpace(pHandle);
    if (n > space && space > 0)
    {
        n = space;
    }

    // *** Ring ***
    FIFO_INDEX_TYPE idx_temp = pHandle->write_idx + pHandle->basetype_size * n;

    if (idx_temp >= pHandle->size)
    {
        idx_temp -= pHandle->size;
    }

    // *** Check if space available ***
    if (idx_temp == pHandle->read_idx)  // No space
    {
        ret = FIFO_FULL;
    }
    else
    {
        ret = FIFO_NO_ERROR;
        pHandle->write_idx = idx_temp;
    }
FIFO_LEAVE_CRITICAL();
    return ret;
}

/**
 * @brief returns the fill level of a fifo
 * @param pHandle pointer to a fifo handle
 * @retval fill level of the fifo in elements
 */
FIFO_INDEX_TYPE fifo_getLevel(fifo_handle_t *pHandle)
{
#ifdef _DEBUG
    assert(pHandle != NULL);
#endif
    if (pHandle == NULL)
    {
        return 0;
    }
FIFO_ENTER_CRITICAL();
    FIFO_INDEX_TYPE ret = pHandle->write_idx, read_idx = pHandle->read_idx;
FIFO_LEAVE_CRITICAL();
    if (read_idx > ret) // ret alias write_idx
    {
        ret += (pHandle->size - read_idx);
    }
    else    // because ret may overflow if this is not done in if / else
    {
        ret -= read_idx;
    }
    return ret / pHandle->basetype_size;
}

/**
 * @brief returns the empty space of a fifo
 * @param pHandle pointer to a fifo handle
 * @retval empty space in the fifo in elements
 */
FIFO_INDEX_TYPE fifo_getEmptySpace(fifo_handle_t *pHandle)
{
#ifdef _DEBUG
    assert(pHandle != NULL);
#endif
    if (pHandle == NULL)
    {
        return 0;
    }
    return ((pHandle->size / pHandle->basetype_size) - fifo_getLevel(pHandle)) -1; // maximum fifo fill is size / basetype -1
}