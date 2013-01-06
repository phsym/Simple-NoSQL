/*
* md5.h
* Implementation of the md5 algorithm described in RFC1321
* Copyright (C) 2005 Quentin Carbonneaux <crazyjoke@free.fr>
* Modified by Pierre-Henri Symoneaux
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 US
*/

#ifndef MD5_H_
#define MD5_H_

#include <assert.h>
#include <stdlib.h>

/* WARNING :
 * This implementation is using 32 bits long values for sizes
 */

#define MD5_DIGEST_STR_LENGTH 33 // 32 + 1 for the '\0' character
#define MD5_DIGEST_LENGTH 16

typedef unsigned int md5_size;

/* MD5 context */
struct md5_ctx {
        struct {
                unsigned int A, B, C, D; /* registers */
        } regs;
        unsigned char *buf;
        md5_size size;
        md5_size bits;
};

/* Size of the MD5 buffer */
#define MD5_BUFFER 1024

/* Basic md5 functions */
#define F(x,y,z) ((x & y) | (~x & z))
#define G(x,y,z) ((x & z) | (~z & y))
#define H(x,y,z) (x ^ y ^ z)
#define I(x,y,z) (y ^ (x | ~z))

/* Rotate left 32 bits values (words) */
#define ROTATE_LEFT(w,s) ((w << s) | ((w & 0xFFFFFFFF) >> (32 - s)))

#define FF(a,b,c,d,x,s,t) (a = b + ROTATE_LEFT((a + F(b,c,d) + x + t), s))
#define GG(a,b,c,d,x,s,t) (a = b + ROTATE_LEFT((a + G(b,c,d) + x + t), s))
#define HH(a,b,c,d,x,s,t) (a = b + ROTATE_LEFT((a + H(b,c,d) + x + t), s))
#define II(a,b,c,d,x,s,t) (a = b + ROTATE_LEFT((a + I(b,c,d) + x + t), s))

unsigned char *md5 (unsigned char *, md5_size, unsigned char *);
void md5_init (struct md5_ctx *);
void md5_update (struct md5_ctx *context);
void md5_final (unsigned char *digest, struct md5_ctx *context);

void md5_to_str(unsigned char *d, char* str);
void md5_str(char *M, md5_size len, char* digest_str);

#endif /* MD5_H_ */
