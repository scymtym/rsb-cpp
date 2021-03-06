/* ============================================================
 *
 * This file is a part of the RSB project.
 *
 * Copyright (C) 2011 by Johannes Wienke <jwienke at techfak dot uni-bielefeld dot de>
 *
 * Written by Ulrich Drepper <drepper@gnu.ai.mit.edu> and heavily modified
 * for GnuPG by <werner.koch@guug.de> and adapted for the need of FPM Blowfish
 * Plugin
 *
 * Latest author:
 * Frederic RUAUDEL <grumz@users.sf.net>
 * FPMBlowfishPlugin
 * Copyleft (c) 2003 Frederic RUAUDEL, all rights reversed
 * Copyleft (C) 1995, 1996, 1998, 1999 Free Software Foundation, Inc.
 *
 * This file may be licensed under the terms of the
 * GNU Lesser General Public License Version 3 (the ``LGPL''),
 * or (at your option) any later version.
 *
 * Software distributed under the License is distributed
 * on an ``AS IS'' basis, WITHOUT WARRANTY OF ANY KIND, either
 * express or implied. See the LGPL for the specific language
 * governing rights and limitations.
 *
 * You should have received a copy of the LGPL along with this
 * program. If not, go to http://www.gnu.org/licenses/lgpl.html
 * or write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * The development of this software was supported by:
 *   CoR-Lab, Research Institute for Cognition and Robotics
 *     Bielefeld University
 *
 * ============================================================ */

#include "MD5.h"

#include <sstream>
#include <iomanip>
#include <ios>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

using namespace std;

namespace rsb {
namespace util {

/**
 * Rotate a 32 bit integer by n bytes
 */
#if defined(__GNUC__) && defined(__i386__)
static inline unsigned int rol(unsigned int x, int n)
{
    __asm__("roll %%cl,%0"
            :"=r" (x)
            :"0" (x),"c" (n));
    return x;
}
#else
#define rol(x,n) ( ((x) << (n)) | ((x) >> (32-(n))) )
#endif

/* These are the four functions used in the four steps of the MD5 algorithm
 and defined in the RFC 1321.  The first function is a little bit optimized
 (as found in Colin Plumbs public domain implementation).  */
/* #define FF(b, c, d) ((b & c) | (~b & d)) */
#define FF(b, c, d) (d ^ (b & (c ^ d)))
#define FG(b, c, d) FF (d, b, c)
#define FH(b, c, d) (b ^ c ^ d)
#define FI(b, c, d) (c ^ (b | ~d))

class MD5Hasher {
public:

    typedef unsigned int u32;
    typedef unsigned char byte;

    MD5Hasher() {
        this->init();
    }

    ~MD5Hasher() {
    }

    /**
     * FPM: At this point I diverge from GnuPG's implementation to write my
     * own wrapper function.  I'll use this to hash the passphrase.
     */
    byte* hash(byte* buf, size_t nbytes) {
        byte* ret_val;

        this->init();
        this->write(buf, nbytes);
        this->final();

        ret_val = (byte*) malloc(16);
        memset(ret_val, 0, 16);
        memcpy(ret_val, this->read(), 16);

        return (ret_val);
    }

private:

    void init() {
        this->A = 0x67452301;
        this->B = 0xefcdab89;
        this->C = 0x98badcfe;
        this->D = 0x10325476;

        this->nblocks = 0;
        this->count = 0;

        memset(buf, 0, 64);
    }

