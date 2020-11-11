#include <curl/curl.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
static int  do_upload(char  filename[],char  url[] ,char  username[],char  passwd[],char retry[]);
static void usage_show();
static size_t read_callback(size_t ssid,void *ptr, size_t size, size_t nmemb, void *stream);
static size_t read_callback(size_t ssid,void *ptr, size_t size, size_t nmemb, void *stream)
{
    curl_off_t nread;
    size_t retcode = fread(ptr, size, nmemb, stream);
    

    nread = (curl_off_t)retcode;
    return retcode;
}


static int  do_upload(char  filename[],char  url[] ,char  username[],char  passwd[],char retry[])
{

    CURL *curl =  NULL;
    CURLcode res;
    FILE *hd_src = NULL;
    char err_buf[CURL_ERROR_SIZE];
    struct stat file_info;
    curl_off_t fsize;
    int i = 0;
    int retry_num =1;

    if (strlen(retry))
        retry_num = atoi(retry);
    if(stat(filename, &file_info)) {
        return 1;
    }
    fsize = (curl_off_t)file_info.st_size;

    hd_src = fopen(filename, "rb");
    if (!hd_src)
    {
        return 1;
    }

    char long_url[512];
    char * file = NULL;
    memset(long_url,0,sizeof(long_url));    
    snprintf(long_url,sizeof(long_url),"%s",url);

    if ((file = strrchr(filename,'/')))
    {
        strncat(long_url,file,sizeof(long_url));
    }
    else
    {
        strncat(long_url,filename,sizeof(long_url));
    }


    curl = curl_easy_init();
    if(curl) {
        curl_easy_setopt(curl, CURLOPT_URL, long_url);
        if (strncmp(url,"http",4) == 0)
            curl_easy_setopt(curl, CURLOPT_PUT, 1L);
        else
            curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
        if (strlen(username))
            curl_easy_setopt(curl, CURLOPT_USERNAME, username);
        if (strlen(passwd))
            curl_easy_setopt(curl, CURLOPT_PASSWORD, passwd);
        curl_easy_setopt(curl, CURLOPT_READFUNCTION, read_callback);
        curl_easy_setopt(curl, CURLOPT_READDATA, hd_src);
        curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, err_buf);
        curl_easy_setopt(curl, CURLOPT_FTP_CREATE_MISSING_DIRS,1L);
        curl_easy_setopt(curl, CURLOPT_INFILESIZE_LARGE, (curl_off_t)fsize);

        size_t len;
        for(i = 1;i<= retry_num;i++)
        {
            if ( curl_easy_perform(curl) == CURLE_OK)
            {
                break;
            }
        }

        curl_easy_cleanup(curl);
    }
    fclose(hd_src); /* close the local file */ 
    return 0;

}



int main(int argc,char *argv[])
{
    int ret = -1;
    char filepath[128] = "";
    char url[128] = "";
    char name[128] = "";
    char pw[128] = "";
    char retry[128] = "";
    int opt = 0;

    //support anonymous upload,username and passwd is optional
    while( (opt = getopt(argc, argv, "f:u:n::p::r::h")) != -1 )
    {
        switch(opt)
        {
            case 'f':
                {
                    snprintf(filepath, sizeof(filepath),"%s",optarg);
                    break;
                }
            case 'u':
                {
                    snprintf(url,sizeof(url),"%s",optarg);
                    break;
                }
            case 'n':
                {
                    snprintf(name,sizeof(name),"%s",optarg);
                    break;
                }
            case 'p':
                {
                    snprintf(pw,sizeof(pw),"%s",optarg);
                    break;
                }
            case 'r':
                {
                    snprintf(retry,sizeof(retry),"%s",optarg);
                    break;
                }
            case 'h':
                {
                    ret=0;
                }
            default:
                usage_show();
                break;
        }
    }
    ret = do_upload(filepath,url,name,pw,retry);
    return ret;
}

static void usage_show(void)
{
    printf(
            "\nUsage:curl -f [filename] | -u [url] | -n [name] | -p [passwd] | -r [retry number] | -h [ help ]\n"
            "\n  -f filename ,upload the filename\n"
            "\n  -u url,upload url(http,ftp,so on)\n"
            "\n  -n username\n"
            "\n  -p passwd\n"
            "\n  -r retry number,retry number of upload\n"
            "\n  -h help\n");
    exit(1);
}
