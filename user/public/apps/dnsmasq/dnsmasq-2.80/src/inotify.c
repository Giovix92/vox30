/* dnsmasq is Copyright (c) 2000-2018 Simon Kelley
 
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; version 2 dated June, 1991, or
   (at your option) version 3 dated 29 June, 2007.
 
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
     
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "dnsmasq.h"
#ifdef HAVE_INOTIFY

#include <sys/inotify.h>
#include <sys/param.h> /* For MAXSYMLINKS */

/* the strategy is to set an inotify on the directories containing
   resolv files, for any files in the directory which are close-write 
   or moved into the directory.
   
   When either of those happen, we look to see if the file involved
   is actually a resolv-file, and if so, call poll-resolv with
   the "force" argument, to ensure it's read.

   This adds one new error condition: the directories containing
   all specified resolv-files must exist at start-up, even if the actual
   files don't. 
*/

static char *inotify_buffer;
#define INOTIFY_SZ (sizeof(struct inotify_event) + NAME_MAX + 1)

/* If path is a symbolic link, return the path it
   points to, made absolute if relative.
   If path doesn't exist or is not a symlink, return NULL.
   Return value is malloc'ed */
static char *my_readlink(char *path)
{
  ssize_t rc, size = 64;
  char *buf;

  while (1)
    {
      buf = safe_malloc(size);
      rc = readlink(path, buf, (size_t)size);
      
      if (rc == -1)
	{
	  /* Not link or doesn't exist. */
	  if (errno == EINVAL || errno == ENOENT)
	    {
	      free(buf);
	      return NULL;
	    }
	  else
	    die(_("cannot access path %s: %s"), path, EC_MISC);
	}
      else if (rc < size-1)
	{
	  char *d;
	  
	  buf[rc] = 0;
	  if (buf[0] != '/' && ((d = strrchr(path, '/'))))
	    {
	      /* Add path to relative link */
	      char *new_buf = safe_malloc((d - path) + strlen(buf) + 2);
	      *(d+1) = 0;
	      strcpy(new_buf, path);
	      strcat(new_buf, buf);
	      free(buf);
	      buf = new_buf;
	    }
	  return buf;
	}

      /* Buffer too small, increase and retry */
      size += 64;
      free(buf);
    }
}

void inotify_dnsmasq_init()
{
  struct resolvc *res;
  inotify_buffer = safe_malloc(INOTIFY_SZ);
  daemon->inotifyfd = inotify_init1(IN_NONBLOCK | IN_CLOEXEC);
  
  if (daemon->inotifyfd == -1)
    die(_("failed to create inotify: %s"), NULL, EC_MISC);

  if (option_bool(OPT_NO_RESOLV))
    return;
  
  for (res = daemon->resolv_files; res; res = res->next)
    {
      char *d, *new_path, *path = safe_malloc(strlen(res->name) + 1);
      int links = MAXSYMLINKS;

      strcpy(path, res->name);

      /* Follow symlinks until we reach a non-symlink, or a non-existent file. */
      while ((new_path = my_readlink(path)))
	{
	  if (links-- == 0)
	    die(_("too many symlinks following %s"), res->name, EC_MISC);
	  free(path);
	  path = new_path;
	}

      res->wd = -1;

      if ((d = strrchr(path, '/')))
	{
	  *d = 0; /* make path just directory */
	  res->wd = inotify_add_watch(daemon->inotifyfd, path, IN_CLOSE_WRITE | IN_MOVED_TO);

	  res->file = d+1; /* pointer to filename */
	  *d = '/';
	  
	  if (res->wd == -1 && errno == ENOENT)
	    die(_("directory %s for resolv-file is missing, cannot poll"), res->name, EC_MISC);
	}	  
	 
      if (res->wd == -1)
	die(_("failed to create inotify for %s: %s"), res->name, EC_MISC);
	
    }
}


