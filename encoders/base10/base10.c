#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <limits.h>
#include <stdint.h>

#include "crypto_scrypt-hexconvert.h"

int libscrypt_b10_encode(src, srclength, target, targsize)
    unsigned char const *src;
    size_t srclength;
    char *target;
    size_t targsize;
{
    size_t i;
    int len = 0;

    if (!src || srclength < 1 || targsize < (srclength * 2 + 1))
        return -1;

    memset(target, 0, targsize);

    for(i=0; i<=(srclength-1); i++)
    {
        /* snprintf(target, srclength,"%s...", target....) has undefined results
         * and can't be used. Using offests like this makes snprintf
         * nontrivial. we therefore have use inescure sprintf() and
         * lengths checked elsewhere (start of function) */
        /*@ -bufferoverflowhigh @*/
        len += sprintf(target+len, "%02u", (unsigned int) src[i]);
    }

    return 0;

    /*if (libscrypt_hex_encode(src, srclength, target, targsize) == -1) return -1;*/
    /*char *p;*/
    /*errno = 0;*/
    /*unsigned long int b10 = strtoul(target,&p,16);*/

    /*if (p == target) //no digits found*/
        /*return -1;*/
    /*else if ((b10 == LONG_MIN || b10 == ULONG_MAX) && errno == ERANGE)*/
        /*//value out of range*/
        /*return -1;*/
    /*else*/
        /*snprintf(target, targsize, "%ld", b10);*/

    /*return 0;*/
}
