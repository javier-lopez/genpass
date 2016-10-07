#include <stddef.h>
#include <stdint.h>

/**
 * Converts a binary string to a hex representation of that string
 * outbuf must have size of at least buf * 2 + 1.
 */
int libscrypt_hexconvert(const unsigned char * buf, size_t s, char *outbuf, size_t obs);
int libscrypt_hex_encode(const unsigned char * buf, size_t s, char *outbuf, size_t obs);
