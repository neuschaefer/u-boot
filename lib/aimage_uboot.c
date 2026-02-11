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

#include <common.h>
#include <stddef.h>
#include <aimage.h>

#include <linux/time.h>

extern int config_noaccess(void);

/*******************************************************************************
*******************************************************************************/

/*
   Eventually, platform ID and RSA + AES keys will be platform and/or customer
   specific. Until then, hardcode default values for initial development.
*/

static unsigned int platform_platform_id (void)
{
    return 0x53B22500;
}

static aimage_keypub_t const *platform_firmware_rsa_public_key(unsigned release_id)
{
    static const aimage_keypub_t stb225_default_rsa_public_key =
    {
        {
            0xcb77881b,
            0x8d62eded,0xc0616517,0xef01237b,0x41a11380,0xd6ef602a,0xf616552c,0x021ef68e,0x9fd5c979,
            0xc3d84951,0x3c223e7a,0x87d4481a,0xc753fe61,0x4fe6c8d6,0x0bdc4693,0xfded0d34,0x9149ba16,
            0x4797c0a3,0xb402b913,0xe6fc8c06,0x266bf25f,0x48297a42,0xf0e50389,0x4c3cf6ad,0x24456612,
            0xe8663de3,0xecf16642,0x6206570b,0xbbb30001,0xe5195832,0xf9027425,0xef8e594c,0xb95731b7,
            0x00000000
        }
    };

    static const aimage_keypub_t stb225_custom_pkg_sideload_key =
    {
        {
            0xf51902eb,
            0x123f6a3d,0x9b2aa85f,0xd3921e06,0x301454d4,0x6ee83e57,0x82f0823f,0x9127b85d,0x45c9da6c,
            0x62017ba0,0xc1994b59,0x829608e6,0xe8347ecb,0x8717d427,0x80d9fdf3,0x62334a7b,0x95326c84,
            0x0255afe5,0xc93f6bd5,0xdd1c84e5,0x6c191005,0xaa5df280,0x28634a00,0x7353834c,0xc28fae20,
            0x925c4cea,0x790d3bd1,0x4aeb8528,0xcf658aac,0xa8485b25,0x4219e881,0x1be938c1,0xc96a93f9,
            0x00000000
        }
    };

    static const aimage_keypub_t stb225_roku_rsa_public_key =
    {
        {
		0x8c2b841f,
		0x0d5d0821,0xf0a97cea,0xf3e456b3,0x0f3cbf8a,0x86d49300,0x9ab1b0da,0x2d7aecbe,0x1d80221c,
		0x3a85c196,0x3595b356,0x98bfb1ba,0xa9aabaa5,0x5787dcaa,0xa876613d,0x3b18ab51,0x2c397649,
		0x17d95245,0xe564faef,0xe17bd759,0xcac59b48,0x5807a6b1,0xa4c3da1b,0x014c9966,0xf4ca60e6,
		0xabc02055,0x0c29955d,0x4e8d261d,0xf22e7f51,0xc22bd4db,0x1154ee35,0x242993e9,0xd6b0c6f9,
		0x00000000
        }
    };

    if (release_id == 0x40000001)
        return &stb225_custom_pkg_sideload_key; 
    if ((release_id & 0x80000000) == 0 && !config_noaccess())
        return &stb225_default_rsa_public_key;
    return &stb225_roku_rsa_public_key;
}


/*******************************************************************************
*******************************************************************************/

static int abn_cmp (unsigned int *bn1, unsigned int *bn2, int words)
{
    int i;

    for (i = words - 1; i >= 0; i--) {
        if (bn1[i] > bn2[i]) return  1;
        if (bn1[i] < bn2[i]) return -1;
    }
    return 0;
}

static int abn_sub (unsigned int *bn, unsigned int *q, int words)
{
    unsigned int t1, t2, c1, c2 = 0;

    do {
        t1 = *bn;
        t2 = t1 - *q++;
        c1 = (t2 > t1) ? 1 : 0;
        t1 = t2 - c2;
        c2 = (t1 > t2) ? 1 : 0;
        c2 |= c1;
        *bn++ = t1;
    }
    while (--words != 0);
    return c2;
}

