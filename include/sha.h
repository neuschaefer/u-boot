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

#ifndef _SHA_H_
#define _SHA_H_

#define SHA_DIGEST_LENGTH 20

/*
   Do not modify this structure definition.
   The layout below is assumed by the assembler functions.
*/
typedef struct
{
    unsigned int  state[5];     /* */
    unsigned int  bytesHandled; /* Not quite offical SHA1: will behave differently if hashing >= 2^32 bytes */
    unsigned char buffer[64];   /* */
}
SHA_CTX;

extern void SHA1_Init (SHA_CTX *context);
extern void SHA1_Update (SHA_CTX *context, unsigned char *input, unsigned long input_bytes);
extern void SHA1_Final (unsigned char digest[20], SHA_CTX *context);

extern int  SHA1_Selftest (void);

#endif
