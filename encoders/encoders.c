#include <string.h>

#include "encoders.h"

int base91_glue_encode(unsigned char const *src, size_t srclength,
                       void *target, size_t targsize) {
    static struct basE91 b91;
    size_t s;
    int total_size = 0;

    // zero the output buffer
    memset(target,'\0',targsize);

    // setup the base91 struct
    basE91_init(&b91);

    // encode most of it and keep the size
    s = basE91_encode(&b91, src, srclength, target);
    total_size += s;

    // pickup anything left and keep the size
    s = basE91_encode_end(&b91, (void*)(target+s));
    total_size += s;

    // return the total size
    return total_size;
}