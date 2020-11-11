/*
	Main program.

	Michał Obrembski (byku@byku.com.pl)
*/
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <string.h>
#include "http.h"
#include "SpeedtestConfig.h"
#include "SpeedtestServers.h"
#include "Speedtest.h"
#ifdef CONFIG_SUPPORT_WEBAPI
#include "sal/sal_misc.h"
#endif
int sortServers(SPEEDTESTSERVER_T **srv1, SPEEDTESTSERVER_T **srv2)
{
    return((*srv1)->distance - (*srv2)->distance);
}

float getElapsedTime(struct timeval tval_start) {
    struct timeval tval_end, tval_diff;
    gettimeofday(&tval_end, NULL);
    tval_diff.tv_sec = tval_end.tv_sec - tval_start.tv_sec;
    tval_diff.tv_usec = tval_end.tv_usec - tval_start.tv_usec;
    if(tval_diff.tv_usec < 0) {
        --tval_diff.tv_sec;
        tval_diff.tv_usec += 1000000;
    }
    return (float)tval_diff.tv_sec + (float)tval_diff.tv_usec / 1000000;
}

void parseCmdLine(int argc, char **argv) {
    int i;
    for(i=1; i<argc; i++)
    {
        if(strcmp("--help", argv[i]) == 0 || strcmp("-h", argv[i]) == 0)
        {
            printf("Usage (options are case sensitive):\n\
            \t--help - Show this help.\n\
            \t--server URL - use server URL, don'read config.\n\
            \t--upsize SIZE - use upload size of SIZE bytes.\n"
            "\nDefault action: Get server from Speedtest.NET infrastructure\n"
            "and test download with 30MB download size and 1MB upload size.\n");
//            "and test download with 15MB download size and 1MB upload size.\n");
            exit(1);
        }
        if(strcmp("--server", argv[i]) == 0)
        {
            downloadUrl = malloc(sizeof(char) * strlen(argv[i+1]) + 1);
            strcpy(downloadUrl, argv[i + 1]);
        }
        if(strcmp("--upsize", argv[i]) == 0)
        {
            totalToBeTransfered = strtoul(argv[i + 1], NULL, 10);
        }
    }
}

void freeMem()
{
    free(downloadUrl);
    free(uploadUrl);
    free(serverList);
    free(speedTestConfig);
}

void getBestServer()
{
    speedTestConfig = getConfig();
    if (speedTestConfig == NULL)
    {
        printf("Cannot download speedtest.net configuration. Something is wrong...\n");
        freeMem();
        exit(1);
    }
    printf("Your IP: %s And ISP: %s\n",
                speedTestConfig->ip, speedTestConfig->isp);
    printf("Lat: %f Lon: %f\n", speedTestConfig->lat, speedTestConfig->lon);
    serverList = getServers(&serverCount, "http://www.speedtest.net/speedtest-servers-static.php");
    if (serverCount == 0)
    {
        // Primary server is not responding. Let's give a try with secondary one.
        serverList = getServers(&serverCount, "http://c.speedtest.net/speedtest-servers-static.php");
    }
    printf("Grabbed %d servers\n", serverCount);
    if (serverCount == 0)
    {
        printf("Cannot download any speedtest.net server. Something is wrong...\n");
        freeMem();
        exit(1);
    }
    for(i=0; i<serverCount; i++)
        serverList[i]->distance = haversineDistance(speedTestConfig->lat,
            speedTestConfig->lon,
            serverList[i]->lat,
            serverList[i]->lon);

    qsort(serverList, serverCount, sizeof(SPEEDTESTSERVER_T *),
                (int (*)(const void *,const void *)) sortServers);

    printf("Best Server URL: %s\n\t Name:%s Country: %s Sponsor: %s Dist: %ld\n",
        serverList[0]->url, serverList[0]->name, serverList[0]->country,
        serverList[0]->sponsor, serverList[0]->distance);
    downloadUrl = getServerDownloadUrl(serverList[0]->url);
    uploadUrl = malloc(sizeof(char) * strlen(serverList[0]->url) + 1);
    strcpy(uploadUrl, serverList[0]->url);

    for(i=0; i<serverCount; i++){
        free(serverList[i]->url);
        free(serverList[i]->name);
        free(serverList[i]->sponsor);
        free(serverList[i]->country);
        free(serverList[i]);
    }
}

