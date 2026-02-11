/*
 * (C) Copyright 2007 Andre McCurdy, NXP Semiconductors
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <asm/byteorder.h>

#include <sha.h>

#define SHA_ROUND_UNROLLING_LEVEL		3
#define SHA_SUPPORT_UNALIGNED_SOURCE_DATA

/*****************************************************************************
*****************************************************************************/

#define sha_htonl(x) __cpu_to_be32(x)

#define K1 0x5A827999U
#define K2 0x6ED9EBA1U
#define K3 0x8F1BBCDCU
#define K4 0xCA62C1D6U

#define ROTATE_LEFT(x, n) (((x) << (n)) | ((x) >> (32 - (n))))

/*
    The SHA f() functions...

    Note: 'Applied Cryptography, 2nd Ed, B. Schneider' gives them as :

    #define f1(B,C,D) ( ( B & C ) | ( ~B & D ) )
    #define f2(B,C,D) ( B ^ C ^ D )
    #define f3(B,C,D) ( ( B & C ) | ( B & D ) | ( C & D ) )
    #define f4(B,C,D) ( B ^ C ^ D )

    However, f1 and f3 can be expressed with slightly fewer operations
    as stated in: 'Implementing Elliptic Curve Cryptography', M. Rosing.
*/

#define f1(B,C,D) (((C ^ D) & B) ^ D)           /* Rounds  0-19 */
#define f2(B,C,D)  ((B ^ C) ^ D)                /* Rounds 20-39 */
#define f3(B,C,D) (((B | C) & D) | (B & C))     /* Rounds 40-59 */
#define f4(B,C,D)  ((B ^ C) ^ D)                /* Rounds 60-79 */

#define subRound(a, b, c, d, e, f, k, data) \
    ( e += ROTATE_LEFT(a,5) + f(b,c,d) + k + data, b = ROTATE_LEFT(b,30) )


/*****************************************************************************
*****************************************************************************/

void SHA1_Init (SHA_CTX *context)
{
    context->bytesHandled = 0;
    context->state[0] = 0x67452301U;
    context->state[1] = 0xEFCDAB89U;
    context->state[2] = 0x98BADCFEU;
    context->state[3] = 0x10325476U;
    context->state[4] = 0xC3D2E1F0U;
}

/*****************************************************************************
*****************************************************************************/

#if defined (CFG_SHA_ASSEMBLER)
extern void SHA1_Transform_asm (unsigned int *state, unsigned int *in, int repeat);
static void SHA1_Transform (unsigned int *state, unsigned int *in, int repeat)
{
    SHA1_Transform_asm (state, in, repeat);
}
#else

/*
    Block copy of 32bit words with endian swap on little endian platforms.
    Must work correctly if (src == dst).
    May assume: src and dst are correctly 32bit aligned.
                bytecount is initially non-zero and a multiple of 4.
*/
static void copy_words_to_big_endian (unsigned int *dst, unsigned int *src, int bytecount)
{
//  assert (bytecount);

    do {
        unsigned int value = *src++;
        *dst++ = sha_htonl (value);
        bytecount -= 4;
    }
    while (bytecount > 0);
}