static void abn_mont_mul (unsigned int *z, unsigned int *x, unsigned int *y, const aimage_keypub_t *key)
{
    int i, j;
    int words = RSA_MOD_WORDS_1024;
    unsigned int temp, u, slice_x, cb, cw1, cw2;
    unsigned int mod_dash = key->mod_data[0];
    unsigned int *modulus = (unsigned int *) &key->mod_data[1];
    unsigned long long product;

    for (i = 0; i < (words + 1); i++)
        z[i] = 0;

    for (i = 0; i < words; i++) {
        slice_x = *x++;
        product = (((unsigned long long) *y++) * slice_x) + *z;
        cw1 = (product >> 32);
        temp = (product >> 0);
        u = (temp * mod_dash);
        product = (((unsigned long long) modulus[0]) * u) + temp;
        cw2 = (product >> 32);
        for (j = 1; j < words; j++) {
            product = (((unsigned long long) *y++) * slice_x) + z[1] + cw1;
            cw1 = (product >> 32);
            temp = (product >> 0);
            product = (((unsigned long long) modulus[j]) * u) + temp + cw2;
            cw2 = (product >> 32);
            temp = (product >> 0);
            *z++ = temp;
        }
        temp = cw1 + z[1];
        cb = (temp < cw1) ? 1 : 0;
        temp += cw2;
        if (temp < cw2)
            cb = 1;
        *z++ = temp;
        *z = cb;
        z -= words;
        y -= words;
    }

    if (abn_cmp (z, modulus, (words + 1)) >= 0)
        abn_sub (z, modulus, (words + 1));
}

static int abn_mont_exp (unsigned int *res, unsigned int *tmp, unsigned int *input, const aimage_keypub_t *key)
{
    int i;
    int words = RSA_MOD_WORDS_1024;
    unsigned int exp;

#if defined (AIMAGE_HARDCODED_EXP)
    exp = AIMAGE_HARDCODED_EXP;
#else
    exp = aimage_pubkey_exp (key);
#endif

    if (exp == 3) {
        abn_mont_mul (tmp, input, input, key);      /* tmp = input ^ 2 */
        abn_mont_mul (res, tmp, input, key);        /* res = input ^ 3 */
        return 0;
    }

    if (exp == 17) {
        abn_mont_mul (res, input, input, key);      /* res = input ^ 2 */
        abn_mont_mul (tmp, res, res, key);          /* tmp = input ^ 4 */
        abn_mont_mul (res, tmp, tmp, key);          /* res = input ^ 8 */
        abn_mont_mul (tmp, res, res, key);          /* tmp = input ^ 16 */
        abn_mont_mul (res, tmp, input, key);        /* res = input ^ 17 */
        return 0;
    }

    if (exp == 65537) {
        abn_mont_mul (res, input, input, key);      /* res = input ^ 2 */
        for (i = 0; i < 7; i++) {
            abn_mont_mul (tmp, res, res, key);      /* tmp = res ^ 2 */
            abn_mont_mul (res, tmp, tmp, key);      /* res = tmp ^ 2 */
        }
        abn_mont_mul (tmp, res, res, key);          /* tmp = input ^ 65536 */
        abn_mont_mul (res, tmp, input, key);        /* res = input ^ 65537 */
        return 0;
    }

    for (i = 0; i < (words + 1); i++)
        res[i] = 0;

    return -1;
}


/*******************************************************************************
*******************************************************************************/

unsigned int aimage_pubkey_sbdata (const aimage_keypub_t *key)
{
	int i;
	unsigned int sbdata = 0;

	for (i = 0; i < 4; i++)
		sbdata ^= (key->mod_data[i + 1] >> i);

	return sbdata;
}

unsigned int aimage_pubkey_exp (const aimage_keypub_t *key)
{
	switch ((aimage_pubkey_sbdata (key) >> 0) & 0x03)
	{
		case 0: return 3;
		case 1: return 17;
		case 2: return 65537;

		default:
			return 0;
	}
}

unsigned int aimage_pubkey_mod_words (const aimage_keypub_t *key)
{
	switch ((aimage_pubkey_sbdata (key) >> 2) & 0x07)
	{
		case 0: return RSA_MOD_WORDS_512;
		case 1: return RSA_MOD_WORDS_640;
		case 2: return RSA_MOD_WORDS_1024;
		case 3: return RSA_MOD_WORDS_1536;
		case 4: return RSA_MOD_WORDS_2048;

		default:
			return RSA_MOD_WORDS_RESERVED;
	}
}

unsigned int aimage_pubkey_mod_bits (const aimage_keypub_t *key)
{
	return aimage_pubkey_mod_words (key) * 32;
}


/*******************************************************************************
*******************************************************************************/

typedef enum
{
    SANITY_CHECK,
    VERIFY_HASH,
    VERIFY_SIGNATURE,
    GENERATE_HASH
}
aimage_process_mode_t;