void testDownload(const char *url)
{
    /* Testing download... */
#ifdef CONFIG_SUPPORT_WEBAPI
	float down_speed;
	char buf[16];
#endif
    sockId = httpGetRequestSocket(url);
    if(sockId == 0)
    {
        printf("Unable to open socket for Download!");
        freeMem();
        exit(1);
    }
    size = -1;
    totalTransfered = 0;
    gettimeofday(&tval_start, NULL);
    while(size != 0)
    {
        size = httpRecv(sockId, buffer, 1500);
        totalTransfered += size;
    }
    elapsedSecs = getElapsedTime(tval_start);
    speed = (totalTransfered / elapsedSecs) / 1024;
    httpClose(sockId);
    printf("Bytes %lu downloaded in %.2f seconds %.2f kB/s\n",
        totalTransfered, elapsedSecs, speed);
#ifdef CONFIG_SUPPORT_WEBAPI
	down_speed = (speed * 8) / 1024;
	sprintf(buf, "%f", down_speed);
	sal_misc_set_speed_test_download_speed(buf);
#endif
}

void testUpload(const char *url)
{
    /* Testing upload... */
#ifdef CONFIG_SUPPORT_WEBAPI
	float up_speed;
	char buf[16];
#endif
    totalTransfered = totalToBeTransfered;
    sockId = httpPutRequestSocket(uploadUrl, totalToBeTransfered);
    if(sockId == 0)
    {
        printf("Unable to open socket for Upload!");
        freeMem();
        exit(1);
    }
    gettimeofday(&tval_start, NULL);
    while(totalTransfered != 0)
    {
        for(i=0; i < 1500; i++)
            buffer[i] = (char)i;
        size = httpSend(sockId, buffer, 1500);
        /* To check for unsigned overflow */
        if(totalTransfered < size)
            break;
        totalTransfered -= size;
    }
    elapsedSecs = getElapsedTime(tval_start);
    speed = (totalToBeTransfered / elapsedSecs) / 1024;
    httpClose(sockId);
    printf("Bytes %lu uploaded in %.2f seconds %.2f kB/s\n",
        totalToBeTransfered, elapsedSecs, speed);
#ifdef CONFIG_SUPPORT_WEBAPI
	up_speed = (speed * 8) / 1024;
	sprintf(buf, "%f", up_speed);
	sal_misc_set_speed_test_upload_speed(buf);
#endif
}

#ifdef CONFIG_SUPPORT_WEBAPI
char *server_url = NULL;
#endif
int main(int argc, char **argv)
{
#ifdef CONFIG_SUPPORT_WEBAPI
	sal_misc_set_speed_test_status("1");
#endif
    speedTestConfig = NULL;
    parseCmdLine(argc, argv);

    if(downloadUrl == NULL)
    {
        getBestServer();
    }
    else
    {
        uploadUrl = downloadUrl;
        tmpUrl = malloc(sizeof(char) * strlen(downloadUrl) + 1);
        strcpy(tmpUrl, downloadUrl);
        downloadUrl = getServerDownloadUrl(tmpUrl);
        free(tmpUrl);
    }
#ifdef CONFIG_SUPPORT_WEBAPI
    uploadUrl = server_url;
#endif
    testDownload(downloadUrl);
    testUpload(uploadUrl);
#ifdef CONFIG_SUPPORT_WEBAPI
	free(server_url);	
	system("/usr/sbin/notification 7 true & 2> /dev/null");
	sal_misc_set_speed_test_status("0");
#endif
    freeMem();
    return 0;
}