static void SHA1_Transform (unsigned int *state, unsigned int *in, int repeat)
{
    unsigned int x[80];         /* Beware... 320 bytes going on the stack !! */

    int i;
    unsigned int temp;
    unsigned int A = state[0];
    unsigned int B = state[1];
    unsigned int C = state[2];
    unsigned int D = state[3];
    unsigned int E = state[4];

    for ( ; repeat; repeat--)
    {
        copy_words_to_big_endian (x, in, (16 * 4));

        in += 16;

        for (i = 16; i < 80; i++) {                         /* expand 16 32bit words to fill 80 word workspace */
            temp = x[i-16] ^ x[i-14] ^ x[i-8] ^ x[i-3];
            x[i] = ROTATE_LEFT(temp, 1);
        }

#if (SHA_ROUND_UNROLLING_LEVEL == 0)                        /* Fully rolled up loops (smallest code size) */

        for (i = 0; i < 80; i++) {
            if (i < 40)
                if (i < 20)
                    temp = f1(B,C,D) + K1;
                else
                    temp = f2(B,C,D) + K2;
            else
                if (i < 60)
                    temp = f3(B,C,D) + K3;
                else
                    temp = f4(B,C,D) + K4;

            temp += ROTATE_LEFT(A,5) + E + x[i];
            E = D;
            D = C;
            C = ROTATE_LEFT(B,30);
            B = A;
            A = temp;
        }

#elif (SHA_ROUND_UNROLLING_LEVEL == 1)                      /* slightly unrolled loops... */

        for (i = 0; i < 20; i++) {
            temp = ROTATE_LEFT(A,5) + E + x[i] + f1(B,C,D) + K1;
            E = D;
            D = C;
            C = ROTATE_LEFT(B,30);
            B = A;
            A = temp;
        }

        for (i = 20; i < 40; i++) {
            temp = ROTATE_LEFT(A,5) + E + x[i] + f2(B,C,D) + K2;
            E = D;
            D = C;
            C = ROTATE_LEFT(B,30);
            B = A;
            A = temp;
        }

        for (i = 40; i < 60; i++) {
            temp = ROTATE_LEFT(A,5) + E + x[i] + f3(B,C,D) + K3;
            E = D;
            D = C;
            C = ROTATE_LEFT(B,30);
            B = A;
            A = temp;
        }

        for (i = 60; i < 80; i++) {
            temp = ROTATE_LEFT(A,5) + E + x[i] + f4(B,C,D) + K4;
            E = D;
            D = C;
            C = ROTATE_LEFT(B,30);
            B = A;
            A = temp;
        }

#elif (SHA_ROUND_UNROLLING_LEVEL == 2)                      /* more unrolled loops... */

        {
            unsigned int *xptr = x;

            for (i = 4; i; i--) {
                temp = *xptr++; subRound( A, B, C, D, E, f1, K1, temp );
                temp = *xptr++; subRound( E, A, B, C, D, f1, K1, temp );
                temp = *xptr++; subRound( D, E, A, B, C, f1, K1, temp );
                temp = *xptr++; subRound( C, D, E, A, B, f1, K1, temp );
                temp = *xptr++; subRound( B, C, D, E, A, f1, K1, temp );
            }

            for (i = 4; i; i--) {
                temp = *xptr++; subRound( A, B, C, D, E, f2, K2, temp );
                temp = *xptr++; subRound( E, A, B, C, D, f2, K2, temp );
                temp = *xptr++; subRound( D, E, A, B, C, f2, K2, temp );
                temp = *xptr++; subRound( C, D, E, A, B, f2, K2, temp );
                temp = *xptr++; subRound( B, C, D, E, A, f2, K2, temp );
            }

            for (i = 4; i; i--) {
                temp = *xptr++; subRound( A, B, C, D, E, f3, K3, temp );
                temp = *xptr++; subRound( E, A, B, C, D, f3, K3, temp );
                temp = *xptr++; subRound( D, E, A, B, C, f3, K3, temp );
                temp = *xptr++; subRound( C, D, E, A, B, f3, K3, temp );
                temp = *xptr++; subRound( B, C, D, E, A, f3, K3, temp );
            }

            for (i = 4; i; i--) {
                temp = *xptr++; subRound( A, B, C, D, E, f4, K4, temp );
                temp = *xptr++; subRound( E, A, B, C, D, f4, K4, temp );
                temp = *xptr++; subRound( D, E, A, B, C, f4, K4, temp );
                temp = *xptr++; subRound( C, D, E, A, B, f4, K4, temp );
                temp = *xptr++; subRound( B, C, D, E, A, f4, K4, temp );
            }
        }

#elif (SHA_ROUND_UNROLLING_LEVEL == 3)                      /* fully unrolled loops... largest code size and usually the fastest */

        subRound( A, B, C, D, E, f1, K1, x[ 0] );
        subRound( E, A, B, C, D, f1, K1, x[ 1] );
        subRound( D, E, A, B, C, f1, K1, x[ 2] );
        subRound( C, D, E, A, B, f1, K1, x[ 3] );
        subRound( B, C, D, E, A, f1, K1, x[ 4] );
        subRound( A, B, C, D, E, f1, K1, x[ 5] );
        subRound( E, A, B, C, D, f1, K1, x[ 6] );
        subRound( D, E, A, B, C, f1, K1, x[ 7] );
        subRound( C, D, E, A, B, f1, K1, x[ 8] );
        subRound( B, C, D, E, A, f1, K1, x[ 9] );
        subRound( A, B, C, D, E, f1, K1, x[10] );
        subRound( E, A, B, C, D, f1, K1, x[11] );
        subRound( D, E, A, B, C, f1, K1, x[12] );
        subRound( C, D, E, A, B, f1, K1, x[13] );
        subRound( B, C, D, E, A, f1, K1, x[14] );
        subRound( A, B, C, D, E, f1, K1, x[15] );
        subRound( E, A, B, C, D, f1, K1, x[16] );
        subRound( D, E, A, B, C, f1, K1, x[17] );
        subRound( C, D, E, A, B, f1, K1, x[18] );
        subRound( B, C, D, E, A, f1, K1, x[19] );

        subRound( A, B, C, D, E, f2, K2, x[20] );
        subRound( E, A, B, C, D, f2, K2, x[21] );
        subRound( D, E, A, B, C, f2, K2, x[22] );
        subRound( C, D, E, A, B, f2, K2, x[23] );
        subRound( B, C, D, E, A, f2, K2, x[24] );
        subRound( A, B, C, D, E, f2, K2, x[25] );
        subRound( E, A, B, C, D, f2, K2, x[26] );
        subRound( D, E, A, B, C, f2, K2, x[27] );
        subRound( C, D, E, A, B, f2, K2, x[28] );
        subRound( B, C, D, E, A, f2, K2, x[29] );
        subRound( A, B, C, D, E, f2, K2, x[30] );
        subRound( E, A, B, C, D, f2, K2, x[31] );
        subRound( D, E, A, B, C, f2, K2, x[32] );
        subRound( C, D, E, A, B, f2, K2, x[33] );
        subRound( B, C, D, E, A, f2, K2, x[34] );
        subRound( A, B, C, D, E, f2, K2, x[35] );
        subRound( E, A, B, C, D, f2, K2, x[36] );
        subRound( D, E, A, B, C, f2, K2, x[37] );
        subRound( C, D, E, A, B, f2, K2, x[38] );
        subRound( B, C, D, E, A, f2, K2, x[39] );

        subRound( A, B, C, D, E, f3, K3, x[40] );
        subRound( E, A, B, C, D, f3, K3, x[41] );
        subRound( D, E, A, B, C, f3, K3, x[42] );
        subRound( C, D, E, A, B, f3, K3, x[43] );
        subRound( B, C, D, E, A, f3, K3, x[44] );
        subRound( A, B, C, D, E, f3, K3, x[45] );
        subRound( E, A, B, C, D, f3, K3, x[46] );
        subRound( D, E, A, B, C, f3, K3, x[47] );
        subRound( C, D, E, A, B, f3, K3, x[48] );
        subRound( B, C, D, E, A, f3, K3, x[49] );
        subRound( A, B, C, D, E, f3, K3, x[50] );
        subRound( E, A, B, C, D, f3, K3, x[51] );
        subRound( D, E, A, B, C, f3, K3, x[52] );
        subRound( C, D, E, A, B, f3, K3, x[53] );
        subRound( B, C, D, E, A, f3, K3, x[54] );
        subRound( A, B, C, D, E, f3, K3, x[55] );
        subRound( E, A, B, C, D, f3, K3, x[56] );
        subRound( D, E, A, B, C, f3, K3, x[57] );
        subRound( C, D, E, A, B, f3, K3, x[58] );
        subRound( B, C, D, E, A, f3, K3, x[59] );

        subRound( A, B, C, D, E, f4, K4, x[60] );
        subRound( E, A, B, C, D, f4, K4, x[61] );
        subRound( D, E, A, B, C, f4, K4, x[62] );
        subRound( C, D, E, A, B, f4, K4, x[63] );
        subRound( B, C, D, E, A, f4, K4, x[64] );
        subRound( A, B, C, D, E, f4, K4, x[65] );
        subRound( E, A, B, C, D, f4, K4, x[66] );
        subRound( D, E, A, B, C, f4, K4, x[67] );
        subRound( C, D, E, A, B, f4, K4, x[68] );
        subRound( B, C, D, E, A, f4, K4, x[69] );
        subRound( A, B, C, D, E, f4, K4, x[70] );
        subRound( E, A, B, C, D, f4, K4, x[71] );
        subRound( D, E, A, B, C, f4, K4, x[72] );
        subRound( C, D, E, A, B, f4, K4, x[73] );
        subRound( B, C, D, E, A, f4, K4, x[74] );
        subRound( A, B, C, D, E, f4, K4, x[75] );
        subRound( E, A, B, C, D, f4, K4, x[76] );
        subRound( D, E, A, B, C, f4, K4, x[77] );
        subRound( C, D, E, A, B, f4, K4, x[78] );
        subRound( B, C, D, E, A, f4, K4, x[79] );

#else
#error "Unsupported SHA_ROUND_UNROLLING_LEVEL"
#endif

        A += state[0];
        B += state[1];
        C += state[2];
        D += state[3];
        E += state[4];

        state[0] = A;                   /* Update the accumulated result */
        state[1] = B;
        state[2] = C;
        state[3] = D;
        state[4] = E;
   }
}
#endif