/* initialisation for dynamic-dir. Set inotify watch for each directory, and read pre-existing files */
void set_dynamic_inotify(int flag, int total_size, struct crec **rhash, int revhashsz)
{
  struct hostsfile *ah;
 #ifdef __SC_BUILD__
	struct servers_dir *sd;
	sd = daemon->dynamic_servers_dir;
	DIR *servers_dir_stream = NULL;
	struct dirent *servers_ent;
	struct stat servers_buf;
	char servers_file_path[200] = "";
	/*below: conf-dir option inotify_add_watch*/
	struct conf_dir *cd;
	cd = daemon->dynamic_conf_dir;
	DIR *conf_dir_stream = NULL;
	struct dirent *conf_ent;
	struct stat conf_buf;
	char conf_file_path[200] = "";
	if (stat(cd->fname, &conf_buf) == -1 || !(S_ISDIR(conf_buf.st_mode))) {
		my_syslog(LOG_ERR, _("bad dynamic conf directory %s: %s"),
		          cd->fname, strerror(errno));
	}
	if (!(cd->flags & AH_WD_DONE)) {
        my_syslog(LOG_INFO, "set dynamic config file %s", cd->fname);
		cd->wd = inotify_add_watch(daemon->inotifyfd, cd->fname, IN_CLOSE_WRITE | IN_MOVE | IN_MODIFY | IN_DELETE | IN_CREATE);
		cd->flags |= AH_WD_DONE;
	}
	/*
	if (!(conf_dir_stream= opendir(cd->fname)))
	{
	return;
	}
	while((conf_ent= readdir(conf_dir_stream)) != NULL){
	 if(!strcmp(conf_ent->d_name,".") || !strcmp(conf_ent->d_name,".."))
	     continue;
	 sprintf(conf_file_path,"%s/%s",cd->fname,conf_ent->d_name);
	 option_read_dynfile(conf_file_path,CONF_DIR_OPT);
	}
	closedir(conf_dir_stream);*/
#endif
 
  for (ah = daemon->dynamic_dirs; ah; ah = ah->next)
    {
      DIR *dir_stream = NULL;
      struct dirent *ent;
      struct stat buf;
     
      if (!(ah->flags & flag))
	continue;
 
      if (stat(ah->fname, &buf) == -1 || !(S_ISDIR(buf.st_mode)))
	{
	  my_syslog(LOG_ERR, _("bad dynamic directory %s: %s"), 
		    ah->fname, strerror(errno));
	  continue;
	}
      
       if (!(ah->flags & AH_WD_DONE))
	 {
#ifdef __SC_BUILD__
       ah->wd = inotify_add_watch(daemon->inotifyfd, ah->fname, IN_CLOSE_WRITE | IN_MOVE | IN_MODIFY | IN_DELETE | IN_CREATE);
#else
	   ah->wd = inotify_add_watch(daemon->inotifyfd, ah->fname, IN_CLOSE_WRITE | IN_MOVED_TO);
#endif
	   ah->flags |= AH_WD_DONE;
	 }

       /* Read contents of dir _after_ calling add_watch, in the hope of avoiding
	  a race which misses files being added as we start */
       if (ah->wd == -1 || !(dir_stream = opendir(ah->fname)))
	 {
	   my_syslog(LOG_ERR, _("failed to create inotify for %s: %s"),
		     ah->fname, strerror(errno));
	   continue;
	 }

       while ((ent = readdir(dir_stream)))
	 {
	   size_t lendir = strlen(ah->fname);
	   size_t lenfile = strlen(ent->d_name);
	   char *path;
	   
	   /* ignore emacs backups and dotfiles */
	   if (lenfile == 0 || 
	       ent->d_name[lenfile - 1] == '~' ||
	       (ent->d_name[0] == '#' && ent->d_name[lenfile - 1] == '#') ||
	       ent->d_name[0] == '.')
	     continue;
	   
	   if ((path = whine_malloc(lendir + lenfile + 2)))
	     {
	       strcpy(path, ah->fname);
	       strcat(path, "/");
	       strcat(path, ent->d_name);
	       
	       /* ignore non-regular files */
	       if (stat(path, &buf) != -1 && S_ISREG(buf.st_mode))
		 {
		   if (ah->flags & AH_HOSTS)
		     total_size = read_hostsfile(path, ah->index, total_size, rhash, revhashsz);
#ifdef HAVE_DHCP
		   else if (ah->flags & (AH_DHCP_HST | AH_DHCP_OPT))
		     option_read_dynfile(path, ah->flags);
#endif		   
		 }

	       free(path);
	     }
	 }

       closedir(dir_stream);
    }
}

