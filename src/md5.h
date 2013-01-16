#ifndef MD5_H_
#define MD5_H_

#include <assert.h>
#include <stdlib.h>

/* WARNING :
 * This implementation is using 32 bits long values for sizes
 */

#define MD5_DIGEST_STR_LENGTH 33 // 32 + 1 for the '\0' character
#define MD5_DIGEST_LENGTH 16

#include <stdint.h>

/*  The following tests optimise behaviour on little-endian
    machines, where there is no need to reverse the byte order
    of 32 bit words in the MD5 computation.  By default,
    HIGHFIRST is defined, which indicates we're running on a
    big-endian (most significant byte first) machine, on which
    the byteReverse function in md5.c must be invoked. However,
    byteReverse is coded in such a way that it is an identity
    function when run on a little-endian machine, so calling it
    on such a platform causes no harm apart from wasting time. 
    If the platform is known to be little-endian, we speed
    things up by undefining HIGHFIRST, which defines
    byteReverse as a null macro.  Doing things in this manner
    insures we work on new platforms regardless of their byte
    order.  */

#define HIGHFIRST

#if defined(__i386__) || defined(__x86_64__) || defined(__amd64__)
#undef HIGHFIRST
#endif

/*  On machines where "long" is 64 bits, we need to declare
    uint32 as something guaranteed to be 32 bits.  */

typedef uint32_t uint32;

struct MD5Context {
        uint32 buf[4];
        uint32 bits[2];
        unsigned char in[64];
};

void MD5Init(struct MD5Context *ctx);
void MD5Update(struct MD5Context *ctx, unsigned char *buf, uint32_t len);
void MD5Final(unsigned char *digest, struct MD5Context *ctx);
void MD5Transform(uint32 *buf, uint32 *in);

/*
 * This is needed to make RSAREF happy on some MS-DOS compilers.
 */
typedef struct MD5Context MD5_CTX;

unsigned char* MD5(unsigned char * message, uint32_t len, unsigned char * digest);

void MD5_to_str(unsigned char *d, char* str);
void MD5_str(char *M, uint32_t len, char* digest_str);

#endif /* MD5_H_ */