#if defined (SHA_SUPPORT_UNALIGNED_SOURCE_DATA)

/*
    Wrapper for sha1_transform()
    Required on some platforms (ARM, MIPS, but not x86) if src data is not 32bit aligned.
*/
static void SHA1_Transform_unaligned (unsigned int *state, unsigned char *in, int repeat)
{
    unsigned int buf[16 * 4];           /* Max Transform is 4 64byte blocks */

    for ( ; repeat > 0; repeat -= 4) {
        int block_count = ((repeat < 4) ? repeat : 4);
        memcpy (buf, in, (64 * block_count));
        SHA1_Transform (state, buf, block_count);
        in += (64 * 4);
    }
}

#endif


/*****************************************************************************
*****************************************************************************/

void SHA1_Update (SHA_CTX *context, unsigned char *input, unsigned long input_bytes)
{
    int byteIndex;
    int startLen;
    int remainderLen;
    int finalRemainder;

    byteIndex = (context->bytesHandled & 0x3F);
    context->bytesHandled += input_bytes;
    startLen = (64 - byteIndex);
    remainderLen = (input_bytes - startLen);

    if (remainderLen >= 0) {
        memcpy (&context->buffer[byteIndex], input, startLen);
        SHA1_Transform (context->state, (unsigned int *) context->buffer, 1);

#if defined (SHA_SUPPORT_UNALIGNED_SOURCE_DATA)
        if ((((unsigned long) &input[startLen]) & 0x03) != 0)
            SHA1_Transform_unaligned (context->state, &input[startLen], (remainderLen / 64));
        else
#endif
        {
            SHA1_Transform (context->state, (unsigned int *) &input[startLen], (remainderLen / 64));
        }

        /* finalRemainder begins after last complete block */
        finalRemainder = startLen + (remainderLen & ~0x3F);
        byteIndex = 0;
    }
    else {
        /* finalRemainder begins at start of input */
        finalRemainder = 0;
    }

    memcpy (&context->buffer[byteIndex], &input[finalRemainder], (input_bytes - finalRemainder));
}

