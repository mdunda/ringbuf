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

#include <stdio.h>
#include <stdlib.h>

#include "ringbuf.h"

struct dummy_data_s
{
    int a;
    int b;
    int c;
};

int main()
{
    struct dummy_data_s d;
    ringbuf_handle_t *rb;
    size_t sz, i;
    int j;

    rb = ringbuf_init( 10, sizeof( struct dummy_data_s ) );
    printf( "%lu\n", (unsigned long)rb );
    sz = ringbuf_getsize( rb );
    printf( "Ringbuffer size is %lu\n", sz );

    for ( j = 3; j <= 16; j++ )
    {
        printf( "\nNew iteration, fill=%lu"
                "  -------------------------------\n", ringbuf_getfill( rb ) );
        for ( i = 0; i < j; i++ )
        {
            d.a = i;
            d.b = 2 * i;
            d.c = 3 * i;
            ringbuf_send( rb, &d );
            printf( "%lu. sent (%d,%d,%d) fill=%lu\n",
                    i + 1, d.a, d.b, d.c,
                    ringbuf_getfill( rb ) );
            if ( i == 5 )
                ringbuf_flush( rb );
        }

        i = 0;
        while ( ringbuf_receive( rb, &d ) == 0 )
        {
            printf( "%lu. received (%d,%d,%d) fill=%lu\n",
                    ++i, d.a, d.b, d.c,
                    ringbuf_getfill( rb ) );
        }
    }

    return EXIT_SUCCESS;
}

