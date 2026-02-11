#include "common.h"
#include "rle_decoder.h"

static void memset16(void *_ptr, unsigned short val, unsigned count)
{
	unsigned short *ptr = _ptr;
	count >>= 1;
	while (count--)
		*ptr++ = val;
}

/** decode_rle_image - decodes an rle image to buf
* @data:	[in] image pointer
* @buf:		[out] Output buffer.
* #size:	[in] image size in bytes.
* max_size: 	[in] maximum size of output image
*	Uses RGB565 format.
*/
uint32_t decode_rle_image(void *data, void *buf, uint32_t size, uint32_t max_size)
{
	unsigned short *bits, *ptr;

	ptr = (unsigned short *)data;
	bits = (unsigned short *)buf;

	while (size > 3) {
		uint32_t n = ptr[0];
		if (n > max_size)
			return 1;
		memset16(bits, ptr[1], n << 1);
		bits += n;
		max_size -= n;
		ptr += 2;
		size -= 4;
	}

	return 0;
}