static int aimage_v1_process (aimage_v1_header_t *header, const aimage_keypub_t *key, unsigned int type, unsigned int length, aimage_process_mode_t mode)
{
    int i;
    SHA_CTX context;
    unsigned int t[2][RSA_MOD_WORDS_1024 + 1];
    unsigned int *reference_hash;
    unsigned int platform_mask = ~0xF;

    if (((unsigned long) header) & 0x03)            /* header pointer must be aligned... */
        return -2;

    if (mode == GENERATE_HASH) {
        header->magic = IMG_MAGIC;
        header->magic2 = IMG_MAGIC2;
        header->type = type;
        header->length = ((length + 3) & ~0x03);    /* image must be a multiple of 4 bytes, even if data is not */
        header->data_start_offset = 0;              /* prepend not supported when generating headers here... */
        header->data_length = length;               /* prepend not supported when generating headers here... */
    }
    else {
        if (header->magic != IMG_MAGIC || header->magic2 != IMG_MAGIC2 ||
            (type && (header->type != type)) ||
            (header->platform_id & platform_mask) != (platform_platform_id() & platform_mask) ||
            header->length > length || header->length < sizeof(aimage_v1_header_t))
        {
            return -2;
        }
        if (mode == SANITY_CHECK)
            return 0;                               /* our work here is done... */
    }
    SHA1_Init (&context);
    SHA1_Update (&context, (unsigned char *) header, offsetof (aimage_v1_header_t, signature));
    reference_hash = header->hash;
    if (mode == VERIFY_SIGNATURE) {
        SHA1_Update (&context, (unsigned char *) header->hash, sizeof(header->hash));
        abn_mont_exp (t[1], t[0], header->signature, key);
        for (i = 0; i < 5; i++)
            t[1][i] ^= t[1][RSA_MOD_WORDS_1024 - 5 + i];
        reference_hash = t[1];
    }
    t[0][0] = 0;
    SHA1_Update (&context, (unsigned char *) t[0], 4);
    SHA1_Update (&context, (unsigned char *) header->reserved, header->length - offsetof (aimage_v1_header_t, reserved));
    SHA1_Final ((unsigned char *) t[0], &context);
    for (i = 0; i < 5; i++) {
        if (mode == GENERATE_HASH)
            reference_hash[i] = t[0][i];
        else
            if (t[0][i] != reference_hash[i])
                return -1;
    }

    return 0;       /* success */
}

int aimage_v1_sanity_check (aimage_v1_header_t *header, unsigned int type, unsigned int length)
{
    return aimage_v1_process (header, NULL, type, length, SANITY_CHECK);
}

int aimage_v1_verify_hash (aimage_v1_header_t *header, unsigned int type, unsigned int length)
{
    return aimage_v1_process (header, NULL, type, length, VERIFY_HASH);
}

int aimage_v1_verify_signature (aimage_v1_header_t *header, unsigned int type, unsigned int length, const aimage_keypub_t *key)
{
    return aimage_v1_process (header, key, type, length, VERIFY_SIGNATURE);
}

int aimage_v1_verify_signature_dk (aimage_v1_header_t *header, unsigned int type, unsigned int length)
{
    aimage_keypub_t *key = platform_firmware_rsa_public_key(header->release_id);

    return aimage_v1_process (header, key, type, length, VERIFY_SIGNATURE);
}

int aimage_v1_generate_hash (aimage_v1_header_t *header, unsigned int type, unsigned int length)
{
    return aimage_v1_process (header, NULL, type, length, GENERATE_HASH);
}


/*******************************************************************************
*******************************************************************************/

void aimage_v1_verify_signature_stream_init (aimage_stream_context_t *context,
                                             unsigned int type,
                                             unsigned int length,
                                             unsigned char *copy_dst,
                                             const aimage_keypub_t *key)
{
//  assert (length != 0);           /* (length == 0) is not allowed... */

    context->key = key;
    context->type = type;
    context->max_length = length;
    context->bytes_handled = 0;
    context->result_is_known = 0;

    if (copy_dst == NULL) {
        context->copy_dst = NULL;
    }
    else {
        context->copy_dst = copy_dst + sizeof(aimage_v1_header_t);
        memset (copy_dst, 0, sizeof(aimage_v1_header_t));
    }
}

#if 0
void aimage_v1_verify_signature_stream_init_dk (aimage_stream_context_t *context, unsigned int type, unsigned int length, unsigned char *copy_dst)
{
    aimage_keypub_t *key = platform_firmware_rsa_public_key();

    aimage_v1_verify_signature_stream_init (context, type, length, copy_dst, key);
}

