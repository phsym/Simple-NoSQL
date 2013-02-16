/*
SHA-1 in C
By Steve Reid <steve@edmweb.com>
Modified by Pierre-Henri Symoneaux
100% Public Domain
*/

#ifndef SHA1_H_
#define SHA1_H_

#include <stdint.h>

#define SHA1_DIGEST_LENGTH 20
#define SHA1_DIGEST_STR_LENGTH 41 // 40 + 1 for the '\0' character

typedef struct {
    uint32_t state[5];
    uint32_t count[2];
    unsigned char buffer[64];
} SHA1_CTX;

void SHA1Transform(uint32_t state[5], const unsigned char buffer[64]);
void SHA1Init(SHA1_CTX* context);
void SHA1Update(SHA1_CTX* context, const unsigned char* data, uint32_t len);
void SHA1Final(unsigned char digest[20], SHA1_CTX* context);

unsigned char *SHA1 (const unsigned char * message, uint32_t len, unsigned char * digest);

#endif /* SHA1_H_ */
