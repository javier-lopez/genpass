/* BASE64 libraries used internally - should not need to be packaged */
#include <stddef.h>
#define b64_encode_len(A) ((A+2)/3 * 4 + 1)
#define b64_decode_len(A) (A / 4 * 3 + 2)

//modified version, $_ instead of +/ and without '=' padding, optimized
//for generating passwords
int	libscrypt_b64_encode(unsigned char const *src, size_t srclength,
        /*@out@*/ char *target, size_t targetsize);

int	libscrypt_b64_encode_compliant(unsigned char const *src, size_t srclength,
        /*@out@*/ char *target, size_t targetsize);
int	libscrypt_b64_decode_compliant(char const *src,
        /*@out@*/ unsigned char *target, size_t targetsize);