/*
    return :  0   : verify success
           : -1   : verify failure
           : else : bytes of image data required before end of image (not accurate until complete (256 byte) header has been processed)
*/
int aimage_v1_verify_signature_stream_update (aimage_stream_context_t *context, unsigned char *input, unsigned long input_bytes)
{
    unsigned int t[2][RSA_MOD_WORDS_1024 + 1];
    unsigned int header_remaining;
    unsigned int image_remaining;
    unsigned int chunk_size;
    aimage_v1_header_t *header = &context->header;
    int i;

    if (context->result_is_known)
        return context->result;

    if (context->bytes_handled < sizeof (aimage_v1_header_t)) {
        header_remaining = (sizeof (aimage_v1_header_t) - context->bytes_handled);
        chunk_size = (input_bytes > header_remaining) ? header_remaining : input_bytes;
        memcpy (((unsigned char *) header) + context->bytes_handled, input, chunk_size);
        input += chunk_size;
        input_bytes -= chunk_size;
        context->bytes_handled += chunk_size;

        if (context->bytes_handled == sizeof(aimage_v1_header_t)) {
            if (aimage_v1_sanity_check (header, context->type, context->max_length) != 0) {
                context->result = -1;
                context->result_is_known = 1;
                return -1;
            }
            abn_mont_exp (t[1], t[0], header->signature, context->key);
            for (i = 0; i < 5; i++)
                context->hash_expected[i] = t[1][i] ^ t[1][RSA_MOD_WORDS_1024 - 5 + i];
            SHA1_Init (&context->sha_context);
            SHA1_Update (&context->sha_context, (unsigned char *) header, offsetof (aimage_v1_header_t, signature));
            SHA1_Update (&context->sha_context, (unsigned char *) header->hash, sizeof(header->hash));
            t[0][0] = 0;
            SHA1_Update (&context->sha_context, (unsigned char *) t[0], 4);
            SHA1_Update (&context->sha_context, (unsigned char *) header->reserved, sizeof(aimage_v1_header_t) - offsetof (aimage_v1_header_t, reserved));
        }
        else
            return context->max_length;
    }

    image_remaining = header->length - context->bytes_handled;
    chunk_size = (input_bytes > image_remaining) ? image_remaining : input_bytes;
    SHA1_Update (&context->sha_context, input, chunk_size);
    context->bytes_handled += chunk_size;

    if (context->copy_dst) {
        memcpy (context->copy_dst, input, chunk_size);
        context->copy_dst += chunk_size;
    }

    image_remaining -= chunk_size;
    if (image_remaining == 0) {
        SHA1_Final ((unsigned char *) t[0], &context->sha_context);
        if (memcmp (t[0], context->hash_expected, SHA_DIGEST_LENGTH) == 0) {
            context->result = 0;
            if (context->copy_dst) {
                memcpy ((context->copy_dst - header->length), header, sizeof(aimage_v1_header_t));
#if 0
//              aimage_v1_display_info_dk ((aimage_v1_header_t *) (context->copy_dst - header->length), header->length);
                aimage_v1_display_info_with_confidence ((aimage_v1_header_t *) (context->copy_dst - header->length), AIMAGE_CONFIDENCE_SIGNATURE_OK);
#endif
            }
        }
        else
            context->result = -1;

        context->result_is_known = 1;
        return context->result;
    }

    return image_remaining;
}
#endif

#if 0
int aimage_v1_verify_signature_dots (aimage_v1_header_t *header, unsigned int type, unsigned int length, unsigned int chunksize, const aimage_keypub_t *key)
{
    int result;
    aimage_stream_context_t ctx;
    unsigned char *data = (unsigned char *) header;
    int dots_printed = 0;

//  assert (length != 0);       /* (length == 0) is not allowed... */

    if (chunksize > length)
        chunksize = length;

    aimage_v1_verify_signature_stream_init (&ctx, type, length, NULL, key);

    while (1) {
        result = aimage_v1_verify_signature_stream_update (&ctx, data, chunksize);
        data += chunksize;
        if (result == 0)        /* signature check passed */
            break;
        if (result == -1)       /* signature check failed */
            break;
        printf (".");
        dots_printed = 1;
    }

    if (dots_printed)
        printf ("\n");

    return result;
}
#endif

int aimage_v1_verify_signature_dots_dk (aimage_v1_header_t *header, unsigned int type, unsigned int length, unsigned int chunksize)
{
    aimage_keypub_t *key = platform_firmware_rsa_public_key(header->release_id);

    return aimage_v1_verify_signature_dots (header, type, length, chunksize, key);
}


/*******************************************************************************
*******************************************************************************/

unsigned char *aimage_v1_start_of_image_data (aimage_v1_header_t *header)
{
    return (unsigned char *) (((unsigned long) header) + header->data_start_offset);
}