    /**
     * transform n*64 bytes
     */
    void transform(byte* data) {
        u32 correct_words[16];
        u32 A = this->A;
        u32 B = this->B;
        u32 C = this->C;
        u32 D = this->D;
        u32* cwp = correct_words;

#ifdef BIG_ENDIAN_HOST
        {
            int i;
            byte *p2, *p1;

            for (i = 0, p1 = data, p2 = (byte*)correct_words; i < 16; i++, p2 += 4)
            {
                p2[3] = *p1++;
                p2[2] = *p1++;
                p2[1] = *p1++;
                p2[0] = *p1++;
            }
        }
#else
        memcpy(correct_words, data, 64);
#endif

#define OP(a, b, c, d, s, T)                        \
        do                                  \
        {                                   \
            a += FF (b, c, d) + (*cwp++) + T;       \
            a = rol(a, s);                          \
            a += b;                             \
        } while (0)

        /* Before we start, one word about the strange constants.
         They are defined in RFC 1321 as

         T[i] = (int) (4294967296.0 * fabs (sin (i))), i=1..64
         */

        /* Round 1.  */OP (A, B, C, D, 7, 0xd76aa478);
        OP (D, A, B, C, 12, 0xe8c7b756);
        OP (C, D, A, B, 17, 0x242070db);
        OP (B, C, D, A, 22, 0xc1bdceee);
        OP (A, B, C, D, 7, 0xf57c0faf);
        OP (D, A, B, C, 12, 0x4787c62a);
        OP (C, D, A, B, 17, 0xa8304613);
        OP (B, C, D, A, 22, 0xfd469501);
        OP (A, B, C, D, 7, 0x698098d8);
        OP (D, A, B, C, 12, 0x8b44f7af);
        OP (C, D, A, B, 17, 0xffff5bb1);
        OP (B, C, D, A, 22, 0x895cd7be);
        OP (A, B, C, D, 7, 0x6b901122);
        OP (D, A, B, C, 12, 0xfd987193);
        OP (C, D, A, B, 17, 0xa679438e);
        OP (B, C, D, A, 22, 0x49b40821);

#undef OP
#define OP(f, a, b, c, d, k, s, T)  \
        do                                    \
        {                                 \
            a += f (b, c, d) + correct_words[k] + T;              \
            a = rol(a, s);                            \
            a += b;                               \
        } while (0)

        // Round 2.
        OP (FG, A, B, C, D, 1, 5, 0xf61e2562);
        OP (FG, D, A, B, C, 6, 9, 0xc040b340);
        OP (FG, C, D, A, B, 11, 14, 0x265e5a51);
        OP (FG, B, C, D, A, 0, 20, 0xe9b6c7aa);
        OP (FG, A, B, C, D, 5, 5, 0xd62f105d);
        OP (FG, D, A, B, C, 10, 9, 0x02441453);
        OP (FG, C, D, A, B, 15, 14, 0xd8a1e681);
        OP (FG, B, C, D, A, 4, 20, 0xe7d3fbc8);
        OP (FG, A, B, C, D, 9, 5, 0x21e1cde6);
        OP (FG, D, A, B, C, 14, 9, 0xc33707d6);
        OP (FG, C, D, A, B, 3, 14, 0xf4d50d87);
        OP (FG, B, C, D, A, 8, 20, 0x455a14ed);
        OP (FG, A, B, C, D, 13, 5, 0xa9e3e905);
        OP (FG, D, A, B, C, 2, 9, 0xfcefa3f8);
        OP (FG, C, D, A, B, 7, 14, 0x676f02d9);
        OP (FG, B, C, D, A, 12, 20, 0x8d2a4c8a);

        // Round 3.
        OP (FH, A, B, C, D, 5, 4, 0xfffa3942);
        OP (FH, D, A, B, C, 8, 11, 0x8771f681);
        OP (FH, C, D, A, B, 11, 16, 0x6d9d6122);
        OP (FH, B, C, D, A, 14, 23, 0xfde5380c);
        OP (FH, A, B, C, D, 1, 4, 0xa4beea44);
        OP (FH, D, A, B, C, 4, 11, 0x4bdecfa9);
        OP (FH, C, D, A, B, 7, 16, 0xf6bb4b60);
        OP (FH, B, C, D, A, 10, 23, 0xbebfbc70);
        OP (FH, A, B, C, D, 13, 4, 0x289b7ec6);
        OP (FH, D, A, B, C, 0, 11, 0xeaa127fa);
        OP (FH, C, D, A, B, 3, 16, 0xd4ef3085);
        OP (FH, B, C, D, A, 6, 23, 0x04881d05);
        OP (FH, A, B, C, D, 9, 4, 0xd9d4d039);
        OP (FH, D, A, B, C, 12, 11, 0xe6db99e5);
        OP (FH, C, D, A, B, 15, 16, 0x1fa27cf8);
        OP (FH, B, C, D, A, 2, 23, 0xc4ac5665);

        // Round 4.
        OP (FI, A, B, C, D, 0, 6, 0xf4292244);
        OP (FI, D, A, B, C, 7, 10, 0x432aff97);
        OP (FI, C, D, A, B, 14, 15, 0xab9423a7);
        OP (FI, B, C, D, A, 5, 21, 0xfc93a039);
        OP (FI, A, B, C, D, 12, 6, 0x655b59c3);
        OP (FI, D, A, B, C, 3, 10, 0x8f0ccc92);
        OP (FI, C, D, A, B, 10, 15, 0xffeff47d);
        OP (FI, B, C, D, A, 1, 21, 0x85845dd1);
        OP (FI, A, B, C, D, 8, 6, 0x6fa87e4f);
        OP (FI, D, A, B, C, 15, 10, 0xfe2ce6e0);
        OP (FI, C, D, A, B, 6, 15, 0xa3014314);
        OP (FI, B, C, D, A, 13, 21, 0x4e0811a1);
        OP (FI, A, B, C, D, 4, 6, 0xf7537e82);
        OP (FI, D, A, B, C, 11, 10, 0xbd3af235);
        OP (FI, C, D, A, B, 2, 15, 0x2ad7d2bb);
        OP (FI, B, C, D, A, 9, 21, 0xeb86d391);

        // Put checksum in context given as argument.
        this->A += A;
        this->B += B;
        this->C += C;
        this->D += D;
    }

