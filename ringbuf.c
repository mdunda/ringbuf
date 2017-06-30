/*
 * MIT License
 *
 * Copyright (c) 2017 Matthias Dunda
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include "ringbuf.h"

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <pthread.h>
#include <string.h>

#if 1
#define LOGPTR(ptr)             printf( "%s=%lx\n", #ptr, (unsigned long)ptr)
#else
#define LOGPTR(ptr)             (void)0
#endif

typedef struct ringbuf_element_s
{
    void *data;
    struct ringbuf_element_s *prev;
    struct ringbuf_element_s *next;
} ringbuf_element_t;


typedef struct ringbuf_s
{
    size_t element_size;
    size_t capacity;
    size_t fill;
    pthread_mutex_t mutex;
    struct ringbuf_element_s *wtptr;
    struct ringbuf_element_s *rdptr;
} ringbuf_t;


static ringbuf_element_t *ringbuf_allocate_element( size_t data_sz )
{
    ringbuf_element_t *new_elem =
        (ringbuf_element_t *)malloc( sizeof( ringbuf_element_t ) );

    if ( new_elem != NULL )
    {
        new_elem->data = malloc( data_sz );
        if ( new_elem->data == NULL )
        {
            free( new_elem );
            new_elem = NULL;
        }
    }

    return new_elem;
}


ringbuf_handle_t ringbuf_init( size_t count, size_t data_sz )
{
    ringbuf_t *result = NULL;

    if ( count > 0 )
    {
        result = (ringbuf_t *)malloc( sizeof( ringbuf_t ) );

        if ( result != NULL )
        {
            ringbuf_element_t *first = ringbuf_allocate_element( data_sz );
            ringbuf_element_t *current = first;
            size_t i = 1;

            LOGPTR(first);

            if ( first != NULL )
            {
                first->prev = first;
                first->next = first;

                while ( i < count )
                {
                    LOGPTR(current);
                    ringbuf_element_t *new_elem =
                        ringbuf_allocate_element( data_sz );
                    LOGPTR(new_elem);
                    if ( new_elem != NULL )
                    {
                        new_elem->next = first;
                        new_elem->prev = current;
                        current->next = new_elem;
                        first->prev = new_elem;
                        current = new_elem;
                        i++;
                    }
                    else
                    {
                        break;
                    }
                    LOGPTR(new_elem->prev);
                    LOGPTR(new_elem->next);
                }
                LOGPTR(first->prev);
                LOGPTR(first->next);
            }

            result->capacity = i;
            result->fill = 0;
            pthread_mutex_init( &result->mutex, NULL );
            result->element_size = data_sz;
            result->wtptr = first;
            result->rdptr = first;
        }
    }

    return (ringbuf_handle_t)result;
}


int ringbuf_send( ringbuf_handle_t ringbuf, const void *data )
{
    ringbuf_t *hdl = (ringbuf_t *)ringbuf;

    if ( hdl == NULL || data == NULL )
    {
        return -ENOMEM;
    }

    pthread_mutex_lock( &hdl->mutex );

    LOGPTR(hdl->wtptr);
    memcpy( hdl->wtptr->data, data, hdl->element_size );

    hdl->wtptr = hdl->wtptr->next;

    if ( hdl->fill < hdl->capacity )
    {
        hdl->fill++;
    }
    else
    {
        hdl->rdptr = hdl->rdptr->next;
    }

    pthread_mutex_unlock( &hdl->mutex );

    return 0;
}


int ringbuf_receive( ringbuf_handle_t ringbuf, void *data )
{
    ringbuf_t *hdl = (ringbuf_t *)ringbuf;
    int result;

    if ( hdl == NULL || data == NULL )
    {
        return -ENOMEM;
    }

    pthread_mutex_lock( &hdl->mutex );

    if ( hdl->fill > 0 )
    {
        hdl->fill--;
        LOGPTR(hdl->rdptr);
        memcpy( data, hdl->rdptr->data, hdl->element_size );

        hdl->rdptr = hdl->rdptr->next;

        result = 0;
    }
    else
    {
        result = -ENOENT;
    }

    pthread_mutex_unlock( &hdl->mutex );

    return result;
}


size_t ringbuf_getfill( ringbuf_handle_t ringbuf )
{
    size_t result = 0;
    ringbuf_t *hdl = (ringbuf_t *)ringbuf;

    if ( hdl != NULL )
    {
        pthread_mutex_lock( &hdl->mutex );
        result = hdl->fill;
        pthread_mutex_unlock( &hdl->mutex );
    }

    return result;
}


void ringbuf_flush( ringbuf_handle_t ringbuf )
{
    ringbuf_t *hdl = (ringbuf_t *)ringbuf;

    if ( hdl != NULL )
    {
        pthread_mutex_lock( &hdl->mutex );
        hdl->wtptr = hdl->rdptr;
        hdl->fill = 0;
        pthread_mutex_unlock( &hdl->mutex );
    }
}


size_t ringbuf_getsize( ringbuf_handle_t ringbuf )
{
    ringbuf_t *hdl = (ringbuf_t *)ringbuf;
    size_t result = 0;

    if ( hdl != NULL )
    {
        pthread_mutex_lock( &hdl->mutex );
        result = hdl->capacity;
        pthread_mutex_unlock( &hdl->mutex );
    }

    return result;
}