/*
   WARNING: Result returned in a static buffer... not MT safe.
*/
char *aimage_release_id_to_string (unsigned int release_id)
{
    static char buf[20];    /* worst case: 255.255-65536 (ie 13 + nul) */

    sprintf (buf, "%d.%d-%d", RELEASE_ID_MAJOR(release_id),
                              RELEASE_ID_MINOR(release_id),
                              RELEASE_ID_BUILD(release_id));

    return buf;
}

char *aimage_type_to_string (unsigned int type)
{
    switch (type)
    {
        case IMG_TYPE_DEBUG_MESSAGE: return "debug_message";
        case IMG_TYPE_ENV:           return "env";
        case IMG_TYPE_FIRMWARE_BLOB: return "firmware_blob";
        case IMG_TYPE_INITFS_CRAMFS: return "initfs_cramfs";
        case IMG_TYPE_UBOOT_MIPSEB:  return "uboot_mipseb";
        case IMG_TYPE_UBOOT_MIPSEL:  return "uboot_mipsel";
        case IMG_TYPE_UIMAGE:        return "uimage";

        default:
            return "UNKNOWN";
    }
}

char *aimage_flags_to_encmode_string (unsigned int flags)
{
    unsigned int encmode_flags = (flags & IMG_FLAG_ENCMODE_MASK);

    switch (encmode_flags)
    {
        case IMG_FLAG_ENCMODE_NONE:         return "none";
        case IMG_FLAG_ENCMODE_CBC_ONEPASS:  return "cbc_onepass";
        case IMG_FLAG_ENCMODE_CBC_4KBLOCKS: return "cbc_4kblocks";
        case IMG_FLAG_ENCMODE_CTR:          return "ctr";

        default:
            return "UNKNOWN";
    }
}

/*
   Fixme: add AIMAGE_CONFIDENCE_xxx_BAD confidence levels...
*/
int aimage_v1_display_info_with_confidence (aimage_v1_header_t *header, int confidence)
{
    char datestring[64];
    time_t ttime;
    int len;

    if (confidence == AIMAGE_CONFIDENCE_KNOWN_BAD) {
        printf ("%s%-24s: 0x%lx\n", "Image ", "base address", (unsigned long) header);
        printf ("%s%-24s: %s\n", "Image ", "sanity check", "FAILED");
        return 0;
    }

    ttime = (time_t) header->build_time;
    ctime_r (&ttime, datestring);
    len = strlen (datestring);
    if ((len > 0) && (datestring[len - 1] == '\n'))
        datestring[len - 1] = 0;

    printf ("%s%-24s: %s\n",     "Image ", "type",                    aimage_type_to_string (header->type));
//  printf ("%s%-24s: 0x%08x\n", "Image ", "flags",                   header->flags);
    printf ("%s%-24s: %s\n",     "Image ", "encmode",                 aimage_flags_to_encmode_string (header->flags));
    printf ("%s%-24s: %s\n",     "Image ", "release ID",              aimage_release_id_to_string (header->release_id));
    printf ("%s%-24s: 0x%08x\n", "Image ", "platform ID",             header->platform_id);

    /* fixme... */
    printf ("%s%-24s: 0x%lx\n",   "Image ", "base address",            (unsigned long) header);

    printf ("%s%-24s: %d\n",     "Image ", "length",                  header->length);
    printf ("%s%-24s: %d\n",     "Image ", "data start offset",       header->data_start_offset);
    printf ("%s%-24s: %d\n",     "Image ", "data length",             header->data_length);
    printf ("%s%-24s: 0x%08x\n", "Image ", "data link address",       header->data_link_address);
//  printf ("%s%-24s: %d\n",     "Image ", "data entry point offset", header->data_entry_point_offset);
    printf ("%s%-24s: 0x%08x\n", "Image ", "usd",                     header->usd);
    if (confidence == AIMAGE_CONFIDENCE_HEADER_OK) {
        printf ("%s%-24s: %s\n", "Image ", "hash check", "FAILED");
        return 0;
    }
    printf ("%s%-24s: %s\n", "Image ", "build (== signed) time", datestring);
    printf ("%s%-24s: %s\n", "Image ", "build host", (header->build_host_offset != 0) ? ((char *) header) + header->build_host_offset : "(unknown)");
    printf ("%s%-24s: %s\n", "Image ", (confidence == AIMAGE_CONFIDENCE_SIGNATURE_OK) ? "SIGNATURE check" : "HASH check", "passed !!");

    return 0;
}

