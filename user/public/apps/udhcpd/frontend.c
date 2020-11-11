#include <string.h>

extern int udhcpd_main(int argc, char *argv[]);
extern int udhcpc_main(int argc, char *argv[]);
#ifdef CONFIG_SUPPORT_PLUME
int save_to_flash = 1;
#endif

int main(int argc, char *argv[])
{
	int ret = 0;
	char *base = strrchr(argv[0], '/');
	
#ifdef CONFIG_SUPPORT_PLUME
    if (strstr(base ? (base + 1) : argv[0], "dhcpd_plume"))
    {
        save_to_flash = 0;
        ret = udhcpd_main(argc, argv);
    }
    else
#else
#endif
    {
        if (strstr(base ? (base + 1) : argv[0], "dhcpd"))
            ret = udhcpd_main(argc, argv);
        else ret = udhcpc_main(argc, argv);
    }
	return ret;
}