static void unaligned_write32_be (unsigned char *dst, unsigned int value)
{
    *dst++ = value >> 24;
    *dst++ = value >> 16;
    *dst++ = value >> 8;
    *dst++ = value >> 0;
}

void SHA1_Final (unsigned char digest[SHA_DIGEST_LENGTH], SHA_CTX *context)
{
    unsigned char finalblock[64 + 8];
    unsigned int blockStartOffset;
    unsigned int finalBlockLength;
    int i;

    memset (finalblock, 0, 64);
    finalblock[0] = 0x80;
    blockStartOffset = (context->bytesHandled & 0x3F);
    finalBlockLength = (((blockStartOffset < 56) ? 56 : 120) - blockStartOffset);
    unaligned_write32_be (&finalblock[finalBlockLength + 0], context->bytesHandled >> 29);
    unaligned_write32_be (&finalblock[finalBlockLength + 4], context->bytesHandled << 3);
    SHA1_Update (context, finalblock, finalBlockLength + 8);
    for (i = 0; i < (SHA_DIGEST_LENGTH / 4); i++)
        unaligned_write32_be (&digest[i*4], context->state[i]);
}

/*****************************************************************************
*****************************************************************************/
#if 1
/*****************************************************************************
*****************************************************************************/

#include <command.h>