    /**
     * The routine updates the message-digest context to
     * account for the presence of each of the characters inBuf[0..inLen-1]
     * in the message whose digest is being computed.
     */
    void write(byte* inbuf, size_t inlen) {
        if (this->count == 64) /* flush the buffer */
        {
            this->transform(this->buf);
            this->count = 0;
            this->nblocks++;
        }

        if (!inbuf) {
            return;
        }

        if (this->count) {
            for (; inlen && this->count < 64; inlen--) {
                this->buf[this->count++] = *inbuf++;
            }

            this->write(NULL, 0);
            if (!inlen) {
                return;
            }
        }

        while (inlen >= 64) {
            this->transform(inbuf);
            this->count = 0;
            this->nblocks++;
            inlen -= 64;
            inbuf += 64;
        }

        for (; inlen && this->count < 64; inlen--) {
            this->buf[this->count++] = *inbuf++;
        }
    }

    /**
     * The routine final terminates the message-digest computation and
     * ends with the desired message digest in mdContext->digest[0...15].
     * The handle is prepared for a new MD5 cycle.
     * Returns 16 bytes representing the digest.
     */
    void final() {
        u32 t, msb, lsb;
        byte* p;

        this->write(NULL, 0);

        msb = 0;
        t = this->nblocks;

        if ((lsb = t << 6) < t) /* multiply by 64 to make a byte count */
        {
            msb++;
        }

        msb += t >> 26;
        t = lsb;

        if ((lsb = t + this->count) < t) /* add the count */
        {
            msb++;
        }

        t = lsb;

        if ((lsb = t << 3) < t) /* multiply by 8 to make a bit count */
        {
            msb++;
        }

        msb += t >> 29;

        if (this->count < 56) /* enough room */
        {
            this->buf[this->count++] = 0x80; /* pad */

            while (this->count < 56) {
                this->buf[this->count++] = 0; /* pad */
            }
        } else /* need one extra block */
        {
            this->buf[this->count++] = 0x80; /* pad character */

            while (this->count < 64) {
                this->buf[this->count++] = 0;
            }

            this->write(NULL, 0); /* flush */

            memset(this->buf, 0, 56); /* fill next block with zeroes */
        }

        /* append the 64 bit count */
        this->buf[56] = lsb;
        this->buf[57] = lsb >> 8;
        this->buf[58] = lsb >> 16;
        this->buf[59] = lsb >> 24;
        this->buf[60] = msb;
        this->buf[61] = msb >> 8;
        this->buf[62] = msb >> 16;
        this->buf[63] = msb >> 24;

        this->transform(this->buf);

        p = this->buf;

#ifdef BIG_ENDIAN_HOST
#define X(a) do { *p++ = this->##a    ; *p++ = this->##a >> 8;      \
                  *p++ = this->##a >> 16; *p++ = this->##a >> 24; } while(0)
#else /* little endian */
        /*#define X(a) do { *(u32*)p = this->##a ; p += 4; } while(0)*/
        /* Unixware's cpp doesn't like the above construct so we do it his way:
         * (reported by Allan Clark) */
#define X(a) do { *(u32*)p = (*this).a ; p += 4; } while(0)
#endif

        X(A);
        X(B);
        X(C);
        X(D);

#undef X
    }

    byte* read() {
        return this->buf;
    }

    u32 A;
    u32 B;
    u32 C;
    u32 D;
    u32 nblocks;
    byte buf[64];
    int count;

};

void freeMd5Hash(void* hash) {
    free((unsigned char*) hash);
}

MD5::MD5(const string& s) :
            hasher(new MD5Hasher),
            hash(hasher->hash((unsigned char*) s.c_str(), strlen(s.c_str())),
                    freeMd5Hash) {
}

MD5::~MD5() {
}

string MD5::toHexString(const bool& pretty) const {

    stringstream s;
    s << hex;

    for (int i = 0; i < 16; ++i) {
        if (pretty && ((i % 4) == 0) && i != 0) {
            s << " ";
        }
        s << setw(2) << setfill('0') << (int) hash[i];
    }

    return s.str();

}

ostream& operator<<(ostream& stream, const MD5& sum) {
    return stream << sum.toHexString(true);
}

}
}
