/*
 * (C) Copyright 2004-2008 Andre McCurdy, NXP Semiconductors
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

#ifndef _AIMAGE_H_
#define _AIMAGE_H_

#include <sha.h>

#define RSA_MOD_WORDS_RESERVED          (0)
#define RSA_MOD_WORDS_512               (16)        /* 16 32bit words ==  512 bit RSA signature */
#define RSA_MOD_WORDS_640               (20)        /* 20 32bit words ==  640 bit RSA signature */
#define RSA_MOD_WORDS_1024              (32)        /* 32 32bit words == 1024 bit RSA signature */
#define RSA_MOD_WORDS_1536              (48)        /* 48 32bit words == 1536 bit RSA signature (extended header required) */
#define RSA_MOD_WORDS_2048              (64)        /* 64 32bit words == 2048 bit RSA signature (extended header required) */

#define RSA_MOD_WORDS_MAX               (64)

#define HOST_STRING_LENGTH_MAX          (32)

#define IMG_TYPE_INVALID                (0x00)      /* never used */
#define IMG_TYPE_DEBUG_MESSAGE          (0x01)      /* misc text messages */
#define IMG_TYPE_BLOADER                (0x02)
#define IMG_TYPE_SLAVELOADER            (0x03)      /* slaveloader is stored in Flash as an SRAM bootloader image */
#define IMG_TYPE_ENV                    (0x04)
#define IMG_TYPE_RBF                    (0x05)
#define IMG_TYPE_JBC                    (0x06)
#define IMG_TYPE_CEXAPP                 (0x07)
#define IMG_TYPE_SLAVEAPP               (0x08)
#define IMG_TYPE_ZIMAGE                 (0x09)
#define IMG_TYPE_INITFS_CRAMFS          (0x0A)
#define IMG_TYPE_INITFS_ZEXT2           (0x0B)      /* obsolete... replaced by IMG_TYPE_INITRD */
#define IMG_TYPE_APPFS_EXT2             (0x0C)
#define IMG_TYPE_APPFS_CRAMFS           (0x0D)
#define IMG_TYPE_FIRMWARE_BLOB          (0x0E)
#define IMG_TYPE_BOOTCOUNT              (0x0F)
#define IMG_TYPE_BOOTSELECTION          (0x10)
#define IMG_TYPE_GNFS_EXT2              (0x11)      /* a gracenote database contained within an ext[23] filesystem image */
#define IMG_TYPE_ETH0MAC                (0x12)      /* used to wrap up a MAC address string for HAS7752 ISP Flash programming */
#define IMG_TYPE_BLOADER_TESTMODE       (0x13)      /* bootloader image for test mode uart download */
#define IMG_TYPE_BLOADER_DFU            (0x14)      /* bootloader image for DFU mode USB download */
#define IMG_TYPE_SPLASHSCREEN           (0x15)      /* startup LCD screen image */
#define IMG_TYPE_ROPIC                  (0x16)      /* Read Only Per Instance Configuration */
#define IMG_TYPE_INITRD                 (0x17)      /* */
#define IMG_TYPE_UIMAGE                 (0x18)      /* */
#define IMG_TYPE_UBOOT_MIPSEL           (0x19)
#define IMG_TYPE_UBOOT_MIPSEB           (0x1A)
#define IMG_TYPE_INITFS_SQUASHFS        (0x1B)
#define IMG_TYPE_CRAMFS_AUTH            (0x101)
#define IMG_TYPE_CUSTOM_PKG_TOKEN       IMG_TYPE_ROPIC
#define IMG_TYPE_BOOTDATA               IMG_TYPE_JBC