extern void sha1sum_display (unsigned char *buf, unsigned int length);

static void show_result (unsigned char *hash, char *expected_string)
{
    int i;
    char actual_string[(2 * SHA_DIGEST_LENGTH) + 1 + 100];      /* 40 chars, 1 nul, a few for luck */

    for (i = 0; i < SHA_DIGEST_LENGTH; i++)
        sprintf (&actual_string[i*2], "%02X", hash[i]);

    printf ("Actual  : %s\nExpected: %s%s\n\n",
            actual_string, expected_string,
            (strcmp (actual_string, expected_string) != 0) ? " FAILED !!" : "");
}

int SHA1_Selftest (void)
{
#if defined (SHA_SUPPORT_UNALIGNED_SOURCE_DATA)
    static const char rawdata[] = "abcdefghijklmnopqrstuvwxyz0123456789abcdefghijklmnopqrstuvwxyz0123456789"
                                  "bcdefghijklmnopqrstuvwxyz0123456789abcdefghijklmnopqrstuvwxyz0123456789"
                                  "cdefghijklmnopqrstuvwxyz0123456789abcdefghijklmnopqrstuvwxyz0123456789"
                                  "defghijklmnopqrstuvwxyz0123456789abcdefghijklmnopqrstuvwxyz0123456789"
                                  "efghijklmnopqrstuvwxyz0123456789abcdefghijklmnopqrstuvwxyz0123456789"
                                  "fghijklmnopqrstuvwxyz0123456789abcdefghijklmnopqrstuvwxyz0123456789"
                                  "ghijklmnopqrstuvwxyz0123456789abcdefghijklmnopqrstuvwxyz0123456789"
                                  "hijklmnopqrstuvwxyz0123456789abcdefghijklmnopqrstuvwxyz0123456789"
                                  "ijklmnopqrstuvwxyz0123456789abcdefghijklmnopqrstuvwxyz0123456789"
                                  "jklmnopqrstuvwxyz0123456789abcdefghijklmnopqrstuvwxyz0123456789"
                                  "klmnopqrstuvwxyz0123456789abcdefghijklmnopqrstuvwxyz0123456789"
                                  "lmnopqrstuvwxyz0123456789abcdefghijklmnopqrstuvwxyz0123456789"
                                  "mnopqrstuvwxyz0123456789abcdefghijklmnopqrstuvwxyz0123456789"
                                  "nopqrstuvwxyz0123456789abcdefghijklmnopqrstuvwxyz0123456789"
                                  "opqrstuvwxyz0123456789abcdefghijklmnopqrstuvwxyz0123456789"
                                  "pqrstuvwxyz0123456789abcdefghijklmnopqrstuvwxyz0123456789"
                                  "qrstuvwxyz0123456789abcdefghijklmnopqrstuvwxyz0123456789"
                                  "rstuvwxyz0123456789abcdefghijklmnopqrstuvwxyz0123456789"
                                  "stuvwxyz0123456789abcdefghijklmnopqrstuvwxyz0123456789"
                                  "tuvwxyz0123456789abcdefghijklmnopqrstuvwxyz0123456789"
                                  "uvwxyz0123456789abcdefghijklmnopqrstuvwxyz0123456789"
                                  "vwxyz0123456789abcdefghijklmnopqrstuvwxyz0123456789"
                                  "wxyz0123456789abcdefghijklmnopqrstuvwxyz0123456789"
                                  "xyz0123456789abcdefghijklmnopqrstuvwxyz0123456789";
#endif

    int i;
    SHA_CTX context;
    unsigned char hash[SHA_DIGEST_LENGTH];
    unsigned char buf[1024];

    printf ("abc (aligned)\n");
    SHA1_Init (&context);
    SHA1_Update (&context, (unsigned char *) "abc", 3);
    SHA1_Final (hash, &context);
    show_result (hash, "A9993E364706816ABA3E25717850C26C9CD0D89D");

#if defined (SHA_SUPPORT_UNALIGNED_SOURCE_DATA)
    /*
       Note: This isn't actually a good test for mis-aligned data.
             With only 3 bytes of input, the data will be passed
             through context->buffer rather than being hashed
             directly from it's source address.
    */
    printf ("abc (mis-aligned)\n");
    SHA1_Init (&context);
    memcpy (buf, "_abc", 4);
    SHA1_Update (&context, &buf[1], 3);
    SHA1_Final (hash, &context);
    show_result (hash, "A9993E364706816ABA3E25717850C26C9CD0D89D");
#endif

    printf ("abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq\n");
    SHA1_Init (&context);
    SHA1_Update (&context, (unsigned char *) "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq", 56);
    SHA1_Final (hash, &context);
    show_result (hash, "84983E441C3BD26EBAAE4AA1F95129E5E54670F1");

    printf ("A million repetitions of \"a\"\n");
    SHA1_Init (&context);
    memset (buf, 'a', 1000);
    for (i = 0; i < 1000; i++)
        SHA1_Update (&context, buf, 1000);
    SHA1_Final (hash, &context);
    show_result (hash, "34AA973CD4C4DAA4F61EEB2BDBAD27316534016F");

#if defined (SHA_SUPPORT_UNALIGNED_SOURCE_DATA)
    printf ("Lots of misaligned input data...\n");
    SHA1_Init (&context);
    for (i = 0; i < sizeof(rawdata); i++)
        SHA1_Update (&context, (unsigned char *) &rawdata[i], (sizeof(rawdata) - i));
    SHA1_Final (hash, &context);
    show_result (hash, "1966198757BF74D485DF596320B76E8FEE9F526E");
#endif

#if defined (CONFIG_STB225)
    /*
       Slightly hacky sha1 benchmark (assumes SDRAM is at least 8 MBytes and
       that hashing SDRAM contents while the system is running is safe...).
    */
    sha1sum_display ((unsigned char *) CFG_SDRAM_BASE, (8 * 1024 * 1024));
#endif

    return 0;
}

U_BOOT_CMD(
	st_sha1, 1, 1, (int (*)(struct cmd_tbl_s *, int, int, char *[])) SHA1_Selftest,
	"st_sha1 - run sha1 selftest\n",
	" - run sha1 selftest\n"
);

/*****************************************************************************
*****************************************************************************/
#endif
/*****************************************************************************
*****************************************************************************/