int aimage_v1_display_info (aimage_v1_header_t *header, unsigned int image_length_max, const aimage_keypub_t *key)
{
    int confidence;

    if (aimage_v1_verify_signature (header, 0, image_length_max, key) == 0)
        confidence = AIMAGE_CONFIDENCE_SIGNATURE_OK;
    else if (aimage_v1_verify_hash (header, 0, image_length_max) == 0)
        confidence = AIMAGE_CONFIDENCE_HASH_OK;
    else if (aimage_v1_sanity_check (header, 0, image_length_max) == 0)
        confidence = AIMAGE_CONFIDENCE_HEADER_OK;
    else
        confidence = AIMAGE_CONFIDENCE_KNOWN_BAD;

    return aimage_v1_display_info_with_confidence (header, confidence);
}

int aimage_v1_display_info_dk (aimage_v1_header_t *header, unsigned int image_length_max)
{
    aimage_keypub_t *key = platform_firmware_rsa_public_key(header->release_id);

    return aimage_v1_display_info (header, image_length_max, key);
}


/*****************************************************************************
*****************************************************************************/
#if 0
/*****************************************************************************
*****************************************************************************/

#include <command.h>

int aimage_selftest (void)
{
    static const aimage_keypub_t rsa_testkey_3 =
    {
        {
            0xcb77881b,
            0x8d62eded,0xc0616517,0xef01237b,0x41a11380,0xd6ef602a,0xf616552c,0x021ef68e,0x9fd5c979,
            0xc3d84951,0x3c223e7a,0x87d4481a,0xc753fe61,0x4fe6c8d6,0x0bdc4693,0xfded0d34,0x9149ba16,
            0x4797c0a3,0xb402b913,0xe6fc8c06,0x266bf25f,0x48297a42,0xf0e50389,0x4c3cf6ad,0x24456612,
            0xe8663de3,0xecf16642,0x6206570b,0xbbb30001,0xe5195832,0xf9027425,0xef8e594c,0xb95731b7,
            0x00000000
        }
    };

    static const unsigned char data_for_testkey_3[] =
    {
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x69, 0x6d, 0x67, 0x41,
        0x52, 0x4d, 0x63, 0x43, 0xb8, 0x22, 0x58, 0x58, 0x00, 0x25, 0xb2, 0x53,
        0x01, 0x00, 0x00, 0x00, 0x08, 0x01, 0x00, 0x00, 0x05, 0x00, 0x00, 0x00,
        0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x01, 0x00, 0x00, 0x00, 0xb0, 0x0c, 0x20, 0x47, 0x00, 0x00, 0x00, 0x00,
        0xc6, 0x94, 0x71, 0xaa, 0x2f, 0x0c, 0xba, 0xdb, 0xec, 0x60, 0x25, 0x59,
        0xd2, 0xfb, 0x80, 0xa5, 0xee, 0x32, 0x23, 0x1b, 0x0c, 0x2a, 0xb0, 0xe3,
        0xac, 0x7f, 0x5e, 0xba, 0x96, 0x02, 0xb6, 0xe9, 0x42, 0xfa, 0x90, 0x26,
        0xa1, 0x5f, 0xde, 0xc0, 0xf1, 0x49, 0x62, 0xe1, 0xb3, 0x5e, 0x11, 0x19,
        0x39, 0x0d, 0xa9, 0xd4, 0xd1, 0xce, 0x4e, 0x3a, 0xf1, 0x6f, 0xb8, 0xed,
        0x2d, 0x93, 0x51, 0x9c, 0xb0, 0x17, 0xb2, 0x82, 0x54, 0xdb, 0xb6, 0x36,
        0x1c, 0x26, 0x87, 0x93, 0x22, 0x04, 0x02, 0x2f, 0x11, 0x48, 0x98, 0x34,
        0x63, 0x33, 0xad, 0xb5, 0x25, 0x7a, 0x45, 0x91, 0x25, 0x71, 0x21, 0x16,
        0x27, 0x16, 0x02, 0xb0, 0xed, 0xb0, 0xb5, 0xbd, 0x5d, 0x7c, 0xef, 0x2f,
        0xca, 0xfc, 0x69, 0xce, 0x1a, 0x3f, 0x4b, 0x9a, 0x49, 0x60, 0xd3, 0xf4,
        0xad, 0xcf, 0xf2, 0x94, 0xd2, 0x25, 0x08, 0x02, 0xb3, 0xcd, 0xa2, 0xf7,
        0x57, 0x4d, 0xd4, 0x99, 0xa2, 0xdb, 0xaf, 0x1a, 0xc5, 0x16, 0x34, 0x0d,
        0xc6, 0xb0, 0x80, 0xe5, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x74, 0x65, 0x73, 0x74, 0x0a, 0x00, 0x00, 0x00
    };

    static const aimage_keypub_t rsa_testkey_17 =
    {
        {
            0x27c9c35f,
            0x7226a761,0x681bd7d7,0xa1056a60,0x604d29db,0xcd22c5ad,0x48e9c55f,0x4906d0ad,0x182a2526,
            0x0a1d4105,0xd268d6c6,0x961c1d10,0x974677af,0x170b96ac,0x6c63d264,0xa53e2b52,0x16449873,
            0x0b9ae41b,0xc07216f1,0xbf509c2d,0x807dcb09,0x67550b02,0x938d92a6,0xd02663fb,0x96c8e501,
            0x359acb37,0x8372b622,0x05c2cbb1,0x6010b4e4,0xaf15a816,0xe2f89245,0x1c6b6075,0x95c0b80a,
            0x00000000
        }
    };

    static const unsigned char data_for_testkey_17[] =
    {
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x69, 0x6d, 0x67, 0x41,
        0x52, 0x4d, 0x63, 0x43, 0xb8, 0x22, 0x58, 0x58, 0x00, 0x25, 0xb2, 0x53,
        0x01, 0x00, 0x00, 0x00, 0x08, 0x01, 0x00, 0x00, 0x05, 0x00, 0x00, 0x00,
        0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x01, 0x00, 0x00, 0x00, 0x79, 0x16, 0x20, 0x47, 0x00, 0x00, 0x00, 0x00,
        0x3b, 0x20, 0x1f, 0xb7, 0x1d, 0x30, 0x7e, 0xbd, 0x3f, 0xb0, 0x79, 0x69,
        0x22, 0x8c, 0xc2, 0x14, 0xd0, 0xe5, 0x7f, 0xd5, 0xaa, 0x5d, 0xa8, 0x27,
        0x23, 0x5e, 0xed, 0x90, 0x9a, 0xd0, 0xf2, 0x2e, 0x92, 0x59, 0xd8, 0x55,
        0x28, 0x92, 0x77, 0x5e, 0x22, 0xed, 0xf8, 0x7e, 0x69, 0xb7, 0x08, 0xa4,
        0x59, 0x8e, 0xb0, 0xa5, 0xc3, 0x37, 0x02, 0x30, 0xa7, 0x4f, 0x9b, 0x15,
        0xd0, 0xce, 0xd4, 0xf4, 0x3f, 0x20, 0x88, 0x90, 0xe1, 0x41, 0x50, 0xfd,
        0xeb, 0xa9, 0x30, 0xb7, 0x99, 0x11, 0x14, 0x40, 0x9e, 0x02, 0xa6, 0x98,
        0xac, 0x6d, 0xb0, 0xc6, 0x6e, 0x16, 0x13, 0xb3, 0x8d, 0x79, 0x33, 0xfa,
        0x10, 0x37, 0xc0, 0x43, 0x85, 0x39, 0xf8, 0xb4, 0xef, 0xde, 0x90, 0x03,
        0xbc, 0x3b, 0x03, 0x5b, 0x88, 0x41, 0x3d, 0x36, 0xd9, 0x43, 0xf4, 0x75,
        0xa8, 0x8a, 0xf9, 0x0d, 0xb5, 0xee, 0x30, 0x91, 0x88, 0xa6, 0x48, 0xf7,
        0x85, 0xd6, 0x25, 0x61, 0xeb, 0xad, 0xb2, 0x49, 0x29, 0x23, 0x9e, 0x3f,
        0xb8, 0x50, 0xae, 0x86, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x74, 0x65, 0x73, 0x74, 0x0a, 0x00, 0x00, 0x00
    };

    unsigned char buf[1024];

    int i;
    int result;
    unsigned long start, stop;
    const aimage_keypub_t *testkey = &rsa_testkey_3;
    unsigned char *data = (unsigned char *) data_for_testkey_3;
    unsigned int data_size = sizeof(data_for_testkey_3);
    aimage_stream_context_t ctx;

    printf ("----\n");

    printf ("rsa_testkey_3 : bits %d, exp %d\n", aimage_pubkey_mod_bits (&rsa_testkey_3), aimage_pubkey_exp (&rsa_testkey_3));
    printf ("rsa_testkey_17: bits %d, exp %d\n", aimage_pubkey_mod_bits (&rsa_testkey_17), aimage_pubkey_exp (&rsa_testkey_17));

    printf ("----\n");

    aimage_v1_display_info ((aimage_v1_header_t *) data, data_size, testkey);

    printf ("----\n");

    aimage_v1_verify_signature_stream_init (&ctx, IMG_TYPE_DEBUG_MESSAGE, sizeof(buf), buf, testkey);
    for (i = 0; i < data_size; i++) {
        result = aimage_v1_verify_signature_stream_update (&ctx, (unsigned char *) &data[i], 1);
        if (result == -1) {
            printf ("aimage_v1_verify_signature_stream_update: failed !?! (i == %d)\n", i);
            break;
        }
        if (result == 0) {
            printf ("aimage_v1_verify_signature_stream_update: success (i == %d)\n", i);
            break;
        }
    }
    printf ("----\n");

    memcpy (buf, data, data_size);
    aimage_v1_verify_signature_stream_init (&ctx, IMG_TYPE_DEBUG_MESSAGE, (1 * 1024 * 1024), NULL, testkey);
    result = aimage_v1_verify_signature_stream_update (&ctx, buf, sizeof(buf));
    printf ("aimage_v1_verify_signature_stream_update: %s\n", (result == 0) ? "success (test passed)" : "failure (test failed)");

    printf ("----\n");

    memcpy (buf, data, data_size);
    buf[0] ^= 0xFF;     /* corrupt the image data... verify should now fail */
    aimage_v1_verify_signature_stream_init (&ctx, IMG_TYPE_DEBUG_MESSAGE, (1 * 1024 * 1024), NULL, testkey);
    result = aimage_v1_verify_signature_stream_update (&ctx, buf, sizeof(buf));
    printf ("aimage_v1_verify_signature_stream_update: %s\n", (result == 0) ? "success (test failed)" : "failure (test passed)");

    printf ("----\n");

    memcpy (buf, data, data_size);
    ((aimage_v1_header_t *) buf)->usd ^= 0x12345678;    /* corrupt the image usd data... verify should still pass */

    aimage_v1_display_info ((aimage_v1_header_t *) buf, data_size, testkey);

    printf ("----\n");

    aimage_v1_verify_signature_stream_init (&ctx, IMG_TYPE_DEBUG_MESSAGE, (1 * 1024 * 1024), NULL, testkey);
    result = aimage_v1_verify_signature_stream_update (&ctx, buf, sizeof(aimage_v1_header_t) - 1);
    result = aimage_v1_verify_signature_stream_update (&ctx, buf + (sizeof(aimage_v1_header_t) - 1), sizeof(buf) - (sizeof(aimage_v1_header_t) - 1));
    printf ("aimage_v1_verify_signature_stream_update: %s\n", (result == 0) ? "success (test passed)" : "failure (test failed)");

    printf ("----\n");

    result = aimage_v1_verify_signature_dots ((aimage_v1_header_t *) data, 0, data_size, 5, testkey);
    printf ("aimage_v1_verify_signature_dots: %s\n", (result == 0) ? "success (test passed)" : "failure (test failed)");

    printf ("----\n");

    for (i = 0; i < 1; i++) {
        start = get_timer(0);
        result = aimage_v1_verify_hash ((aimage_v1_header_t *) data, 0, data_size);
        stop = get_timer(0);
        printf ("aimage_v1_verify_hash         :         image size %d, %5d usec%s\n", data_size,
                (unsigned int) ((((unsigned long long) (stop - start)) * 1000000) / CONFIG_SYS_HZ),
                (result == 0) ? "" : " FAILED ?!?");
    }

    for (i = 0; i < 1; i++) {
        start = get_timer(0);
        result = aimage_v1_verify_signature ((aimage_v1_header_t *) data, 0, data_size, testkey);
        stop = get_timer(0);
        printf ("aimage_v1_verify_signature    : exp %2d, image size %d, %5d usec%s\n",
                aimage_pubkey_exp (testkey), data_size,
                (unsigned int) ((((unsigned long long) (stop - start)) * 1000000) / CONFIG_SYS_HZ),
                (result == 0) ? "" : " FAILED ?!?");
    }

    testkey = &rsa_testkey_17;
    data = (unsigned char *) data_for_testkey_17;
    data_size = sizeof(data_for_testkey_17);

    for (i = 0; i < 2; i++) {
        start = get_timer(0);
        result = aimage_v1_verify_signature ((aimage_v1_header_t *) data, 0, data_size, testkey);
        stop = get_timer(0);
        printf ("aimage_v1_verify_signature    : exp %2d, image size %d, %5d usec%s\n",
                aimage_pubkey_exp (testkey), data_size,
                (unsigned int) ((((unsigned long long) (stop - start)) * 1000000) / CONFIG_SYS_HZ),
                (result == 0) ? "" : " FAILED ?!?");
    }

    printf ("----\n");

    return 0;
}

U_BOOT_CMD(
	st_ai, 1, 1, (int (*)(struct cmd_tbl_s *, int, int, char *[])) aimage_selftest,
	"st_ai   - run aimage selftest\n",
	"   - run aimage selftest\n"
);


/*****************************************************************************
*****************************************************************************/
#endif
/*****************************************************************************
*****************************************************************************/

