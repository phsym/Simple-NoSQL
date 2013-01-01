/*
* Simple-NoSQL
* Copyright (C) 2012 Pierre-Henri Symoneaux
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

/*
 * md5.c
 *
 *  Created on: 1 janv. 2013
 *      Author: Pierre-Henri Symoneaux
 */

// MD5 implementation is an improved version of the one found at rosettacode.org

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "md5.h"

typedef union uwb {
    unsigned w;
    unsigned char b[4];
} WBunion;

typedef unsigned Digest[4];

unsigned f0(unsigned abcd[]) {
	return (abcd[1] & abcd[2]) | (~abcd[1] & abcd[3]);
}

unsigned f1(unsigned abcd[]) {
	return (abcd[3] & abcd[1]) | (~abcd[3] & abcd[2]);
}

unsigned f2(unsigned abcd[]) {
	return abcd[1] ^ abcd[2] ^ abcd[3];
}

unsigned f3(unsigned abcd[]) {
	return abcd[2] ^ (abcd[1] | ~abcd[3]);
}

typedef unsigned (*DgstFctn)(unsigned a[]);

unsigned k[] = { 0xd76aa478, 0xe8c7b756, 0x242070db, 0xc1bdceee, 0xf57c0faf,
		0x4787c62a, 0xa8304613, 0xfd469501, 0x698098d8, 0x8b44f7af, 0xffff5bb1,
		0x895cd7be, 0x6b901122, 0xfd987193, 0xa679438e, 0x49b40821, 0xf61e2562,
		0xc040b340, 0x265e5a51, 0xe9b6c7aa, 0xd62f105d, 0x02441453, 0xd8a1e681,
		0xe7d3fbc8, 0x21e1cde6, 0xc33707d6, 0xf4d50d87, 0x455a14ed, 0xa9e3e905,
		0xfcefa3f8, 0x676f02d9, 0x8d2a4c8a, 0xfffa3942, 0x8771f681, 0x6d9d6122,
		0xfde5380c, 0xa4beea44, 0x4bdecfa9, 0xf6bb4b60, 0xbebfbc70, 0x289b7ec6,
		0xeaa127fa, 0xd4ef3085, 0x04881d05, 0xd9d4d039, 0xe6db99e5, 0x1fa27cf8,
		0xc4ac5665, 0xf4292244, 0x432aff97, 0xab9423a7, 0xfc93a039, 0x655b59c3,
		0x8f0ccc92, 0xffeff47d, 0x85845dd1, 0x6fa87e4f, 0xfe2ce6e0, 0xa3014314,
		0x4e0811a1, 0xf7537e82, 0xbd3af235, 0x2ad7d2bb, 0xeb86d391 };

// ROtate v Left by amt bits
inline unsigned rol(unsigned v, short amt) {
	unsigned msk1 = (1 << amt) - 1;
	return ((v >> (32 - amt)) & msk1) | ((v << amt) & ~msk1);
}

unsigned *md5(const char *msg, int mlen) {
	static Digest h0 = { 0x67452301, 0xEFCDAB89, 0x98BADCFE, 0x10325476 };
	static DgstFctn ff[] = { &f0, &f1, &f2, &f3 };
	static short M[] = { 1, 5, 3, 7 };
	static short O[] = { 0, 1, 5, 0 };
	static short rot0[] = { 7, 12, 17, 22 };
	static short rot1[] = { 5, 9, 14, 20 };
	static short rot2[] = { 4, 11, 16, 23 };
	static short rot3[] = { 6, 10, 15, 21 };
	static short *rots[] = { rot0, rot1, rot2, rot3 };

	static Digest h;
	Digest abcd;
	DgstFctn fctn;
	short m, o, g;
	unsigned f;
	short *rotn;
	union {
		unsigned w[16];
		char b[64];
	} mm;
	int os = 0;
	int grp, q, p;
	int grps = 1 + (mlen + 8) / 64;
	unsigned char msg2[64 * grps];

	for (q = 0; q < 4; q++)	h[q] = h0[q]; // initialize
	{
		memcpy(msg2, msg, mlen);
		msg2[mlen] = (unsigned char) 0x80;
		q = mlen + 1;
		while (q < 64 * grps) {
			msg2[q] = 0;
			q++;
		}
		{
			WBunion u;
			u.w = 8 * mlen;
			q -= 8;
			memcpy(msg2 + q, &u.w, 4);
		}
	}

	for (grp = 0; grp < grps; grp++) {
		memcpy(mm.b, msg2 + os, 64);
		for (q = 0; q < 4; q++)
			abcd[q] = h[q];
		for (p = 0; p < 4; p++) {
			fctn = ff[p];
			rotn = rots[p];
			m = M[p];
			o = O[p];
			for (q = 0; q < 16; q++) {
				g = (m * q + o) % 16;
				f = abcd[1] + rol(abcd[0] + fctn(abcd) + k[q + 16 * p] + mm.w[g], rotn[q % 4]);

				abcd[0] = abcd[3];
				abcd[3] = abcd[2];
				abcd[2] = abcd[1];
				abcd[1] = f;
			}
		}
		for (p = 0; p < 4; p++)
			h[p] += abcd[p];
		os += 64;
	}
	return h;
}

void md5_to_str(unsigned *d, char* str) {
	int j, k;
	WBunion u;
	char tmp[8];
	str[0] = '\0';

	for (j = 0; j < 4; j++) {
		u.w = d[j];
		for (k = 0; k < 4; k++) {
			sprintf(tmp, "%02x", u.b[k]);
			strcat(str, tmp);
		}
	}
}
