static long (*boot_services)(int svc, ...);

static void upx_putchar(char c)
{
	boot_services(0x23, c);
}

static void upx_putstr(char *s)
{
	while (*s)
		upx_putchar(*s++);
}

static void upx_puts(char *s)
{
	upx_putstr(s);
	upx_putchar('\n');
}

static bool upx_read(void *p, size_t size)
{
	return boot_services(0x51, p, size);
}

static void upx_close(void)
{
	boot_services(0x52);
}

void upx_main(void *p)
{
	boot_services = p;

	upx_puts("LANCOM UPX shim running.");
}