#define IMG_FLAG_PREPEND                (1 <<  0)   /* default is overwrite (ie header forms part of image) */
#define IMG_FLAG_BOOTABLE               (1 <<  1)   /* should the target ever try to execute this image */
#define IMG_FLAG_DFU_VID_PRESENT        (1 <<  2)   /* should the dfu_vid field in header be considered valid */
#define IMG_FLAG_DFU_PID_PRESENT        (1 <<  3)   /* should the dfu_pid field in header be considered valid */
#define IMG_FLAG_DFU_STRING_PRESENT     (1 <<  4)   /* should the dfu_string field in header be considered valid */
#define IMG_FLAG_EXTENDED_HEADER        (1 <<  5)   /* */
#define IMG_FLAG_ENCMODE_NONE           (0 <<  6)   /* image data is not encrypted */
#define IMG_FLAG_ENCMODE_CBC_ONEPASS    (1 <<  6)   /* image data is encrypted before signing: CBC mode, entire image in one pass */
#define IMG_FLAG_ENCMODE_CBC_4KBLOCKS   (2 <<  6)   /* image data is encrypted before signing: CBC mode, 4k byte blocks, block group iv = (iv + (first block's block offset)) */
#define IMG_FLAG_ENCMODE_CTR            (3 <<  6)   /* image data is encrypted before signing: CTR mode, block iv = (iv + (block offset)) */
#define IMG_FLAG_ENCMODE_MASK           (7 <<  6)   /* */

#define IMG_MAGIC                       0x41676d69
#define IMG_MAGIC2                      0x43634d52
#define EXT_MAGIC                       0x41747865
#define EXT_MAGIC2                      0x43634d52

#define RELEASE_ID_MAJOR_SHIFT          21
#define RELEASE_ID_MAJOR_MASK           0x3FF
#define RELEASE_ID_MINOR_SHIFT          15
#define RELEASE_ID_MINOR_MASK           0x3F
#define RELEASE_ID_BUILD_SHIFT          1
#define RELEASE_ID_BUILD_MASK           0x3FFF

#define RELEASE_ID_MAJOR(release_id)    (((release_id) >> RELEASE_ID_MAJOR_SHIFT) & RELEASE_ID_MAJOR_MASK)
#define RELEASE_ID_MINOR(release_id)    (((release_id) >> RELEASE_ID_MINOR_SHIFT) & RELEASE_ID_MINOR_MASK)
#define RELEASE_ID_BUILD(release_id)    (((release_id) >> RELEASE_ID_BUILD_SHIFT) & RELEASE_ID_BUILD_MASK)

typedef struct
{
    unsigned int vector;                        /* reserved for a branch instruction (if data_start_offset and data_entry_point_offset are 0) */
    unsigned int vector2;                       /* reserved for a branch instruction branch delay slot */
    unsigned int magic;                         /* */
    unsigned int magic2;                        /* */
    unsigned int release_id;                    /* toplevel build / release version */
    unsigned int platform_id;                   /* target platform information */
    unsigned int type;                          /* image type */
    unsigned int length;                        /* length of entire image (header + data + trailing padding) */
    unsigned int data_length;                   /* length of image data (ie from data_start_offset to start of trailing padding) */
    unsigned int data_start_offset;             /* 0 if header is part of data, sizeof(aimage_v1_header_t) if header is prepended to data */
    unsigned int data_link_address;             /* for non-pic executable images, where should the image placed in memory in order to execute */
    unsigned int data_entry_point_offset;       /* 0 if execution entry point is at the beginning of image data */
    unsigned int flags;                         /* image flags */
    unsigned int build_time;                    /* if non-zero, gives time when image was created (seconds since 1970) */
    unsigned int build_host_offset;             /* if non-zero, offset of a string in the image data giving build host information */
    unsigned int signature[RSA_MOD_WORDS_1024]; /* 32 32bit words == 1024 bit RSA signature */
    unsigned int hash[5];                       /* raw sha1 hash (fallback if not checking signature) */
    unsigned int usd;                           /* "un-signed data" (treated as 0 during hash and signature verification, regardless of actual value) */
    unsigned int reserved[7];                   /* image type specific params etc (number tweaked to keep header size == 256 bytes) */

    /*
        Note: The final 16 bytes of the header are used to form the
              IV for aimage images which are encrypted with AES (ie if
              IMG_FLAG_ENCRYPTED_V1 flag is set).
              From a security standpoint, the IV does _not_ have to be
              secret in any way, but it _should_ be unique for every
              image which is encrypted with a given AES key.
    */
    unsigned int iv[4];                         /* space reserved for random data or timestamp to ensure IV is unique for every image which is signed */
}
aimage_v1_header_t;

