/**
 * @brief this file contains a c++ template wrapper class for the c fifo library
 * @author Josef Aschwanden
 * @date 29.07.2020
 * @version 1.0
 */
// *** INCLUDES ***
#include "fifo.h"
#include <string>

#pragma once

namespace utils{
    template<typename T>
    class Fifo{
    public:

        /**
         * @brief constructr: creates a fifo of the template typ with space for size elements
         */
        Fifo(size_t size)
        {
            m_pHandle = fifo_init_malloc(size, sizeof(T));
            m_error = FIFO_NO_ERROR;
        }

        /**
         * @brief destructor
         */
        ~Fifo()
        {
            fifo_deinit_free(m_pHandle);
        }

        /**
         * @brief puts one element into the fifo
         * @retval 0 = success
         * @retval -1 = fail
         */
        int put(const T& data)
        {
            if ((m_error = fifo_put(m_pHandle, std::addressof(data))) == FIFO_NO_ERROR)
                return 0;
            else
                return -1;
        }

        /**
         * @brief gets one element from the fifo
         * @note element adress gets passed by reference
         * @retval 0 = success
         * @retval -1 = fail
         */
        int get(T& data)
        {
            if ((m_error = fifo_get(m_pHandle, std::addressof(data))) == FIFO_NO_ERROR)
                return 0;
            else
                return -1;
        }

        /**
         * @brief returns error enum value of the last operation
         */
        fifoerror_t getError() const
        {
            return m_error;
        }

        /**
         * @brief gets the error string of the last operation
         */
        std::string getErrorStr() const
        {
            std::string ret;
            switch (m_error)
            {
                case FIFO_NO_ERROR: ret = "No Error"; break;
                case FIFO_WRONG_PARAM: ret = "Wrong Parameter"; break;
                case FIFO_BUISY: ret = "Fifo is Buisy"; break;
                case FIFO_EMPTY: ret = "Fifo is empty"; break;
                case FIFO_FULL: ret = "Fifo is full"; break;
            }
            return ret;
        }
        
        /**
         * @brief checks if elements are in the fifo
         */
        bool hasElementsLeft()
        {
            return fifo_hasElementsLeft(m_pHandle);
        }

        /**
         * @brief checks if space is left in the fifo
         */
        bool hasSpaceLeft()
        {
            return fifo_hasSpaceLeft(m_pHandle);
        }

        /**
         * @brief flushes a fifo
         * @retval 0 = success
         * @retval -1 = fail
         */
        int flush()
        {
            if ((m_error = fifo_flush(m_pHandle)) == FIFO_NO_ERROR)
                return 0;
            else 
                return -1;
        }

        /**
         * @brief returns fill level in elements
         */
        size_t getLevel()
        {
            return fifo_getLevel(m_pHandle);
        }

        /**
         * @brief returns empty space in elements
         */
        size_t getEmptySpace()
        {
            return fifo_getEmptySpace(m_pHandle);
        }

        /**
         * @brief skips one read cycle
         * @retval 0 = success
         * @retval -1 = fail
         */
        int skipRead()
        {
            if ((m_error = fifo_skip_read(m_pHandle)) == FIFO_NO_ERROR)
                return 0;
            else
                return -1;
        }

        /**
         * @brief skips n read cycles
         * @retval 0 = success
         * @retval -1 = fail
         */
        int skipRead(size_t n)
        {
            if ((m_error = fifo_skip_read_n(m_pHandle, n)) == FIFO_NO_ERROR)
                return 0;
            else
                return -1;
        }

        /**
         * @brief returns size in elements
         */
        size_t size() const
        {
            return m_pHandle->size / m_pHandle->basetype_size;
        }

    private:
        fifo_handle_t *m_pHandle;
        fifoerror_t m_error;
    };
}