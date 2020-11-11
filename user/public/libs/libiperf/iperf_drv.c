#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "iperf_drv.h"


static int open_sc_drv_dev(void)
{
    return open(SC_DRV_FD_NAME_STR, O_RDWR);
}

static int close_sc_drv_dev(int fd)
{
    return close(fd);
}
int util_scDrv_set_iperf_create_stream_info(IPERF_STREAM_INFO *information)
{
    int fd;
    int ret;

    if ((fd = open_sc_drv_dev()) == -1)
    {
        return -1;
    }
    ret = ioctl(fd, SC_DRV_IOCTL_SET_IPERF_CREATE_STREAM, information);
    close_sc_drv_dev(fd);
    
    if (ret < 0)
    {
        return -1;
    }
    return 0;
}
int util_scDrv_get_iperf_create_stream_state(IPERF_STREAM_INFO *information)
{
    int fd;
    int ret;

    if ((fd = open_sc_drv_dev()) == -1)
    {
        return -1;
    }
    ret = ioctl(fd, SC_DRV_IOCTL_GET_IPERF_CREATE_STREAM_STATE, information);
    close_sc_drv_dev(fd);
    
    if (ret < 0)
    {
        return -1;
    }
    return 0;
}
int util_scDrv_get_iperf_stream_interval(struct report *report_test)
{
    int fd;
    int ret;

    if ((fd = open_sc_drv_dev()) == -1)
    {
        return -1;
    }
    ret = ioctl(fd, SC_DRV_IOCTL_GET_IPERF_UPDATE_RESULT, report_test);
    close_sc_drv_dev(fd);
    
    if (ret < 0)
    {
        return -1;
    }
    return 0;
}
int util_scDrv_set_iperf_stream_end(void)
{
    int fd;
    int ret;

    if ((fd = open_sc_drv_dev()) == -1)
    {
        return -1;
    }
    ret = ioctl(fd, SC_DRV_IOCTL_SET_IPERF_TEST_DONE, NULL);
    close_sc_drv_dev(fd);
    
    if (ret < 0)
    {
        return -1;
    }
    return 0;
}
int util_scDrv_set_iperf_stream_run(void)
{
    int fd;
    int ret;

    if ((fd = open_sc_drv_dev()) == -1)
    {
        return -1;
    }
    ret = ioctl(fd, SC_DRV_IOCTL_SET_IPERF_TEST_RUN, NULL);
    close_sc_drv_dev(fd);
    
    if (ret < 0)
    {
        return -1;
    }
    return 0;
}