typedef struct
{
    unsigned int magic;                         /* */
    unsigned int magic2;                        /* */
    unsigned int signature[RSA_MOD_WORDS_1024]; /* 32 32bit words == second half of 2048 bit RSA signature */
    unsigned int reserved[30];                  /* image type specific params etc (number tweaked to keep header size == 256 bytes) */
}
aimage_v1_header_extension_t;

typedef struct
{
    unsigned int mod_data[RSA_MOD_WORDS_MAX + 2];
}
aimage_keypub_t;

typedef struct
{
    int result;
    int result_is_known;
    SHA_CTX sha_context;
    unsigned char *copy_dst;
    unsigned int bytes_handled;
    aimage_v1_header_t header;
    const aimage_keypub_t *key;
    unsigned int type;
    unsigned int max_length;
    unsigned int hash_expected[SHA_DIGEST_LENGTH / 4];
}
aimage_stream_context_t;

#define AIMAGE_CONFIDENCE_KNOWN_BAD	(0)
#define AIMAGE_CONFIDENCE_HEADER_OK	(1)
#define AIMAGE_CONFIDENCE_HASH_OK	(2)
#define AIMAGE_CONFIDENCE_SIGNATURE_OK	(3)


/****************************************************************************/
/****************************************************************************/

extern int aimage_v1_sanity_check (aimage_v1_header_t *header, unsigned int type, unsigned int length);
extern int aimage_v1_verify_signature (aimage_v1_header_t *header, unsigned int type, unsigned int length, const aimage_keypub_t *key);
extern int aimage_v1_verify_signature_dk (aimage_v1_header_t *header, unsigned int type, unsigned int length);
extern int aimage_v1_verify_signature_dots (aimage_v1_header_t *header, unsigned int type, unsigned int length, unsigned int chunksize, const aimage_keypub_t *key);
extern int aimage_v1_verify_signature_dots_dk (aimage_v1_header_t *header, unsigned int type, unsigned int length, unsigned int chunksize);
extern int aimage_v1_verify_hash (aimage_v1_header_t *header, unsigned int type, unsigned int length);
extern int aimage_v1_generate_hash (aimage_v1_header_t *header, unsigned int type, unsigned int length);
extern void aimage_v1_verify_signature_stream_init (aimage_stream_context_t *context, unsigned int type, unsigned int length, unsigned char *copy_dst, const aimage_keypub_t *key);
extern void aimage_v1_verify_signature_stream_init_dk (aimage_stream_context_t *context, unsigned int type, unsigned int length, unsigned char *copy_dst);
extern int aimage_v1_verify_signature_stream_update (aimage_stream_context_t *context, unsigned char *input, unsigned long input_bytes);
extern int aimage_v1_decrypt_inplace (aimage_v1_header_t *header, unsigned char *aes_key_raw);
extern int aimage_v1_decrypt_inplace_dk (aimage_v1_header_t *header);
extern int aimage_v1_display_info (aimage_v1_header_t *header, unsigned int length, const aimage_keypub_t *key);
extern int aimage_v1_display_info_dk (aimage_v1_header_t *header, unsigned int length);
extern int aimage_v1_display_info_with_confidence (aimage_v1_header_t *header, int confidence);
extern unsigned char *aimage_v1_start_of_image_data (aimage_v1_header_t *image);

/****************************************************************************/
/****************************************************************************/

extern unsigned int aimage_pubkey_sbdata (const aimage_keypub_t *key);
extern unsigned int aimage_pubkey_exp (const aimage_keypub_t *key);
extern unsigned int aimage_pubkey_mod_words (const aimage_keypub_t *key);
extern unsigned int aimage_pubkey_mod_bits (const aimage_keypub_t *key);

extern char *aimage_release_id_to_string (unsigned int release_id);
extern char *aimage_type_to_string (unsigned int type);
extern char *aimage_flags_to_encmode_string (unsigned int flags);

extern int aimage_selftest (void);


/****************************************************************************/
/****************************************************************************/

#endif

