#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef long (*boot_services)(int svc, ...);

static void upx_putchar(boot_services svc, char c)
{
	svc(0x23, c);
}

static bool upx_read(boot_services svc, void *p, size_t size)
{
	return svc(0x51, p, size);
}

/* Cache operations */

#define CACHELINE       32  /* e300 core manual, 1.1.5.2 "Cache Units" */

#define dcbst(a)        asm("dcbst 0, %0\n" :: "r"(a): "r0");
#define icbi(a)         asm("icbi 0, %0\n" :: "r"(a): "r0");
#define sync()          asm("sync\n");
#define isync()         asm("isync\n");

/* Writeback/invalidate a block */
#define wbinv_block(x)  do { dcbst(x); sync(); icbi(x); } while(0)

static void flush_range(void *base, size_t size)
{
	for (size_t i = 0; i < size; i += CACHELINE)
		wbinv_block(base + i);
	isync();
}

#define CHUNK_SIZE 1024

void upx_main(void *p)
{
	boot_services svc = p;
	void (* uboot)(void) = (void *)CONFIG_SYS_UBOOT_START + 0x100;

	upx_putchar(svc, 'U');
	upx_putchar(svc, 'P');
	upx_putchar(svc, 'X');

	for (void *dst = (void *)CONFIG_SYS_UBOOT_START; ; dst += CHUNK_SIZE) {
		bool good = upx_read(svc, dst, CHUNK_SIZE);
		upx_putchar(svc, '.');
		flush_range(dst, CHUNK_SIZE);
		if (!good)
			break;
	}

	// Let's go!
	upx_putchar(svc, '\r');  // <-- necessary?
	upx_putchar(svc, '\n');
	uboot();
}