int inotify_check(time_t now)
{
  int hit = 0;
  struct hostsfile *ah;
#ifdef __SC_BUILD__
	struct servers_dir *sd;
	char servers_file_path[200]="";
	sd=daemon->dynamic_servers_dir;

	/*inotify conf-dir*/
	struct conf_dir *cd;
	char conf_file_path[200]="";
	cd=daemon->dynamic_conf_dir;
#endif

  while (1)
    {
      int rc;
      char *p;
      struct resolvc *res;
      struct inotify_event *in;

      while ((rc = read(daemon->inotifyfd, inotify_buffer, INOTIFY_SZ)) == -1 && errno == EINTR);
      
      if (rc <= 0)
	break;
      
      for (p = inotify_buffer; rc - (p - inotify_buffer) >= (int)sizeof(struct inotify_event); p += sizeof(struct inotify_event) + in->len) 
	{
	  size_t namelen;

	  in = (struct inotify_event*)p;
	  
	  /* ignore emacs backups and dotfiles */
	  if (in->len == 0 || (namelen = strlen(in->name)) == 0 ||
	      in->name[namelen - 1] == '~' ||
	      (in->name[0] == '#' && in->name[namelen - 1] == '#') ||
	      in->name[0] == '.')
	    continue;

#ifdef __SC_BUILD__
			if (sd->wd == in->wd) {
				my_syslog(LOG_INFO, _("inotify,enter into sercomm add inotify for server file"));
				strcpy(servers_file_path, sd->fname);
				strcat(servers_file_path, "/");
				strcat(servers_file_path, in->name);
				if (strcmp(servers_file_path,DNS_DEFAULT_WAN_CFG) == 0) {
					//daemon->soa_sn++; /* Bump zone serial, as it may have changed. */
					/*
					 * clear_cache_and_reload(now);
					if (daemon->resolv_files && option_bool(OPT_NO_POLL))
					    reload_servers(daemon->resolv_files->name);
					*/
					cleanup_servers_by_domain_type(SERV_OPT_FROM_DATA_CONF);
					option_read_dynfile(servers_file_path,SERVERS_OPT);
					//check_servers();
				}
				if (strcmp(servers_file_path,DNS_DEFAULT_WAN_MAN_CFG) == 0) {
					//daemon->soa_sn++; /* Bump zone serial, as it may have changed. */
					/*
					 * clear_cache_and_reload(now);
					if (daemon->resolv_files && option_bool(OPT_NO_POLL))
					    reload_servers(daemon->resolv_files->name);
					*/
					cleanup_servers_by_domain_type(SERV_OPT_FROM_DATA_MAN_CONF);
					option_read_dynfile(servers_file_path,SERVERS_OPT);
					//check_servers();
				}
				if (strcmp(servers_file_path,DNS_NTP_WAN_CFG) == 0) {
					//daemon->soa_sn++; /* Bump zone serial, as it may have changed. */
					/*
					clear_cache_and_reload(now);
					if (daemon->resolv_files && option_bool(OPT_NO_POLL))
					    reload_servers(daemon->resolv_files->name);
					*/
					cleanup_servers_by_domain_type(SERV_OPT_FROM_NTP_CONF);
					option_read_dynfile(servers_file_path,SERVERS_OPT);
					//check_servers();
				}
				if (strcmp(servers_file_path,DNS_VOIP_WAN_CFG) == 0) {
					//daemon->soa_sn++; /* Bump zone serial, as it may have changed. */
					/*
					clear_cache_and_reload(now);
					if (daemon->resolv_files && option_bool(OPT_NO_POLL))
					    reload_servers(daemon->resolv_files->name);
					*/
					cleanup_servers_by_domain_type(SERV_OPT_FROM_VOIP_CONF);
					option_read_dynfile(servers_file_path,SERVERS_OPT);
					//check_servers();
				}
				if (strcmp(servers_file_path,DNS_IPTV_WAN_CFG) == 0) {
					//daemon->soa_sn++; /* Bump zone serial, as it may have changed. */
					/*
					clear_cache_and_reload(now);
					if (daemon->resolv_files && option_bool(OPT_NO_POLL))
					    reload_servers(daemon->resolv_files->name);
					*/
					cleanup_servers_by_domain_type(SERV_OPT_FROM_IPTV_CONF);
					option_read_dynfile(servers_file_path,SERVERS_OPT);
					//check_servers();
				}
				if (strcmp(servers_file_path,DNS_TR069_WAN_CFG) == 0) {
					//daemon->soa_sn++; /* Bump zone serial, as it may have changed. */
					/*
					clear_cache_and_reload(now);
					if (daemon->resolv_files && option_bool(OPT_NO_POLL))
					    reload_servers(daemon->resolv_files->name);
					*/
					cleanup_servers_by_domain_type(SERV_OPT_FROM_TR069_CONF);
					option_read_dynfile(servers_file_path,SERVERS_OPT);
					//check_servers();
				}
                if (strcmp(servers_file_path,DNS_IPPHONE_WAN_CFG) == 0) {
					//daemon->soa_sn++; /* Bump zone serial, as it may have changed. */
					/*
					clear_cache_and_reload(now);
					if (daemon->resolv_files && option_bool(OPT_NO_POLL))
					    reload_servers(daemon->resolv_files->name);
					*/
					cleanup_servers_by_domain_type(SERV_OPT_FROM_IPPHONE_CONF);
					option_read_dynfile(servers_file_path,SERVERS_OPT);
					//check_servers();
				}

			}
			if (cd->wd == in->wd) {

				strcpy(conf_file_path, cd->fname);
				strcat(conf_file_path, "/");
				strcat(conf_file_path, in->name);
				if (strcmp(conf_file_path,DNS_REDIRECT_CFG) == 0) {
					my_syslog(LOG_INFO, _("inotify, sercomm add %s"), conf_file_path);
					cleanup_servers_by_flag(SERV_LITERAL_ADDRESS);
					if (access(DNS_REDIRECT_CFG, F_OK) == 0) {
						redirect = 1;
						FILE *fp = NULL;
						int fd = 0;
                        char buf[256] ={0};
						fd = util_lock(DNSMASQ_CFG_LOCK, 0);
						fp = fopen(DNS_REDIRECT_CFG,"r");
						if (!fp) {
							util_unlock(fd);
							return -1;
						}
                        while(fgets(buf,256,fp) != NULL)
                        {
#ifdef HAVE_IPV6
                            if(strstr(buf,"ipv6"))
                            {
                                snprintf(daemon->redirect_ipv6,256,"%s",buf);
                            }
                            else
#endif
                            {
                                snprintf(daemon->redirect_ip,100,"%s",buf);
                            }
                        }
						fclose(fp);
						util_unlock(fd);
						//option_read_dynfile(conf_file_path,CONF_DIR_OPT);
					} else {
						redirect = 0;
					}
				}
				if (strcmp(conf_file_path,DNS_NAMESRV_CFG) == 0) {
					my_syslog(LOG_INFO, _("inotify, sercomm add %s"), conf_file_path);
                    cleanup_servers_by_domain_type(SERV_OPT_FROM_NAME_SRV);
					option_read_dynfile(conf_file_path,CONF_DIR_OPT);
                }
			}
#endif

	  for (res = daemon->resolv_files; res; res = res->next)
	    if (res->wd == in->wd && strcmp(res->file, in->name) == 0)
	      hit = 1;

	  for (ah = daemon->dynamic_dirs; ah; ah = ah->next)
	    if (ah->wd == in->wd)
	      {
		size_t lendir = strlen(ah->fname);
		char *path;
		
		if ((path = whine_malloc(lendir + in->len + 2)))
		  {
		    strcpy(path, ah->fname);
		    strcat(path, "/");
		    strcat(path, in->name);
		     
		    my_syslog(LOG_INFO, _("inotify, new or changed file %s"), path);

		    if (ah->flags & AH_HOSTS)
		      {
			read_hostsfile(path, ah->index, 0, NULL, 0);
#ifdef HAVE_DHCP
			if (daemon->dhcp || daemon->doing_dhcp6) 
			  {
			    /* Propagate the consequences of loading a new dhcp-host */
			    dhcp_update_configs(daemon->dhcp_conf);
			    lease_update_from_configs(); 
			    lease_update_file(now); 
			    lease_update_dns(1);
			  }
#endif
		      }
#ifdef HAVE_DHCP
		    else if (ah->flags & AH_DHCP_HST)
		      {
			if (option_read_dynfile(path, AH_DHCP_HST))
			  {
			    /* Propagate the consequences of loading a new dhcp-host */
			    dhcp_update_configs(daemon->dhcp_conf);
			    lease_update_from_configs(); 
			    lease_update_file(now); 
			    lease_update_dns(1);
			  }
		      }
		    else if (ah->flags & AH_DHCP_OPT)
		      option_read_dynfile(path, AH_DHCP_OPT);
#endif
		    
		    free(path);
		  }
	      }
	}
    }
  return hit;
}

#endif  /* INOTIFY */
  
