/* public domain sha256 implementation based on fips180-3 */

#ifndef SHA256_H_
#define SHA256_H_

#include <stdint.h>

struct sha256 {
	uint64_t len;    /* processed message length */
	uint32_t h[8];   /* hash state */
	uint8_t buf[64]; /* message block buffer */
};

/* reset state */
void sha256_init(struct sha256 *s);
/* process message */
void sha256_update(struct sha256 *s, const void *m, uint32_t len);
/* get message digest */
/* state is ruined after sum, keep a copy if multiple sum is needed */
/* part of the message might be left in s, zero it if secrecy is needed */
void sha256_sum(struct sha256 *s, uint8_t md[32]);

/* openssl api */
#define SHA256_CTX struct sha256
#define SHA256_DIGEST_LENGTH 32
#define SHA256_DIGEST_STR_LENGTH 65 // 64 + 1 for the '\0' character

unsigned char *SHA256(const unsigned char *m, uint32_t len, unsigned char *md);

static inline int SHA256_Init(SHA256_CTX *s) {sha256_init(s); return 1;}
static inline int SHA256_Update(SHA256_CTX *s, const void *m, uint32_t len) {sha256_update(s, m, len); return 1;}
static inline int SHA256_Final(unsigned char *md, SHA256_CTX *s) {sha256_sum(s, md); return 1;}

#endif /* SHA256_H_ */
