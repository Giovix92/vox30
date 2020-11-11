/**
 * @file   
 * @author 
 * @date   2011-08-11
 * @brief  
 *
 * Copyright - 2011 SerComm Corporation. All Rights Reserved.
 * SerComm Corporation reserves the right to make changes to this document without notice.
 * SerComm Corporation makes no warranty, representation or guarantee regarding the suitability
 * of its products for any particular purpose. SerComm Corporation assumes no liability arising
 * out of the application or use of any product or circuit. SerComm Corporation specifisally
 * disclaims any and all liability, including without limitation consequential or incidental damages;
 * neither does it convey any license under its patent rights, nor the rights of others.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <linux/limits.h>

#define MAX_LOCK_WAIT	10

int _nvram_lock(const char *path)
{
	char lock_path[PATH_MAX];
	char *p;
	int fd;
    char pid[16] = {0};

	snprintf(lock_path, sizeof(lock_path),"/var/lock/%s.lock", path);
	p = lock_path + sizeof("/var/lock/");
	while (*p) {
		if (*p == '/') *p = '_';
		p++;
	}
	fd = open(lock_path,O_WRONLY|O_CREAT|O_EXCL,0644);
	if (fd < 0 && errno == EEXIST) {
		return -1;		
	} else if (fd < 0){
		return -1;
	}
    snprintf(pid,sizeof(pid), "%d", getpid());
    pid[sizeof(pid)-1]= '\0';
    write(fd, pid, sizeof(pid));
	close(fd);
	return 0;
}
int _nvram_unlock(const char *path)
{
	char lock_path[PATH_MAX];
	char *p;
	snprintf(lock_path, sizeof(lock_path),"/var/lock/%s.lock", path);
	p = lock_path + sizeof("/var/lock/");
	while (*p) {
		if (*p == '/') *p = '_';
		p++;
	}
	if (unlink(lock_path) < 0) {
		return -1;
	}		
	return 0;
}
int check_is_process_exsit(const char *path)
{
	char lock_path[PATH_MAX];
	char *p;
    FILE *fp = NULL;
    char buf[32] = "";
    pid_t pid;
    int fd = -1;
    struct flock flockptr = {.l_type = F_RDLCK,
                             .l_start = 0,
                             .l_whence = SEEK_SET,
                             .l_len = 0
    };
	snprintf(lock_path, sizeof(lock_path),"/var/lock/%s.lock", path);
	p = lock_path + sizeof("/var/lock/");
	while (*p) {
		if (*p == '/') *p = '_';
		p++;
	}

    fp = fopen(lock_path, "r+");
    if(!fp)
    {
        return 0;
    }
 	fd = fileno(fp);
    if (fd < 0)
    {
        fclose(fp);
    	return 0;
    }

    if (fcntl(fd, F_SETLKW, &flockptr) < 0)
    {
        fclose(fp);
    	return 0;
    }
    if(!fgets(buf, sizeof(buf), fp))
    {
        flockptr.l_type = F_UNLCK;
        fcntl(fd, F_SETLKW, &flockptr);
        fclose(fp);
        return 0;
    }


    pid = atoi(buf);
    memset(buf, 0, sizeof(buf));
    snprintf(buf, sizeof(buf), "/proc/%d/cmdline", pid);

    if(access(buf, F_OK) == 0)
    {
        flockptr.l_type = F_UNLCK;
        fcntl(fd, F_SETLKW, &flockptr);
        fclose(fp);
        return 1;
    }
    snprintf(buf,sizeof(buf), "%d", getpid());
    buf[sizeof(buf)-1]= '\0';
    fseek(fp, 0, SEEK_SET);
    fwrite(buf, sizeof(buf), 1, fp);
    flockptr.l_type = F_UNLCK;
    fcntl(fd, F_SETLKW, &flockptr);
    fclose(fp);
    return 0;
}
int nvram_lock(const char *path)
{
	int i=0;

	while (i++ < MAX_LOCK_WAIT) {		
		if(_nvram_lock(path) == 0)
			return 0;
		else			
			usleep(500000);	
	}
    if(!check_is_process_exsit(path))
    {
        return 0;
    }
    return -1;  
}
/*nvram file unlock*/
int nvram_unlock(const char *path)
{
	int i=0;

	while (i++ < MAX_LOCK_WAIT) {		
		if(_nvram_unlock(path) == 0)
			return 0;
		else			
			usleep(500000);	
	}
	return -1;  
}



