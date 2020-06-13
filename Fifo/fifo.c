// *** INCLUDES ***
#include "fifo.h"
#include <string.h> // memcpy 
#include <assert.h>

// *** DEFINES ***

/*
#define _READ_LOCKED 0x01
#define _WRITE_LOCKED 0x02

static uint8_t _isReadLocked(fifo_handle_t *pHandle)
{
    return pHandle->_lock & _READ_LOCKED;
}
static uint8_t _isWriteLocked(fifo_handle_t *pHandle)
{
    return pHandle->_lock & _WRITE_LOCKED;
}
static void _read_lock(fifo_handle_t *pHandle)
{
    pHandle->_lock |= _READ_LOCKED;
}
static void _write_lock(fifo_handle_t *pHandle)
{
    pHandle->_lock |= _WRITE_LOCKED;
}
static void _write_unlock(fifo_handle_t *pHandle)
{
    pHandle->_lock &= ~_WRITE_LOCKED;
}
static void _read_unlock(fifo_handle_t *pHandle)
{
    pHandle->_lock &= ~_READ_LOCKED;
}
*/

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
int8_t fifo_init(fifo_handle_t *pHandle, void *pFifo, FIFO_INDEX_TYPE size_fifo, uint8_t basetype_size)
{
#ifdef _DEBUG
    assert(pHandle);
    assert(pFifo);
    assert(size_fifo <= MAX_FIFO_SIZE && size_fifo > 0);
    assert(basetype_size > 0 && basetype_size <= FIFO_MAX_BASETYPE_SIZE);
#endif

    // *** Checking Parameters ***
    if (pHandle == NULL || pFifo == NULL)
        return -1;
    if (size_fifo > MAX_FIFO_SIZE || size_fifo == 0)
        return -2;
    if (basetype_size == 0 || basetype_size > FIFO_MAX_BASETYPE_SIZE)
        return -3;

    // *** Initialize Handle ***
    pHandle->size = size_fifo;
    pHandle->basetype_size = basetype_size;
    pHandle->pFifo = pFifo;
    pHandle->read_idx = 0;
    pHandle->write_idx = 0;
    return 0;
}

/**
 * @brief puts an element into the fifo.
 * @note If _DEBUG is defined every Parameter will be checked with assert()
 * @param pHandle pointer to the fifo handle
 * @param [in] pData pointer to the data to be put onto the fifo
 * @return fifoerror_t
 */
fifoerror_t fifo_put(fifo_handle_t *pHandle, const void *pData)
{
    fifoerror_t ret;
#ifdef _DEBUG
    assert(pHandle);
    assert(pData);
#endif
    // *** Checking Parameters ***
    if (pHandle == NULL)
        return FIFO_WRONG_PARAM;
    if (pData == NULL)
        return FIFO_WRONG_PARAM;
FIFO_ENTER_CRITICAL();
    // *** Ring ***
    FIFO_INDEX_TYPE idx_temp = pHandle->write_idx + pHandle->basetype_size;

    if (idx_temp >= pHandle->size)
    {
        idx_temp = 0;
    }

    // *** Check if space available ***
    if (idx_temp == pHandle->read_idx)  // No space
    {
        ret = FIFO_FULL;
    }
    else        // space available
    {
        // *** Write to the fifo ***
        memcpy(((uint8_t *)(pHandle->pFifo) + (pHandle->write_idx = idx_temp)), pData, pHandle->basetype_size);
        ret = FIFO_NO_ERROR;
    }
FIFO_LEAVE_CRITICAL();
    return ret;
}

/**
 * @brief puts an element into the fifo.
 * @note If _DEBUG is defined every Parameter will be checked with assert()
 * @param pHandle pointer to the handle of the fifo
 * @param [out] pData pointer to the storage for the data from the fifo
 * @return fifoerror_t
 */
fifoerror_t fifo_get(fifo_handle_t *pHandle, void *pData)
{
#ifdef _DEBUG
    assert(pHandle);
    assert(pData);
#endif
    // *** Checking Parameters ***
    if (pHandle == NULL)
        return FIFO_WRONG_PARAM;
    if (pData == NULL)
        return FIFO_WRONG_PARAM;

    FIFO_INDEX_TYPE idx_temp;

FIFO_ENTER_CRITICAL();
    // *** Check if data available ***
    if (pHandle->write_idx == pHandle->read_idx)  // no data in fifo
    {
        return FIFO_EMPTY;
    }

    // *** Ring ***
    idx_temp = pHandle->read_idx + pHandle->basetype_size;

    if (idx_temp >= pHandle->size) 
    {
        idx_temp = 0;
    }

    // *** Copty the data ***
    memcpy(pData, ((uint8_t *)(pHandle->pFifo)) + (pHandle->read_idx = idx_temp), pHandle->basetype_size);
FIFO_LEAVE_CRITICAL();
    return FIFO_NO_ERROR;
}
