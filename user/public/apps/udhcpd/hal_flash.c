
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <utility.h>


#define DHCP_LEASE_MAX_FLASH_SIZE (24*1024)
#define DHCP_LEASE_FLASH_DATA_PATH "/mnt/1/dhcp_lease_info"

#define DHCP_LEASE_MAGIC_NUMBER 0x20111228

typedef struct lease_flash_header_s
{
    unsigned long magic;
    unsigned long data_len;
    unsigned long data_crc;
    unsigned long reserved;
}lease_flash_header_t;


static int write_data_to_file(char *buf, unsigned long buf_len, char *path)
{
    int fd = -1;
    struct flock flockptr = {
        .l_type = F_WRLCK,
        .l_start = 0,
        .l_whence = SEEK_SET,
        .l_len = 0
    };

    if (!buf)
    {
        goto _error;
    }

    if (!path)
    {
        goto _error;
    }

    if ((fd = open(path, O_RDWR|O_CREAT|O_TRUNC, S_IRUSR|S_IWUSR)) < 0)
    {
        goto _error;
    }

    if (fcntl(fd, F_SETLK, &flockptr) < 0)
    {
        goto _error;
    }

    if (write(fd, buf, buf_len) != (long)buf_len)
    {
        goto _error2;
    }

    flockptr.l_type = F_UNLCK;
    fcntl(fd, F_SETLK, &flockptr);
    close(fd);
    return 0;

_error2:
    flockptr.l_type = F_UNLCK;
    fcntl(fd, F_SETLK, &flockptr);
_error:
    if (fd >= 0)
        close(fd);
    return -1;
}

static int read_data_from_file(char **buf, unsigned long *buf_len, char *path, unsigned long header_len)
{
    struct stat file_stat;
    int fd = -1;
    struct flock flockptr = {
        .l_type = F_WRLCK,
        .l_start = 0,
        .l_whence = SEEK_SET,
        .l_len = 0
    };

    *buf = NULL;
    *buf_len = 0;

    if (!path)
    {
        goto _error;
    }

    if ((fd = open(path, O_RDWR)) < 0)
    {
        goto _error;
    }

    if (fcntl(fd, F_SETLK, &flockptr) < 0)
    {
        goto _error;
    }

    if (stat(path, &file_stat) < 0)
    {
        goto _error2;
    }

    *buf = (char *)malloc(header_len + file_stat.st_size);
    if (*buf == NULL)
    {
        goto _error2;
    }

    if (read(fd, (*buf)+header_len, file_stat.st_size) != file_stat.st_size)
    {
        goto _error2;
    }

    *buf_len = header_len + file_stat.st_size;

    flockptr.l_type = F_UNLCK;
    fcntl(fd, F_SETLK, &flockptr);
    close(fd);
    return 0;

_error2:
    flockptr.l_type = F_UNLCK;
    fcntl(fd, F_SETLK, &flockptr);
_error:
    if (*buf)
    {
        free(*buf);
        *buf = NULL;
    }
    if (fd >= 0)
        close(fd);
    return -1;
}

static int write_data_to_flash(char *buf, unsigned long buf_len, char *path)
{
    return write_data_to_file(buf, buf_len, path);
}

static int read_data_from_flash(char **buf, unsigned long *buf_len, char *path)
{
    return read_data_from_file(buf, buf_len, path, 0);
}

unsigned long _util_crc32(char *data, int length)
{
	unsigned long crc, poly;
	long crcTable[256];
	int i, j;
	poly = 0xEDB88320L;
	for (i=0; i<256; i++) {
		crc = i;
		for (j=8; j>0; j--) {
			if (crc&1) {
				crc = (crc >> 1) ^ poly;
			} else {
				crc >>= 1;
			}
		}
		crcTable[i] = crc;
	}
	crc = 0xFFFFFFFF;

	while( length-- > 0) {
		crc = ((crc>>8) & 0x00FFFFFF) ^ crcTable[ (crc^((char)*(data++))) & 0xFF ];
	}
	
	return crc^0xFFFFFFFF;
}
int hal_flash_store_dhcp_lease_info(char *data_file, int ifid)
{
    lease_flash_header_t header;
    int header_len = sizeof(lease_flash_header_t);
    char *buf = NULL;
    unsigned long buf_len = 0;
    char lease_file[128];

    if(ifid == 0)
        snprintf(lease_file, sizeof(lease_file), "%s", DHCP_LEASE_FLASH_DATA_PATH);
    else
        snprintf(lease_file, sizeof(lease_file), "%s.%d", DHCP_LEASE_FLASH_DATA_PATH, ifid);

    if (read_data_from_file(&buf, &buf_len, data_file, header_len) < 0)
    {
        goto _error;
    }

    memset(&header, 0, header_len);
    header.magic = DHCP_LEASE_MAGIC_NUMBER;
    header.data_len = buf_len - header_len;
    header.data_crc = _util_crc32(buf+header_len, header.data_len);
    memcpy(buf, &header, header_len);

    if (write_data_to_flash(buf, buf_len, lease_file) < 0)
    {
        goto _error;
    }

    free(buf);
    return 0;

_error:
    if (buf)
        free(buf);
    return -1;
}

int hal_flash_load_dhcp_lease_info(char *data_file, int ifid)
{
    lease_flash_header_t *p_header;
    int header_len = sizeof(lease_flash_header_t);
    char *buf = NULL;
    unsigned long buf_len = 0;

    char lease_file[128];

    if(ifid == 0)
        snprintf(lease_file, sizeof(lease_file), "%s", DHCP_LEASE_FLASH_DATA_PATH);
    else
        snprintf(lease_file, sizeof(lease_file), "%s.%d", DHCP_LEASE_FLASH_DATA_PATH, ifid);

    if (read_data_from_flash(&buf, &buf_len, lease_file) < 0)
    {
        goto _error;
    }

    p_header = (lease_flash_header_t *)buf;

    if (p_header->magic != DHCP_LEASE_MAGIC_NUMBER)
    {
        goto _error;
    }
    if ((p_header->data_len + header_len) != buf_len)
    {
        goto _error;
    }
    if (p_header->data_crc != _util_crc32(buf+header_len, p_header->data_len))
    {
        goto _error;
    }

    if (write_data_to_file(buf+header_len, p_header->data_len, data_file) < 0)
    {
        goto _error;
    }

    free(buf);
    return 0;

_error:
    if (buf)
        free(buf);
    return -1;
}

int hal_flash_get_dhcp_lease_info_max_size(void)
{
    return (DHCP_LEASE_MAX_FLASH_SIZE - sizeof(lease_flash_header_t));
}






