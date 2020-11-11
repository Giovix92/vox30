/**
 * ntfsmount - Part of the Linux-NTFS project.
 *
 * Copyright (c) 2005-2007 Yura Pakhuchiy
 * Copyright (c)      2005 Yuval Fledel
 * Copyright (c)      2006 Szabolcs Szakacsits
 *
 * Userspace read/write NTFS driver.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program (in the main directory of the Linux-NTFS
 * distribution in the file COPYING); if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "config.h"

#include <fuse.h>
#ifdef HAVE_STDIO_H
#include <stdio.h>
#endif
#ifdef HAVE_STRING_H
#include <string.h>
#endif
#ifdef HAVE_ERRNO_H
#include <errno.h>
#endif
#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif
#ifdef HAVE_LOCALE_H
#include <locale.h>
#endif
#include <signal.h>
#ifdef HAVE_LIMITS_H
#include <limits.h>
#endif
#ifdef HAVE_PWD_H
#include <pwd.h>
#endif
#ifdef HAVE_GETOPT_H
#include <getopt.h>
#endif
#ifdef HAVE_SYSLOG_H
#include <syslog.h>
#endif

#ifdef HAVE_SETXATTR
#include <sys/xattr.h>
#endif

#include "attrib.h"
#include "inode.h"
#include "volume.h"
#include "dir.h"
#include "unistr.h"
#include "layout.h"
#include "index.h"
#include "utils.h"
#include "version.h"
#include "ntfstime.h"
#include "support.h"

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

#define NTFS_FUSE_GET_NA(fi) \
	ntfs_attr *na = (ntfs_attr *)(uintptr_t)(fi)->fh

typedef struct {
	fuse_fill_dir_t filler;
	void *buf;
} ntfs_fuse_fill_context_t;

typedef enum {
	NF_STREAMS_INTERFACE_NONE,	/* No access to named data streams. */
	NF_STREAMS_INTERFACE_XATTR,	/* Map named data streams to xattrs. */
	NF_STREAMS_INTERFACE_WINDOWS,	/* "file:stream" interface. */
} ntfs_fuse_streams_interface;

typedef struct {
	ntfs_volume *vol;
	char *mnt_point;
	char *device;
	char *locale;
	unsigned int uid;
	unsigned int gid;
	unsigned int fmask;
	unsigned int dmask;
	ntfs_fuse_streams_interface streams;
	BOOL ro;
	BOOL silent;
	BOOL force;
	BOOL debug;
	BOOL no_detach;
	BOOL quiet;
	BOOL verbose;
	BOOL no_def_opts;
	BOOL case_insensitive;
	BOOL noatime;
	BOOL relatime;
	BOOL blkdev;
	char cached_path[PATH_MAX];
	ntfs_inode *cached_ni;
	ntfs_inode *root_ni;
} ntfs_fuse_context_t;

#define NTFS_FUSE_OPT(t, p) { t, offsetof(ntfs_fuse_context_t, p), TRUE }
#define NTFS_FUSE_OPT_NEG(t, p) { t, offsetof(ntfs_fuse_context_t, p), FALSE }
#define NTFS_FUSE_OPT_VAL(t, p, v) { t, offsetof(ntfs_fuse_context_t, p), v }

enum {
	NF_KEY_HELP,
	NF_KEY_UMASK,
};

static const struct fuse_opt ntfs_fuse_opts[] = {
	NTFS_FUSE_OPT("-v", verbose),
	NTFS_FUSE_OPT("--verbose", verbose),
	NTFS_FUSE_OPT("-q", quiet),
	NTFS_FUSE_OPT("--quiet", quiet),
	NTFS_FUSE_OPT("force", force),
	NTFS_FUSE_OPT("silent", silent),
	NTFS_FUSE_OPT("ro", ro),
	NTFS_FUSE_OPT("fake_rw", ro),
	NTFS_FUSE_OPT("debug", debug),
	NTFS_FUSE_OPT("no_detach", no_detach),
	NTFS_FUSE_OPT("no_def_opts", no_def_opts),
	NTFS_FUSE_OPT("case_insensitive", case_insensitive),
	NTFS_FUSE_OPT("noatime", noatime),
	NTFS_FUSE_OPT("relatime", relatime),
	NTFS_FUSE_OPT("fmask=%o", fmask),
	NTFS_FUSE_OPT("dmask=%o", dmask),
	NTFS_FUSE_OPT("umask=%o", fmask),
	NTFS_FUSE_OPT("uid=%d", uid),
	NTFS_FUSE_OPT("gid=%d", gid),
	NTFS_FUSE_OPT("locale=%s", locale),
	NTFS_FUSE_OPT_NEG("nosilent", silent),
	NTFS_FUSE_OPT_NEG("rw", ro),
	NTFS_FUSE_OPT_NEG("noblkdev", blkdev),
	NTFS_FUSE_OPT_NEG("atime", noatime),
	NTFS_FUSE_OPT_NEG("norelatime", relatime),
	NTFS_FUSE_OPT_VAL("streams_interface=none", streams,
			NF_STREAMS_INTERFACE_NONE),
	NTFS_FUSE_OPT_VAL("streams_interface=windows", streams,
			NF_STREAMS_INTERFACE_WINDOWS),
	NTFS_FUSE_OPT_VAL("streams_interface=xattr", streams,
			NF_STREAMS_INTERFACE_XATTR),
	FUSE_OPT_KEY("-h", NF_KEY_HELP),
	FUSE_OPT_KEY("-?", NF_KEY_HELP),
	FUSE_OPT_KEY("--help", NF_KEY_HELP),
	FUSE_OPT_KEY("umask=", NF_KEY_UMASK),
	FUSE_OPT_KEY("noauto", FUSE_OPT_KEY_DISCARD),
	FUSE_OPT_KEY("fsname=", FUSE_OPT_KEY_DISCARD),
	FUSE_OPT_KEY("blkdev", FUSE_OPT_KEY_DISCARD),
	FUSE_OPT_KEY("blksize=", FUSE_OPT_KEY_DISCARD),
	FUSE_OPT_KEY("ro", FUSE_OPT_KEY_KEEP),
	FUSE_OPT_KEY("rw", FUSE_OPT_KEY_KEEP),
	FUSE_OPT_KEY("atime", FUSE_OPT_KEY_KEEP),
	FUSE_OPT_KEY("noatime", FUSE_OPT_KEY_KEEP),
	FUSE_OPT_END
};

static const char *EXEC_NAME = "ntfsmount";
static char ntfs_fuse_default_options[] =
		"default_permissions,allow_other,use_ino,kernel_cache,nonempty";
static ntfs_fuse_context_t *ctx;

#ifdef SC_CACHE

#include "lcnalloc.h"
#include "sc_cache.h"

struct ntfs_cache_s {
	struct list_head head;        /* cached data list head. */
	s64  s_off;                   /* offset of the first cache, also the offset for all cached data. */
	s64  size;                    /* cache total size */
	struct sc_cache_s *last;      /* the last cache. for speed up operation. */
	ntfs_attr *na;                /* back pointer to cache owner */
	struct list_head cachelist;   /* global cached data list. */
};

#define CACHE_SIZE       4096
#define CACHE_NR         32

struct list_head  ntfs_free_cache;           /* cache pool. */
struct list_head  ntfs_cached_list;          /* all cached data list. */

static int ntfs_cache_create(void);
static void ntfs_cache_destory(void);
static inline void ntfs_cache_init1(void *ch);
static inline void ntfs_cache_init2(void *ch, ntfs_attr *na);
static int ntfs_cache_do(void *ch, const char *buf, size_t size, off_t offset);
static s64 ntfs_cache_flush(void *ch);
static void ntfs_cache_release(void *ch);
static s64 ntfs_cache_attr_write(void *ch);
static int ntfs_cache_fuse_write(const char *path, const char *buf, size_t size,
		off_t offset, struct fuse_file_info *fi);
static int ntfs_cache_flush_cached_list(int thresh);

/*
 * Before do new action to this file, we should flush (finish) 
 * the write cache. 
 */
static inline ntfs_attr* ntfs_cache_attr_open(ntfs_inode *ni, const ATTR_TYPES type,
		ntfschar *name, u32 name_len)
{
	ntfs_attr *na;
	na = ntfs_attr_open(ni, type, name, name_len);
	if ( na && na->write_cache ) {
		ntfs_cache_flush(na->write_cache);
	}
	return na;
}

#define ntfs_attr_open(__a, __b, __c, __d)  \
	ntfs_cache_attr_open(__a, __b, __c, __d)

#endif /* SC_CACHE */

/**
 * ntfs_fuse_is_named_data_stream - check path to be to named data stream
 * @path:	path to check
 *
 * Returns 1 if path is to named data stream or 0 otherwise.
 */
static __inline__ int ntfs_fuse_is_named_data_stream(const char *path)
{
	if (strchr(path, ':') && ctx->streams == NF_STREAMS_INTERFACE_WINDOWS)
		return 1;
	return 0;
}

static __inline__ void ntfs_fuse_update_times(ntfs_inode *ni,
		ntfs_time_update_flags mask)
{
	if (ctx->noatime)
		mask &= ~NTFS_UPDATE_ATIME;
	if (ctx->relatime && mask == NTFS_UPDATE_ATIME &&
			ni->last_access_time >= ni->last_data_change_time &&
			ni->last_access_time >= ni->last_mft_change_time)
		return;
	ntfs_inode_update_times(ni, mask);
}

/**
 * ntfs_fuse_statfs - return information about mounted NTFS volume
 * @path:	ignored (but fuse requires it)
 * @sfs:	statfs structure in which to return the information
 *
 * Return information about the mounted NTFS volume @sb in the statfs structure
 * pointed to by @sfs (this is initialized with zeros before ntfs_statfs is
 * called). We interpret the values to be correct of the moment in time at
 * which we are called. Most values are variable otherwise and this isn't just
 * the free values but the totals as well. For example we can increase the
 * total number of file nodes if we run out and we can keep doing this until
 * there is no more space on the volume left at all.
 *
 * This code based on ntfs_statfs from ntfs kernel driver.
 *
 * Returns 0 on success or -errno on error.
 */
static int ntfs_fuse_statfs(const char *path __attribute__((unused)),
		struct statvfs *sfs)
{
	/* Optimal transfer block size. */
	sfs->f_bsize = ctx->vol->cluster_size;
	sfs->f_frsize = ctx->vol->cluster_size;
	/*
	 * Total data blocks in file system in units of f_bsize and since
	 * inodes are also stored in data blocs ($MFT is a file) this is just
	 * the total clusters.
	 */
	sfs->f_blocks = ctx->vol->nr_clusters;
	/*
	 * Free data blocks and free data block available to non-superuser in
	 * file system in units of f_bsize.
	 */
	sfs->f_bavail = sfs->f_bfree = ctx->vol->nr_free_clusters;
	/* Number of inodes in file system (at this point in time). */
	sfs->f_files = ctx->vol->mft_na->data_size >>
			ctx->vol->mft_record_size_bits;
	/* Free inodes in fs (based on current total_count). */
	sfs->f_ffree = ctx->vol->nr_free_mft_records;
	/* Maximum length of filenames. */
	sfs->f_namemax = NTFS_MAX_NAME_LEN;
	return 0;
}

/**
 * ntfs_fuse_parse_path - split path to path and stream name.
 * @org_path:		path to split
 * @path:		pointer to buffer in which parsed path saved
 * @stream_name:	pointer to buffer where stream name in unicode saved
 *
 * This function allocates buffers for @*path and @*stream, user must free them
 * after use.
 *
 * Return values:
 *	<0	Error occurred, return -errno;
 *	 0	No stream name, @*stream is not allocated and set to AT_UNNAMED.
 *	>0	Stream name length in unicode characters.
 */
static int ntfs_fuse_parse_path(const char *org_path, char **path,
		ntfschar **stream_name)
{
	char *stream_name_mbs;
	int res;

	stream_name_mbs = strdup(org_path);
	if (!stream_name_mbs)
		return -errno;
	if (ctx->streams == NF_STREAMS_INTERFACE_WINDOWS) {
		*path = strsep(&stream_name_mbs, ":");
		if (stream_name_mbs) {
			*stream_name = NULL;
			res = ntfs_mbstoucs(stream_name_mbs, stream_name, 0);
			if (res < 0)
				return -errno;
			return res;
		}
	} else
		*path = stream_name_mbs;
	*stream_name = AT_UNNAMED;
	return 0;
}

/**
 * ntfs_fuse_cache_get_dir
 *
 * WARNING: Do not close inodes obtained with this function. They will be closed
 * automatically upon to next call to this function with different path or in
 * .destroy().
 */
static ntfs_inode *ntfs_fuse_cache_get_dir(const char *path)
{
	ntfs_inode *ni;

	if (!*path)
		path = "/";

	if (ctx->cached_ni && !strcmp(ctx->cached_path, path)) {
		ntfs_log_trace("Got '%s' from cache.\n", path);
		return ctx->cached_ni;
	}

	ni = ntfs_pathname_to_inode(ctx->vol, NULL, path);
	if (!ni)
		return NULL;
	ntfs_inode_close(ctx->cached_ni);
	ctx->cached_ni = ni;
	strncpy(ctx->cached_path, path, sizeof(ctx->cached_path));
	if (ctx->cached_path[sizeof(ctx->cached_path) - 1]) {
		/* Path was truncated, invalidate it since we can not use it. */
		ctx->cached_path[sizeof(ctx->cached_path) - 1] = 0;
		*ctx->cached_path = 0;
	}
	ntfs_log_trace("Cached '%s'.\n", path);
	return ni;
}

/**
 * ntfs_fuse_cache_get_file
 *
 * WARNING: This function changes @path during execution, but restores it
 * original value upon exit. Do *NOT* pass constant strings to this function!
 *
 * WARNING: You should close inode obtained with this function vice-versa to
 * inodes obtained with ntfs_fuse_cache_get_dir().
 */
static ntfs_inode *ntfs_fuse_cache_get_file(char *path)
{
	ntfs_inode *dir_ni;
	char *file;

	file = strrchr(path, '/');
	if (file == path)
		return ntfs_pathname_to_inode(ctx->vol, NULL, file);
	*file = 0;
	dir_ni = ntfs_fuse_cache_get_dir(path);
	*file = '/';
	file++;

	if (dir_ni)
		return ntfs_pathname_to_inode(ctx->vol, dir_ni, file);
	else
		return NULL;
}

static int ntfs_fuse_getattr(const char *org_path, struct stat *stbuf)
{
	int res = 0;
	ntfs_inode *ni;
	ntfs_attr *na;
	char *path = NULL;
	ntfschar *stream_name;
	int stream_name_len;

	stream_name_len = ntfs_fuse_parse_path(org_path, &path, &stream_name);
	if (stream_name_len < 0)
		return stream_name_len;
	memset(stbuf, 0, sizeof(struct stat));
	ni = ntfs_fuse_cache_get_file(path);
	if (!ni) {
		res = -ENOENT;
		goto exit;
	}
	stbuf->st_uid = ctx->uid;
	stbuf->st_gid = ctx->gid;
	stbuf->st_ino = ni->mft_no;
	stbuf->st_atime = ni->last_access_time;
	stbuf->st_ctime = ni->last_mft_change_time;
	stbuf->st_mtime = ni->last_data_change_time;
	if (ni->mrec->flags & MFT_RECORD_IS_DIRECTORY && !stream_name_len) {
		/* Directory. */
		stbuf->st_mode = S_IFDIR | (0777 & ~ctx->dmask);
		na = ntfs_attr_open(ni, AT_INDEX_ALLOCATION, NTFS_INDEX_I30, 4);
		if (na) {
			stbuf->st_size = na->data_size;
			stbuf->st_blocks = na->allocated_size >> 9;
			ntfs_attr_close(na);
		} else {
			stbuf->st_size = 0;
			stbuf->st_blocks = 0;
		}
		stbuf->st_nlink = 1; /* Needed for correct find work. */
	} else {
		/* Regular, data stream or Interix (INTX) file. */
		stbuf->st_mode = S_IFREG;
		stbuf->st_nlink = le16_to_cpu(ni->mrec->link_count);
		na = ntfs_attr_open(ni, AT_DATA, stream_name, stream_name_len);
		if (!na) {
			if (stream_name_len)
				res = -ENOENT;
			else
				stbuf->st_size = stbuf->st_blocks = 0;
			goto exit;
		}
		if (NAttrNonResident(na))
			stbuf->st_blocks = na->allocated_size >> 9;
		else
			stbuf->st_blocks = 0;
		stbuf->st_size = na->data_size;
		if (ni->flags & FILE_ATTR_SYSTEM && !stream_name_len) {
			/* Check whether it's Interix FIFO or socket. */
			if (!(ni->flags & FILE_ATTR_HIDDEN)) {
				/* FIFO. */
				if (na->data_size == 0)
					stbuf->st_mode = S_IFIFO;
				/* Socket link. */
				if (na->data_size == 1)
					stbuf->st_mode = S_IFSOCK;
			}
			/*
			 * Check whether it's Interix symbolic link, block or
			 * character device.
			 */
			if (na->data_size <= sizeof(INTX_FILE_TYPES) + sizeof(
					ntfschar) * MAX_PATH && na->data_size >
					sizeof(INTX_FILE_TYPES)) {
				INTX_FILE *intx_file;

				intx_file = ntfs_malloc(na->data_size);
				if (!intx_file) {
					res = -errno;
					ntfs_attr_close(na);
					goto exit;
				}
				if (ntfs_attr_pread(na, 0, na->data_size,
						intx_file) != na->data_size) {
					res = -errno;
					free(intx_file);
					ntfs_attr_close(na);
					goto exit;
				}
				if (intx_file->magic == INTX_BLOCK_DEVICE &&
						na->data_size == offsetof(
						INTX_FILE, device_end)) {
					stbuf->st_mode = S_IFBLK;
					stbuf->st_rdev = makedev(le64_to_cpu(
							intx_file->major),
							le64_to_cpu(
							intx_file->minor));
				}
				if (intx_file->magic == INTX_CHARACTER_DEVICE &&
						na->data_size == offsetof(
						INTX_FILE, device_end)) {
					stbuf->st_mode = S_IFCHR;
					stbuf->st_rdev = makedev(le64_to_cpu(
							intx_file->major),
							le64_to_cpu(
							intx_file->minor));
				}
				if (intx_file->magic == INTX_SYMBOLIC_LINK)
					stbuf->st_mode = S_IFLNK;
				free(intx_file);
			}
		}
		ntfs_attr_close(na);
		stbuf->st_mode |= (0777 & ~ctx->fmask);
	}
exit:
	if (ni)
		ntfs_inode_close(ni);
	free(path);
	if (stream_name_len)
		free(stream_name);
	return res;
}

static int ntfs_fuse_fgetattr(const char *path __attribute__((unused)),
		struct stat *stbuf, struct fuse_file_info *fi)
{
	NTFS_FUSE_GET_NA(fi);
	ntfs_inode *ni = na->ni;

#ifdef SC_CACHE
	if ( na->write_cache ) {
		ntfs_cache_flush(na->write_cache);
	}
#endif

	stbuf->st_ino = ni->mft_no;
	stbuf->st_mode = S_IFREG | (0777 & ~ctx->fmask);
	stbuf->st_nlink = le16_to_cpu(ni->mrec->link_count);
	stbuf->st_uid = ctx->uid;
	stbuf->st_gid = ctx->gid;
	stbuf->st_atime = ni->last_access_time;
	stbuf->st_ctime = ni->last_mft_change_time;
	stbuf->st_mtime = ni->last_data_change_time;
	if (NAttrNonResident(na))
		stbuf->st_blocks = na->allocated_size >> 9;
	else
		stbuf->st_blocks = 0;
	stbuf->st_size = na->data_size;
	return 0;
}

static int ntfs_fuse_readlink(const char *org_path, char *buf, size_t buf_size)
{
	char *path;
	ntfschar *stream_name;
	ntfs_inode *ni = NULL;
	ntfs_attr *na = NULL;
	INTX_FILE *intx_file = NULL;
	int stream_name_len, res = 0;

	/* Get inode. */
	stream_name_len = ntfs_fuse_parse_path(org_path, &path, &stream_name);
	if (stream_name_len < 0)
		return stream_name_len;
	if (stream_name_len > 0) {
		res = -EINVAL;
		goto exit;
	}
	ni = ntfs_fuse_cache_get_file(path);
	if (!ni) {
		res = -errno;
		goto exit;
	}
	/* Sanity checks. */
	if (!(ni->flags & FILE_ATTR_SYSTEM)) {
		res = -EINVAL;
		goto exit;
	}
	na = ntfs_attr_open(ni, AT_DATA, AT_UNNAMED, 0);
	if (!na) {
		res = -errno;
		goto exit;
	}
	if (na->data_size <= sizeof(INTX_FILE_TYPES)) {
		res = -EINVAL;
		goto exit;
	}
	if (na->data_size > sizeof(INTX_FILE_TYPES) +
			sizeof(ntfschar) * MAX_PATH) {
		res = -ENAMETOOLONG;
		goto exit;
	}
	/* Receive file content. */
	intx_file = ntfs_malloc(na->data_size);
	if (!intx_file) {
		res = -errno;
		goto exit;
	}
	if (ntfs_attr_pread(na, 0, na->data_size, intx_file) != na->data_size) {
		res = -errno;
		goto exit;
	}
	/* Sanity check. */
	if (intx_file->magic != INTX_SYMBOLIC_LINK) {
		res = -EINVAL;
		goto exit;
	}
	/* Convert link from unicode to local encoding. */
	if (ntfs_ucstombs(intx_file->target, (na->data_size -
			offsetof(INTX_FILE, target)) / sizeof(ntfschar),
			&buf, buf_size) < 0) {
		res = -errno;
		goto exit;
	}
exit:
	if (intx_file)
		free(intx_file);
	if (na)
		ntfs_attr_close(na);
	if (ni)
		ntfs_inode_close(ni);
	free(path);
	if (stream_name_len)
		free(stream_name);
	return res;
}

static int ntfs_fuse_filler(ntfs_fuse_fill_context_t *fill_ctx,
		const ntfschar *name, const int name_len, const int name_type,
		const s64 pos __attribute__((unused)), const MFT_REF mref,
		const unsigned dt_type __attribute__((unused)))
{
	char *filename = NULL;
	int ret = 0;

	if (name_type == FILE_NAME_DOS)
		return 0;
	if (ntfs_ucstombs(name, name_len, &filename, 0) < 0) {
		ntfs_log_error("Skipping unrepresentable filename (inode %lld):"
				" %s\n", MREF(mref), strerror(errno));
		return 0;
	}
	if (ntfs_fuse_is_named_data_stream(filename)) {
		ntfs_log_error("Unable to access '%s' (inode %lld) with "
				"current named streams access interface.\n",
				filename, MREF(mref));
		free(filename);
		return 0;
	}
	if (MREF(mref) == FILE_root || MREF(mref) >= FILE_first_user) {
		struct stat st = {
			.st_ino = MREF(mref),
		};

		if (dt_type == NTFS_DT_REG)
			st.st_mode = S_IFREG | (0777 & ~ctx->fmask);
		if (dt_type == NTFS_DT_DIR)
			st.st_mode = S_IFDIR | (0777 & ~ctx->dmask);
		ret = fill_ctx->filler(fill_ctx->buf, filename, &st, 0);
	}
	free(filename);
	return ret;
}

static int ntfs_fuse_readdir(const char *path, void *buf,
		fuse_fill_dir_t filler, off_t offset __attribute__((unused)),
		struct fuse_file_info *fi __attribute__((unused)))
{
	ntfs_fuse_fill_context_t fill_ctx;
	ntfs_inode *ni;
	s64 pos = 0;
	int err = 0;

	fill_ctx.filler = filler;
	fill_ctx.buf = buf;
	ni = ntfs_fuse_cache_get_dir(path);
	if (!ni)
		return -errno;
	if (ntfs_readdir(ni, &pos, &fill_ctx,
			(ntfs_filldir_t)ntfs_fuse_filler))
		err = -errno;
	ntfs_fuse_update_times(ni, NTFS_UPDATE_ATIME);
	return err;
}

static int ntfs_fuse_open(const char *org_path, struct fuse_file_info *fi)
{
	ntfs_inode *ni = NULL;
	ntfs_attr *na = NULL;
	int res = 0;
	char *path = NULL;
	ntfschar *stream_name;
	int stream_name_len;

	stream_name_len = ntfs_fuse_parse_path(org_path, &path, &stream_name);
	if (stream_name_len < 0)
		return stream_name_len;
	ni = ntfs_fuse_cache_get_file(path);
	if (ni) {
		na = ntfs_attr_open(ni, AT_DATA, stream_name, stream_name_len);
		if (na) {
			if (NAttrEncrypted(na) && !na->crypto)
				res = -EACCES;
		} else
			res = -errno;
	} else
		res = -errno;
	free(path);
	if (stream_name_len)
		free(stream_name);
	if (res) {
		ntfs_attr_close(na);
		ntfs_inode_close(ni);
	} else {
		fi->fh = (uintptr_t)na;
	}
	return res;
}

static int ntfs_fuse_flush(const char *path __attribute__((unused)),
		struct fuse_file_info *fi)
{
	NTFS_FUSE_GET_NA(fi);

#ifdef SC_CACHE
	/* flush cached data. I don't know if is the cache owner flushing
	 * the file, but flush cache do no harm. */
	if ( na->write_cache ) {
		ntfs_cache_flush(na->write_cache);
	}
#endif
	if (ntfs_inode_sync(na->ni))
		return -errno;
	return 0;
}

static int ntfs_fuse_release(const char *path __attribute__((unused)),
		struct fuse_file_info *fi)
{
	NTFS_FUSE_GET_NA(fi);
	ntfs_inode *ni = na->ni;

#ifdef SC_CACHE
	if ( na->write_cache ) {
		/* flush cached data. I don't know if is the cache owner 
		 * releasing(close) the file, but flush cache should do no harm. */
		ntfs_cache_flush(na->write_cache);
	}
#endif

	ntfs_attr_close(na);
	ntfs_inode_close(ni);
	return 0;
}

static int ntfs_fuse_read(const char *path, char *buf, size_t size,
		off_t offset, struct fuse_file_info *fi)
{
	NTFS_FUSE_GET_NA(fi);
	int res, total = 0;

	if (!size)
		return 0;

#ifdef SC_CACHE
	if ( na->write_cache ) {
		ntfs_cache_flush(na->write_cache);
	}
#endif

	if (offset + size > na->data_size)
		size = na->data_size - offset;
	while (size) {
		res = ntfs_attr_pread(na, offset, size, buf);
		if (res < (s64)size)
			ntfs_log_error("ntfs_attr_pread returned %d bytes "
					"instead of requested %lld bytes "
					"(offset %lld).\n", res,
					(long long)size, (long long)offset);
		if (res <= 0) {
			res = -errno;
			goto exit;
		}
		size -= res;
		offset += res;
		total += res;
	}
	res = total;
	ntfs_fuse_update_times(na->ni, NTFS_UPDATE_ATIME);
exit:
	return res;
}

static int ntfs_fuse_write(const char *path, const char *buf, size_t size,
		off_t offset, struct fuse_file_info *fi)
{
	NTFS_FUSE_GET_NA(fi);
	int res, total = 0;

	while (size) {
		res = ntfs_attr_pwrite(na, offset, size, buf);
		if (res < (s64)size && errno != ENOSPC)
			ntfs_log_error("ntfs_attr_pwrite returned %d bytes "
					"instead of requested %lld bytes "
					"(offset %lld).\n", res,
					(long long)size, (long long)offset);
		if (res <= 0) {
			res = -errno;
			goto exit;
		}
		size -= res;
		offset += res;
		total += res;
	}
	res = total;
exit:
	if (res > 0)
		ntfs_fuse_update_times(na->ni, NTFS_UPDATE_MTIME |
				NTFS_UPDATE_CTIME);
	return res;
}

static int ntfs_fuse_truncate(const char *org_path, off_t size)
{
	ntfs_inode *ni = NULL;
	ntfs_attr *na;
	int res;
	char *path = NULL;
	ntfschar *stream_name;
	int stream_name_len;

	stream_name_len = ntfs_fuse_parse_path(org_path, &path, &stream_name);
	if (stream_name_len < 0)
		return stream_name_len;
	ni = ntfs_fuse_cache_get_file(path);
	if (!ni) {
		res = -errno;
		goto exit;
	}
	if (ni->data_size == size) {
		res = 0;
		goto exit;
	}
	na = ntfs_attr_open(ni, AT_DATA, stream_name, stream_name_len);
	if (!na) {
		res = -errno;
		goto exit;
	}
	if (ntfs_attr_truncate(na, size))
		res = -errno;
	else {
		ntfs_fuse_update_times(ni, NTFS_UPDATE_MTIME |
				NTFS_UPDATE_CTIME);
		res = 0;
	}
	ntfs_attr_close(na);
exit:
	if (ni && ntfs_inode_close(ni))
		ntfs_log_perror("Failed to close inode");
	free(path);
	if (stream_name_len)
		free(stream_name);
	return res;
}

static int ntfs_fuse_ftruncate(const char *path __attribute__((unused)),
		off_t size, struct fuse_file_info *fi)
{
	NTFS_FUSE_GET_NA(fi);

#ifdef SC_CACHE
	if ( na->write_cache ) {
		ntfs_cache_flush(na->write_cache);
	}
#endif

	if (na->data_size == size)
		return 0;
	if (ntfs_attr_truncate(na, size))
		return -errno;
	ntfs_fuse_update_times(na->ni, NTFS_UPDATE_MTIME | NTFS_UPDATE_CTIME);
	return 0;
}

static int ntfs_fuse_chmod(const char *path,
		mode_t mode __attribute__((unused)))
{
	if (ntfs_fuse_is_named_data_stream(path))
		return -EINVAL; /* n/a for named data streams. */
	if (ctx->silent)
		return 0;
	return -EOPNOTSUPP;
}

static int ntfs_fuse_chown(const char *path, uid_t uid __attribute__((unused)),
		gid_t gid __attribute__((unused)))
{
	if (ntfs_fuse_is_named_data_stream(path))
		return -EINVAL; /* n/a for named data streams. */
	if (ctx->silent)
		return 0;
	return -EOPNOTSUPP;
}

static int __ntfs_fuse_mknod(const char *org_path, dev_t type, dev_t dev,
		const char *target, ntfs_inode **_ni)
{
	char *name;
	ntfschar *uname = NULL, *utarget = NULL;
	ntfs_inode *dir_ni = NULL, *ni;
	char *path;
	int res = 0, uname_len, utarget_len;

	if (_ni)
		*_ni = NULL;
	path = strdup(org_path);
	if (!path)
		return -errno;
	/* Generate unicode filename. */
	name = strrchr(path, '/');
	*name = 0;
	name++;
	uname_len = ntfs_mbstoucs(name, &uname, 0);
	if (uname_len < 0) {
		res = -errno;
		goto exit;
	}
	/* Open parent directory. */
	dir_ni = ntfs_fuse_cache_get_dir(path);
	if (!dir_ni) {
		res = -errno;
		if (res == -ENOENT)
			res = -EIO;
		goto exit;
	}
	/* Create object specified in @type. */
	switch (type) {
		case S_IFCHR:
		case S_IFBLK:
			ni = ntfs_create_device(dir_ni, uname, uname_len, type,
					dev);
			break;
		case S_IFLNK:
			utarget_len = ntfs_mbstoucs(target, &utarget, 0);
			if (utarget_len < 0) {
				res = -errno;
				goto exit;
			}
			ni = ntfs_create_symlink(dir_ni, uname, uname_len,
					utarget, utarget_len);
			break;
		default:
			ni = ntfs_create(dir_ni, uname, uname_len, type);
			break;
	}
	if (ni) {
		if (_ni)
			*_ni = ni;
		else
			ntfs_inode_close(ni);
		ntfs_fuse_update_times(dir_ni,
				NTFS_UPDATE_CTIME | NTFS_UPDATE_MTIME);
	} else
		res = -errno;
exit:
	free(uname);
	if (utarget)
		free(utarget);
	free(path);
	return res;
}

static int ntfs_fuse_create_stream(const char *path, ntfschar *stream_name,
		const int stream_name_len, ntfs_inode **_ni)
{
	ntfs_inode *ni;
	int res = 0;

	if (_ni)
		*_ni = NULL;
	ni = ntfs_pathname_to_inode(ctx->vol, NULL, path);
	if (!ni) {
		res = -errno;
		if (res == -ENOENT)
			res = __ntfs_fuse_mknod(path, S_IFREG, 0, NULL, &ni);
		if (!res && !ni)
			res = -EIO;
		if (res)
			return res;
	}
	if (ntfs_attr_add(ni, AT_DATA, stream_name, stream_name_len, NULL, 0))
		res = -errno;
	if (_ni)
		*_ni = ni;
	else {
		if (ntfs_inode_close(ni))
			ntfs_log_perror("Failed to close inode");
	}
	return res;
}

static int ntfs_fuse_mknod_common(const char *org_path, mode_t mode, dev_t dev,
		ntfs_attr **na)
{
	char *path = NULL;
	ntfschar *stream_name;
	int stream_name_len;
	int res = 0;
	ntfs_inode *ni;

	if (na)
		*na = NULL;
	stream_name_len = ntfs_fuse_parse_path(org_path, &path, &stream_name);
	if (stream_name_len < 0)
		return stream_name_len;
	if (stream_name_len && !S_ISREG(mode)) {
		res = -EINVAL;
		goto exit;
	}
	if (!stream_name_len)
		res = __ntfs_fuse_mknod(path, mode & S_IFMT, dev, NULL,
				na ? &ni : NULL);
	else
		res = ntfs_fuse_create_stream(path, stream_name,
				stream_name_len, na ? &ni : NULL);
	if (na && !res) {
		if (ni) {
			*na = ntfs_attr_open(ni, AT_DATA, stream_name,
					stream_name_len);
			if (!*na) {
				ntfs_inode_close(ni);
				res = -EIO;
			}
		} else
			res = -EIO;
	}
exit:
	free(path);
	if (stream_name_len)
		free(stream_name);
	return res;
}

static int ntfs_fuse_mknod(const char *path, mode_t mode, dev_t dev)
{
	return ntfs_fuse_mknod_common(path, mode, dev, NULL);
}

static int ntfs_fuse_create(const char *path, mode_t mode,
		struct fuse_file_info *fi)
{
	ntfs_attr *na;
	int res;

	res = ntfs_fuse_mknod_common(path, mode, 0, &na);
	fi->fh = (uintptr_t)na;
	return res;
}

static int ntfs_fuse_symlink(const char *to, const char *from)
{
	if (ntfs_fuse_is_named_data_stream(from))
		return -EINVAL; /* n/a for named data streams. */
	return __ntfs_fuse_mknod(from, S_IFLNK, 0, to, NULL);
}

static int ntfs_fuse_link(const char *old_path, const char *new_path)
{
	char *name;
	ntfschar *uname = NULL;
	ntfs_inode *dir_ni = NULL, *ni;
	char *path;
	int res = 0, uname_len;

	if (ntfs_fuse_is_named_data_stream(old_path))
		return -EINVAL; /* n/a for named data streams. */
	if (ntfs_fuse_is_named_data_stream(new_path))
		return -EINVAL; /* n/a for named data streams. */
	path = strdup(new_path);
	if (!path)
		return -errno;
	/* Open file for which create hard link. */
	ni = ntfs_pathname_to_inode(ctx->vol, NULL, old_path);
	if (!ni) {
		res = -errno;
		goto exit;
	}
	/* Generate unicode filename. */
	name = strrchr(path, '/');
	name++;
	uname_len = ntfs_mbstoucs(name, &uname, 0);
	if (uname_len < 0) {
		res = -errno;
		goto exit;
	}
	/* Open parent directory. */
	*name = 0;
	dir_ni = ntfs_pathname_to_inode(ctx->vol, NULL, path);
	if (!dir_ni) {
		res = -errno;
		if (res == -ENOENT)
			res = -EIO;
		goto exit;
	}
	/* Create hard link. */
	if (ntfs_link(ni, dir_ni, uname, uname_len))
		res = -errno;
	else {
		ntfs_fuse_update_times(ni, NTFS_UPDATE_CTIME);
		ntfs_fuse_update_times(dir_ni, NTFS_UPDATE_CTIME |
				NTFS_UPDATE_MTIME);
	}
exit:
	if (ni)
		ntfs_inode_close(ni);
	free(uname);
	if (dir_ni)
		ntfs_inode_close(dir_ni);
	free(path);
	return res;
}

static int ntfs_fuse_rm(const char *org_path)
{
	char *name;
	ntfschar *uname = NULL;
	ntfs_inode *dir_ni = NULL, *ni;
	char *path;
	int res = 0, uname_len;

	path = strdup(org_path);
	if (!path)
		return -errno;
	/* Open object for delete. */
	ni = ntfs_fuse_cache_get_file(path);
	if (!ni) {
		res = -errno;
		goto exit;
	}
	/* Generate unicode filename. */
	name = strrchr(path, '/');
	*name = 0;
	name++;
	uname_len = ntfs_mbstoucs(name, &uname, 0);
	if (uname_len < 0) {
		res = -errno;
		goto exit;
	}
	/* Open parent directory. */
	dir_ni = ntfs_fuse_cache_get_dir(path);
	if (!dir_ni) {
		res = -errno;
		if (res == -ENOENT)
			res = -EIO;
		goto exit;
	}
	/* Delete object. */
	if (ntfs_delete(&ni, dir_ni, uname, uname_len))
		res = -errno;
	else {
		if (ni)
			ntfs_fuse_update_times(ni, NTFS_UPDATE_CTIME);
		ntfs_fuse_update_times(dir_ni, NTFS_UPDATE_CTIME |
				NTFS_UPDATE_MTIME);
	}
exit:
	ntfs_inode_close(ni);
	free(uname);
	free(path);
	return res;
}

static int ntfs_fuse_rm_stream(const char *path, ntfschar *stream_name,
		const int stream_name_len)
{
	ntfs_inode *ni;
	ntfs_attr *na;
	int res = 0;

	ni = ntfs_pathname_to_inode(ctx->vol, NULL, path);
	if (!ni)
		return -errno;
	na = ntfs_attr_open(ni, AT_DATA, stream_name, stream_name_len);
	if (!na) {
		res = -errno;
		goto exit;
	}
	if (ntfs_attr_rm(na))
		res = -errno;
exit:
	if (ntfs_inode_close(ni))
		ntfs_log_perror("Failed to close inode");
	return res;
}

static int ntfs_fuse_unlink(const char *org_path)
{
	char *path = NULL;
	ntfschar *stream_name;
	int stream_name_len;
	int res = 0;

	stream_name_len = ntfs_fuse_parse_path(org_path, &path, &stream_name);
	if (stream_name_len < 0)
		return stream_name_len;
	if (!stream_name_len)
		res = ntfs_fuse_rm(path);
	else
		res = ntfs_fuse_rm_stream(path, stream_name, stream_name_len);
	free(path);
	if (stream_name_len)
		free(stream_name);
	return res;
}

static int ntfs_fuse_rename(const char *old_path, const char *new_path)
{
	int ret;
	u64 inum_new, inum_old;

	/* Check whether destination already exists. */
	if ((inum_new = ntfs_pathname_to_inode_num(ctx->vol, NULL, new_path)) !=
			(u64)-1) {
		if (errno != ENOENT)
			return -errno;
		/*
		 * If source and destination belongs to the same inode, then
		 * just unlink source if mount is case sensitive or return
		 * -EINVAL if mount is case insensitive, because of a lot of
		 * brain damaged cases here. Anyway coreutils is broken for
		 * case sensitive filesystems.
		 *
		 * If source and destination belongs to different inodes, then
		 * unlink current destination, so we can create link to source.
		 */
		inum_old = ntfs_pathname_to_inode_num(ctx->vol, NULL, old_path);
		if (inum_old == inum_new) {
			if (NVolCaseSensitive(ctx->vol))
				goto unlink;
			else
				return -EINVAL;
		} else
			if ((ret = ntfs_fuse_unlink(new_path)))
				return ret;
	}
	if ((ret = ntfs_fuse_link(old_path, new_path)))
		return ret;
unlink:
	if ((ret = ntfs_fuse_unlink(old_path))) {
		ntfs_fuse_unlink(new_path);
		return ret;
	}
	return 0;
}

static int ntfs_fuse_mkdir(const char *path,
		mode_t mode __attribute__((unused)))
{
	if (ntfs_fuse_is_named_data_stream(path))
		return -EINVAL; /* n/a for named data streams. */
	return __ntfs_fuse_mknod(path, S_IFDIR, 0, NULL, NULL);
}

static int ntfs_fuse_rmdir(const char *path)
{
	if (ntfs_fuse_is_named_data_stream(path))
		return -EINVAL; /* n/a for named data streams. */
	return ntfs_fuse_rm(path);
}

static int ntfs_fuse_utimens(const char *path, const struct timespec ts[2])
{
	ntfs_inode *ni;
	time_t now;

	if (ntfs_fuse_is_named_data_stream(path))
		return -EINVAL; /* n/a for named data streams. */
	ni = ntfs_pathname_to_inode(ctx->vol, NULL, path);
	if (!ni)
		return -errno;
	now = time(NULL);
	ni->last_mft_change_time = now;
	if (ts) {
		ni->last_access_time = ts[0].tv_sec;
		ni->last_data_change_time = ts[1].tv_sec;
	} else {
		ni->last_access_time = now;
		ni->last_data_change_time = now;
	}
	NInoFileNameSetDirty(ni);
	NInoSetDirty(ni);
	if (ntfs_inode_close(ni))
		ntfs_log_perror("Failed to close inode");
	return 0;
}

static int ntfs_fuse_bmap(const char *path, size_t blocksize, uint64_t *idx)
{
	ntfs_inode *ni;
	ntfs_attr *na;
	LCN lcn;
	int ret = 0, cl_per_bl = ctx->vol->cluster_size / blocksize;

	if (blocksize > ctx->vol->cluster_size ||
			ntfs_fuse_is_named_data_stream(path))
		return -EINVAL;

	ni = ntfs_pathname_to_inode(ctx->vol, NULL, path);
	if (!ni)
		return -errno;
	na = ntfs_attr_open(ni, AT_DATA, AT_UNNAMED, 0);
	if (!na) {
		ret = -errno;
		goto close_inode;
	}

	if (NAttrCompressed(na) || NAttrEncrypted(na) ||
			!NAttrNonResident(na)) {
		ret = -EINVAL;
		goto close_attr;
	}

	lcn = ntfs_attr_vcn_to_lcn(na, *idx / cl_per_bl);
	if (lcn < 0) {
		if (lcn == LCN_HOLE)
			ret = -EINVAL;
		else
			ret = -EIO;
		goto close_attr;
	}
	*idx = lcn * cl_per_bl + *idx % cl_per_bl;
close_attr:
	ntfs_attr_close(na);
close_inode:
	if (ntfs_inode_close(ni))
		ntfs_log_perror("bmap: failed to close inode");
	return ret;
}

#ifdef HAVE_SETXATTR

static const char nf_ns_xattr_preffix[] = "user.";
static const int nf_ns_xattr_preffix_len = 5;

static int ntfs_fuse_listxattr(const char *path, char *list, size_t size)
{
	ntfs_attr_search_ctx *actx = NULL;
	ntfs_inode *ni;
	char *to = list;
	int ret = 0;

	if (ctx->streams != NF_STREAMS_INTERFACE_XATTR)
		return -EOPNOTSUPP;
	ni = ntfs_pathname_to_inode(ctx->vol, NULL, path);
	if (!ni)
		return -errno;
	actx = ntfs_attr_get_search_ctx(ni, NULL);
	if (!actx) {
		ret = -errno;
		ntfs_inode_close(ni);
		goto exit;
	}
	while (!ntfs_attr_lookup(AT_DATA, NULL, 0, CASE_SENSITIVE,
				0, NULL, 0, actx)) {
		char *tmp_name = NULL;
		int tmp_name_len;

		if (!actx->attr->name_length)
			continue;
		tmp_name_len = ntfs_ucstombs((ntfschar *)((u8*)actx->attr +
				le16_to_cpu(actx->attr->name_offset)),
				actx->attr->name_length, &tmp_name, 0);
		if (tmp_name_len < 0) {
			ret = -errno;
			goto exit;
		}
		ret += tmp_name_len + nf_ns_xattr_preffix_len + 1;
		if (size) {
			if ((size_t)ret <= size) {
				strcpy(to, nf_ns_xattr_preffix);
				to += nf_ns_xattr_preffix_len;
				strncpy(to, tmp_name, tmp_name_len);
				to += tmp_name_len;
				*to = 0;
				to++;
			} else {
				free(tmp_name);
				ret = -ERANGE;
				goto exit;
			}
		}
		free(tmp_name);
	}
	if (errno != ENOENT)
		ret = -errno;
exit:
	if (actx)
		ntfs_attr_put_search_ctx(actx);
	ntfs_inode_close(ni);
	ntfs_log_debug("return %d\n", ret);
	return ret;
}

static int ntfs_fuse_getxattr_windows(const char *path, const char *name,
				char *value, size_t size)
{
	ntfs_attr_search_ctx *actx = NULL;
	ntfs_inode *ni;
	char *to = value;
	int ret = 0;

	if (strcmp(name, "ntfs.streams.list"))
		return -EOPNOTSUPP;
	ni = ntfs_pathname_to_inode(ctx->vol, NULL, path);
	if (!ni)
		return -errno;
	actx = ntfs_attr_get_search_ctx(ni, NULL);
	if (!actx) {
		ret = -errno;
		ntfs_inode_close(ni);
		goto exit;
	}
	while (!ntfs_attr_lookup(AT_DATA, NULL, 0, CASE_SENSITIVE,
				0, NULL, 0, actx)) {
		char *tmp_name = NULL;
		int tmp_name_len;

		if (!actx->attr->name_length)
			continue;
		tmp_name_len = ntfs_ucstombs((ntfschar *)((u8*)actx->attr +
				le16_to_cpu(actx->attr->name_offset)),
				actx->attr->name_length, &tmp_name, 0);
		if (tmp_name_len < 0) {
			ret = -errno;
			goto exit;
		}
		if (ret)
			ret++; /* For space delimiter. */
		ret += tmp_name_len;
		if (size) {
			if ((size_t)ret <= size) {
				/* Don't add space to the beginning of line. */
				if (to != value) {
					*to = ' ';
					to++;
				}
				strncpy(to, tmp_name, tmp_name_len);
				to += tmp_name_len;
			} else {
				free(tmp_name);
				ret = -ERANGE;
				goto exit;
			}
		}
		free(tmp_name);
	}
	if (errno != ENOENT)
		ret = -errno;
exit:
	if (actx)
		ntfs_attr_put_search_ctx(actx);
	ntfs_inode_close(ni);
	return ret;
}

static int ntfs_fuse_getxattr(const char *path, const char *name,
				char *value, size_t size)
{
	ntfs_inode *ni;
	ntfs_attr *na = NULL;
	ntfschar *lename = NULL;
	int res, lename_len;

	if (ctx->streams == NF_STREAMS_INTERFACE_WINDOWS)
		return ntfs_fuse_getxattr_windows(path, name, value, size);
	if (ctx->streams != NF_STREAMS_INTERFACE_XATTR)
		return -EOPNOTSUPP;
	if (strncmp(name, nf_ns_xattr_preffix, nf_ns_xattr_preffix_len) ||
			strlen(name) == (size_t)nf_ns_xattr_preffix_len)
		return -ENODATA;
	ni = ntfs_pathname_to_inode(ctx->vol, NULL, path);
	if (!ni)
		return -errno;
	lename_len = ntfs_mbstoucs(name + nf_ns_xattr_preffix_len, &lename, 0);
	if (lename_len == -1) {
		res = -errno;
		goto exit;
	}
	na = ntfs_attr_open(ni, AT_DATA, lename, lename_len);
	if (!na) {
		res = -ENODATA;
		goto exit;
	}
	if (size) {
		if (size >= na->data_size) {
			res = ntfs_attr_pread(na, 0, na->data_size, value);
			if (res != na->data_size)
				res = -errno;
		} else
			res = -ERANGE;
	} else
		res = na->data_size;
exit:
	if (na)
		ntfs_attr_close(na);
	free(lename);
	if (ntfs_inode_close(ni))
		ntfs_log_perror("Failed to close inode");
	return res;
}

static int ntfs_fuse_setxattr(const char *path, const char *name,
				const char *value, size_t size, int flags)
{
	ntfs_inode *ni;
	ntfs_attr *na = NULL;
	ntfschar *lename = NULL;
	int res, lename_len;

	if (ctx->streams != NF_STREAMS_INTERFACE_XATTR)
		return -EOPNOTSUPP;
	if (strncmp(name, nf_ns_xattr_preffix, nf_ns_xattr_preffix_len) ||
			strlen(name) == (size_t)nf_ns_xattr_preffix_len)
		return -EACCES;
	ni = ntfs_pathname_to_inode(ctx->vol, NULL, path);
	if (!ni)
		return -errno;
	lename_len = ntfs_mbstoucs(name + nf_ns_xattr_preffix_len, &lename, 0);
	if (lename_len == -1) {
		res = -errno;
		goto exit;
	}
	na = ntfs_attr_open(ni, AT_DATA, lename, lename_len);
	if (na && flags == XATTR_CREATE) {
		res = -EEXIST;
		goto exit;
	}
	if (!na) {
		if (flags == XATTR_REPLACE) {
			res = -ENODATA;
			goto exit;
		}
		if (ntfs_attr_add(ni, AT_DATA, lename, lename_len, NULL, 0)) {
			res = -errno;
			goto exit;
		}
		na = ntfs_attr_open(ni, AT_DATA, lename, lename_len);
		if (!na) {
			res = -errno;
			goto exit;
		}
	}
	res = ntfs_attr_pwrite(na, 0, size, value);
	if (res != (s64) size)
		res = -errno;
	else
		res = 0;
exit:
	if (na)
		ntfs_attr_close(na);
	free(lename);
	if (ntfs_inode_close(ni))
		ntfs_log_perror("Failed to close inode");
	return res;
}

static int ntfs_fuse_removexattr(const char *path, const char *name)
{
	ntfs_inode *ni;
	ntfs_attr *na = NULL;
	ntfschar *lename = NULL;
	int res = 0, lename_len;

	if (ctx->streams != NF_STREAMS_INTERFACE_XATTR)
		return -EOPNOTSUPP;
	if (strncmp(name, nf_ns_xattr_preffix, nf_ns_xattr_preffix_len) ||
			strlen(name) == (size_t)nf_ns_xattr_preffix_len)
		return -ENODATA;
	ni = ntfs_pathname_to_inode(ctx->vol, NULL, path);
	if (!ni)
		return -errno;
	lename_len = ntfs_mbstoucs(name + nf_ns_xattr_preffix_len, &lename, 0);
	if (lename_len == -1) {
		res = -errno;
		goto exit;
	}
	na = ntfs_attr_open(ni, AT_DATA, lename, lename_len);
	if (!na) {
		res = -ENODATA;
		goto exit;
	}
	if (ntfs_attr_rm(na))
		res = -errno;
	na = NULL;
exit:
	if (na)
		ntfs_attr_close(na);
	free(lename);
	if (ntfs_inode_close(ni))
		ntfs_log_perror("Failed to close inode");
	return res;
}

#endif /* HAVE_SETXATTR */

static void ntfs_fuse_destroy(void *priv __attribute__((unused)))
{
	if (ctx->vol) {
		ntfs_inode_close(ctx->cached_ni);
		ntfs_inode_close(ctx->root_ni);
		ntfs_log_info("Unmounting %s (%s)\n", ctx->device,
				ctx->vol->vol_name);
		if (ntfs_umount(ctx->vol, FALSE))
			ntfs_log_perror("Failed to unmount volume");
	}
}

static void ntfs_fuse_free_context(void)
{
	free(ctx->device);
	free(ctx->mnt_point);
	free(ctx->locale);
	free(ctx);
}

static void ntfs_fuse_full_destroy(void)
{
	ntfs_fuse_destroy(NULL);
	ntfs_fuse_free_context();
}

static struct fuse_operations ntfs_fuse_oper = {
	.getattr	= ntfs_fuse_getattr,
	.fgetattr	= ntfs_fuse_fgetattr,
	.readlink	= ntfs_fuse_readlink,
	.readdir	= ntfs_fuse_readdir,
	.open		= ntfs_fuse_open,
	.flush		= ntfs_fuse_flush,
	.release	= ntfs_fuse_release,
	.read		= ntfs_fuse_read,
#ifdef SC_CACHE
	.write		= ntfs_cache_fuse_write,
#else
	.write		= ntfs_fuse_write,
#endif
	.truncate	= ntfs_fuse_truncate,
	.ftruncate	= ntfs_fuse_ftruncate,
	.statfs		= ntfs_fuse_statfs,
	.chmod		= ntfs_fuse_chmod,
	.chown		= ntfs_fuse_chown,
	.mknod		= ntfs_fuse_mknod,
	.create		= ntfs_fuse_create,
	.symlink	= ntfs_fuse_symlink,
	.link		= ntfs_fuse_link,
	.unlink		= ntfs_fuse_unlink,
	.rename		= ntfs_fuse_rename,
	.mkdir		= ntfs_fuse_mkdir,
	.rmdir		= ntfs_fuse_rmdir,
	.utimens	= ntfs_fuse_utimens,
	.bmap		= ntfs_fuse_bmap,
	.destroy	= ntfs_fuse_destroy,
#ifdef HAVE_SETXATTR
	.getxattr	= ntfs_fuse_getxattr,
	.setxattr	= ntfs_fuse_setxattr,
	.removexattr	= ntfs_fuse_removexattr,
	.listxattr	= ntfs_fuse_listxattr,
#endif /* HAVE_SETXATTR */
};

static void signal_handler(int arg __attribute__((unused)))
{
	fuse_exit((fuse_get_context())->fuse);
}

static void usage(void)
{
	ntfs_log_info("\n%s v%s (libntfs %s) - Userspace read/write NTFS "
			"driver.\n\n", EXEC_NAME, VERSION,
			ntfs_libntfs_version());
	ntfs_log_info("Copyright (c) 2005-2007 Yura Pakhuchiy\n");
	ntfs_log_info("Copyright (c)      2005 Yuval Fledel\n");
	ntfs_log_info("Copyright (c)      2006 Szabolcs Szakacsits\n\n");
	ntfs_log_info("usage:  %s device mount_point [-o options]\n\n",
			EXEC_NAME);
	ntfs_log_info("Default options:\n\t%s\n\n",
			ntfs_fuse_default_options);
	ntfs_log_info("%s%s\n", ntfs_bugs, ntfs_home);
}

#ifndef HAVE_REALPATH
/* If there is no realpath() on the system, provide a dummy one. */
static char *realpath(const char *path, char *resolved_path)
{
	strncpy(resolved_path, path, PATH_MAX);
	resolved_path[PATH_MAX] = 0;
	return resolved_path;
}
#endif

static int ntfs_fuse_init(void)
{
	utils_set_locale();
	ntfs_log_set_handler(ntfs_log_handler_stderr);
	signal(SIGINT, signal_handler);
	signal(SIGTERM, signal_handler);

	ctx = ntfs_malloc(sizeof(ntfs_fuse_context_t));
	if (!ctx)
		return -1;

	*ctx = (ntfs_fuse_context_t) {
		.uid = getuid(),
		.gid = getgid(),
		.fmask = 0111,
		.dmask = 0,
		.streams = NF_STREAMS_INTERFACE_NONE,
		.silent = TRUE,
		.blkdev = TRUE,
	};
	return 0;
}

static int ntfs_fuse_opt_proc(void *data __attribute__((unused)),
		const char *arg, int key, struct fuse_args *outargs)
{
	switch (key) {
	case NF_KEY_HELP:
		usage();
		return -1; /* Force usage show. */
	case NF_KEY_UMASK:
		ctx->dmask = ctx->fmask;
		return 0;
	case FUSE_OPT_KEY_NONOPT: /* All non-option arguments go here. */
		if (!ctx->device) {
			/* We don't want relative path in /etc/mtab. */
			if (arg[0] != '/') {
				ctx->device = ntfs_malloc(PATH_MAX + 1);
				if (!ctx->device)
					return -1;
				if (!realpath(arg, ctx->device)) {
					ntfs_log_perror("realpath(): %s", arg);
					free(ctx->device);
					ctx->device = NULL;
					return -1;
				}
			} else {
				ctx->device = strdup(arg);
				if (!ctx->device) {
					ntfs_log_perror("strdup()");
					return -1;
				}
			}
			return 0;
		}
		if (!ctx->mnt_point) {
			ctx->mnt_point = strdup(arg);
			if (!ctx->mnt_point) {
				ntfs_log_perror("strdup()");
				return -1;
			}
			return 0;
		}
		ntfs_log_error("You must specify exactly one device and "
				"exactly one mount point.\n");
		return -1;
	default:
		if (!strcmp(arg, "remount")) {
			ntfs_log_error("Remounting is not supported yet. "
					"You have to umount volume and then "
					"mount it once again.\n");
			return -1;
		}
		return 1; /* Just pass all unknown to us options to FUSE. */
	}
}

static int ntfs_fuse_is_block_dev(void)
{
	struct stat st;

	if (stat(ctx->device, &st)) {
		ntfs_log_perror("Failed to stat %s", ctx->device);
		return -1;
	}

	if (S_ISBLK(st.st_mode)) {
		if (!ctx->blkdev) {
			ntfs_log_warning("WARNING: %s is block device, but you "
					"submitted 'noblkdev' option. This is "
					"not recommended.\n", ctx->device);
		}
		if (geteuid()) {
			ntfs_log_warning("WARNING: %s is block device, but you "
					"are not root and %s is not set-uid-"
					"root, so using 'blkdev' option is not "
					"possible. This is not recommended.\n",
					ctx->device, EXEC_NAME);
			ctx->blkdev = FALSE;
		}

	} else
		ctx->blkdev = FALSE;
	return 0;
}

static int parse_options(struct fuse_args *args)
{
	char *buffer = NULL;

	if (fuse_opt_parse(args, ctx, ntfs_fuse_opts, ntfs_fuse_opt_proc))
		return -1;

	if (!ctx->device) {
		ntfs_log_error("No device specified.\n");
		usage();
		return -1;
	}
	if (ctx->quiet && ctx->verbose) {
		ntfs_log_error("You may not use --quiet and --verbose at the "
				"same time.\n");
		return -1;
	}
	if (ctx->debug) {
		ntfs_log_set_levels(NTFS_LOG_LEVEL_DEBUG);
		ntfs_log_set_levels(NTFS_LOG_LEVEL_TRACE);
	}
	if (ctx->locale && !setlocale(LC_ALL, ctx->locale))
		ntfs_log_error("Failed to set locale to %s "
				"(continue anyway).\n", ctx->locale);
	if (ntfs_fuse_is_block_dev())
		return -1;

	buffer = ntfs_malloc(strlen(ctx->device) + 128);
	if (!buffer)
		return -1;
	sprintf(buffer, "-ofsname=%s", ctx->device);
	if (fuse_opt_add_arg(args, buffer) == -1) {
		free(buffer);
		return -1;
	}
	free(buffer);

	if (!ctx->no_def_opts) {
		if (fuse_opt_add_arg(args, "-o") == -1)
			return -1;
		if (fuse_opt_add_arg(args, ntfs_fuse_default_options) == -1)
			return -1;
	}
	if (ctx->debug || ctx->no_detach) {
		if (fuse_opt_add_arg(args, "-odebug") == -1)
			return -1;
	}
	return 0;
}

static int ntfs_fuse_set_blkdev_options(struct fuse_args *args)
{
	int pagesize, blksize = ctx->vol->cluster_size;
	struct passwd *pw;
	char buffer[128];

	pagesize = sysconf(_SC_PAGESIZE);
	if (pagesize < 1)
		pagesize = 4096;
	if (blksize > pagesize)
		blksize = pagesize;
	pw = getpwuid(ctx->uid);
	if (!pw || !pw->pw_name) {
		ntfs_log_perror("getpwuid(%d) failed", ctx->uid);
		return -1;
	}
	sprintf(buffer, "-oblkdev,blksize=%d,user=%s", blksize, pw->pw_name);
	if (fuse_opt_add_arg(args, buffer) == -1)
		return -1;
	return 0;
}

static int ntfs_fuse_mount(void)
{
	ntfs_volume *vol;

	vol = utils_mount_volume(ctx->device,
			(ctx->case_insensitive ? 0 : NTFS_MNT_CASE_SENSITIVE) |
			(ctx->blkdev ? NTFS_MNT_NOT_EXCLUSIVE : 0) |
			(ctx->force ? NTFS_MNT_FORCE : 0) |
			(ctx->ro ? NTFS_MNT_RDONLY : 0) |
			NTFS_MNT_INTERIX
#ifdef SC_FASTMNT
			|NTFS_MNT_FASTMNT  /* DON'T empty $logfile */
#endif
			);
	if (!vol) {
		ntfs_log_error("Mount failed.\n");
		return -1;
	}
	ctx->vol = vol;
	ctx->root_ni = ntfs_inode_open(vol, FILE_root);
	return 0;
}

static void ntfs_fd_init(void)
{
	int fd;
	/*
	 * Make sure file descriptors 0, 1 and 2 are open, otherwise chaos
	 * would ensue.
	 */
	do {
		fd = open("/dev/null", O_RDWR);
		if (fd > 2)
			close(fd);
	} while (fd >= 0 && fd <= 2);
}

int main(int argc, char *argv[])
{
	struct fuse_args args = FUSE_ARGS_INIT(argc, argv);
	struct fuse *fh;
	struct fuse_chan *fch;
	
	ntfs_fd_init();
	ntfs_fuse_init();
	if (parse_options(&args))
		goto err_out;
	/*
	 * Drop effective uid to real uid because we do not want not previleged
	 * user that runs set-uid-root binary to be able to mount devices
	 * normally he do not have rights to.
	 */
	if (seteuid(ctx->uid)) {
		ntfs_log_perror("seteuid(%d)", ctx->uid);
		goto err_out;
	}
	/* Mount volume (libntfs part). */
	if (ntfs_fuse_mount())
		goto err_out;
	if (ctx->blkdev) {
		/* Gain root privileges for blkdev mount. */
		if (seteuid(0) || setuid(0)) {
			ntfs_log_perror("seteuid(0) or setuid(0) failed");
			goto err_out;
		}
		/* Set blkdev, blksize and user options. */
		if (ntfs_fuse_set_blkdev_options(&args))
			goto err_out;
	}
	/* Create filesystem (FUSE part). */
	fch = fuse_mount(ctx->mnt_point, &args);
	if (!fch) {
		ntfs_log_error("fuse_mount failed.\n");
		goto err_out;
	}
	fh = fuse_new(fch, &args , &ntfs_fuse_oper, sizeof(ntfs_fuse_oper),
			NULL);

	fuse_opt_free_args(&args);
	if (!fh) {
		ntfs_log_error("fuse_new failed.\n");
		fuse_unmount(ctx->mnt_point, fch);
		ntfs_fuse_full_destroy();
		return 1;
	}
	/* Drop root privileges if we obtained them. */
	if (ctx->blkdev && setuid(ctx->uid))
		ntfs_log_warning("Failed to drop root privileges.\n");
	/* Detach from terminal. */
	if (!ctx->debug && !ctx->no_detach) {
		if (daemon(0, 0))
			ntfs_log_error("Failed to detach from terminal.\n");
		else {
			ntfs_log_set_handler(ntfs_log_handler_syslog);
			/* Override default libntfs identify. */
			openlog(EXEC_NAME, LOG_PID, LOG_DAEMON);
		}
	}

#ifdef SC_CACHE
	if ( ntfs_cache_create() < 0 ) {
		ntfs_log_error("ntfs_cache_create failed.\n");
		fuse_unmount(ctx->mnt_point, fch);
		ntfs_fuse_full_destroy();
		return 1;
	}
#endif

	ntfs_log_info("Version %s (libntfs %s)\n", VERSION,
			ntfs_libntfs_version());
	ntfs_log_info("Mounted %s (%s, label \"%s\", NTFS version %d.%d)\n",
			ctx->device, (ctx->ro) ? "Read-Only" : "Read-Write",
			ctx->vol->vol_name, ctx->vol->major_ver,
			ctx->vol->minor_ver);

	/* Main loop. */
	fuse_loop(fh);
	/* Destroy. */
	fuse_unmount(ctx->mnt_point, fch);
	fuse_destroy(fh);
	ntfs_fuse_free_context();
#ifdef SC_CACHE
	ntfs_cache_destory();
#endif
	return 0;
err_out:
	fuse_opt_free_args(&args);
	ntfs_fuse_full_destroy();
	return 1;
}

#ifdef SC_CACHE

static int ntfs_cache_create(void)
{
	INIT_LIST_HEAD(&ntfs_free_cache);
	INIT_LIST_HEAD(&ntfs_cached_list);
	if ( sc_cache_create(&ntfs_free_cache, CACHE_NR, CACHE_SIZE) < 0 ) {
		return -1;
	}
	return 0;
}

static void ntfs_cache_destory(void)
{
	ntfs_cache_flush_cached_list(0);
	sc_cache_free(&ntfs_free_cache);
}

/* init part1(basic) */
static inline void ntfs_cache_init1(void *ch)
{
	struct ntfs_cache_s *cachep = ch;
	INIT_LIST_HEAD(&cachep->head);
	cachep->size = 0;
	cachep->s_off = 0;
	cachep->last = NULL;
}

/* init part2(special) */
static inline void ntfs_cache_init2(void *ch, ntfs_attr *na)
{
	struct ntfs_cache_s *cachep = ch;
	cachep->na = na;
	INIT_LIST_HEAD(&cachep->cachelist);
}

/* Cache(cachelist) will be added to `ntfs_cached_list' when its size > 0,
 * and will be deleted from `ntfs_cached_list' when its size drop to 0 
 * (flushed) again. 
 */
static void ntfs_cache_release(void *ch)
{
	struct ntfs_cache_s *cachep = ch;
	sc_cache_put_list(&ntfs_free_cache, &cachep->head);
	ntfs_cache_init1(cachep);
	list_del_init(&cachep->cachelist);
}

static s64 ntfs_cache_flush(void *ch)
{
	struct ntfs_cache_s *cachep = ch;
	s64 res = 0;
	int eo = 0;

	if ( cachep && cachep->size > 0 ) {
		res = ntfs_cache_attr_write(cachep);
		if ( res > 0 ) {
			/* FIXME: update times when do cache. */
			ntfs_fuse_update_times(cachep->na->ni, NTFS_UPDATE_MTIME |
				NTFS_UPDATE_CTIME);
		} else {
			eo = errno;
			ntfs_log_warning("!!!! ntfs_fuse_write_cache failed, errno <%d> \n", errno);
		}
		ntfs_cache_release(cachep);
		errno = eo;
	}
	return res;
}

/* 
 * @thresh == 0 means flush all. 
 * flush cached data in list, and remove it. 
 */
static int ntfs_cache_flush_cached_list(int thresh)
{
	struct list_head *item, *n;
	struct ntfs_cache_s *cachep;
	s64  res;
	int  i = 0;
	
	list_for_each_safe(item, n, &ntfs_cached_list) {
		cachep = list_entry(item, struct ntfs_cache_s, cachelist);
		if ( cachep->size == 0 ) {
			ntfs_log_warning("!!! WARNNING, found cache of zero size in cachelist.\n");
			list_del_init(&cachep->cachelist);
			continue;
		}
		/* flush will do remove cache from this list(ntfs_cached_list) */
		res = ntfs_cache_flush(cachep);
		if ( res < 0 ) {
			return -errno;
		}
		if ( ++i == thresh ) {
			break;
		}
	}
	return i;
}

static int ntfs_cache_do(void *ch, const char *buf, size_t size, off_t offset)
{
	struct ntfs_cache_s *cachep = ch;
	struct sc_cache_s *newcachep = NULL, *first = NULL;
	struct sc_cache_s *last;
	size_t lastw = 0, w = 0;

	ntfs_log_trace("do cachep <%p>, size <%u> offset <0x%llx> \n", 
			cachep, size, offset);

	if ( !size ) {
		return 0;
	}
	/* check if incoming data can fit into cache. */
	if ( cachep->size != 0 && (cachep->s_off + cachep->size) != offset ) {
		return -1;
	}
	last = cachep->last;
	if ( last && last->size < CACHE_SIZE ) {
		lastw = min(CACHE_SIZE - last->size, size);
		memcpy(&(last->data[last->size]), buf, lastw);
		last->size += lastw;
		w += lastw;
		ntfs_log_trace("lastw <%u>, w <%u>, last->size <%lld> \n", lastw, w, last->size);
	}
	while ( w < size ) {
		size_t to_write;
		newcachep = sc_cache_get(&ntfs_free_cache);
		if ( !newcachep ) {
			goto _undo;
		}
		list_add_tail(&newcachep->list, &cachep->head);
		if ( first == NULL ) {
			first = newcachep;
		}
		to_write = min(size - w, CACHE_SIZE);
		memcpy(newcachep->data, &buf[w], to_write);
		newcachep->size = to_write;
		w += to_write;
		ntfs_log_trace("w <%u>, to_write <%u> \n", w, to_write);
	}
	if ( newcachep ) {
		cachep->last = newcachep;
	}
	/* OK. update these fields and out. */
	if ( cachep->size == 0 ) {
		cachep->s_off = offset;
		list_add_tail(&cachep->cachelist, &ntfs_cached_list);
	}
	cachep->size += size;
	return 0;
_undo:
	/* undo half done. */
	if ( lastw > 0 ) {
		last->size -= lastw;
	}
	if ( first != NULL ) {
		struct list_head *this, *next;
		struct sc_cache_s *e;
		for ( this = &first->list, next = this->next; 
			this != &cachep->head; this = next, next = next->next ) {
			e = list_entry(this, struct sc_cache_s, list);
			sc_cache_put(&ntfs_free_cache, e);
		}
	}
	return -2;
}

/*
 * This function is similiar to `ntfs_attr_pwrite', but it forces on dealing  
 * with scatter cached buffer. We do this so that more than one users can
 * share the cache pool. 
 * Hope it will work well.
 */
static s64 ntfs_cache_attr_write(void *ch)
{
	struct ntfs_cache_s *cachep = ch;
	s64 written, to_write, ofs, total, old_initialized_size, old_data_size;
	VCN update_from = -1;
	ntfs_volume *vol;
	ntfs_attr *na;
	ntfs_attr_search_ctx *ctx = NULL;
	runlist_element *rl;
	int eo;
	s64 pos, total_count, this_count;
	struct list_head *head, *cur_item;
	struct sc_cache_s* cur_cache;
	const u8 *b;

	struct {
		unsigned int undo_initialized_size	: 1;
		unsigned int undo_data_size		: 1;
		unsigned int update_mapping_pairs	: 1;
	} need_to = { 0, 0, 0 };

	if ( !cachep || !(na = cachep->na) || !na->ni || !na->ni->vol
		|| (pos = cachep->s_off) < 0 
		|| (total_count = cachep->size) < 0 ) {
		errno = EINVAL;
		return -1;
	}
	
	ntfs_log_trace("Entering for inode 0x%llx, attr 0x%x, pos 0x%llx, "
			"total_count 0x%llx.\n", na->ni->mft_no, na->type,
			(long long)pos, (long long)total_count);

	head = &cachep->head;
	vol = na->ni->vol;

	/*
	 * Encrypted non-resident attributes are not supported.  We return
	 * access denied, which is what Windows NT4 does, too.
	 */
	if (NAttrEncrypted(na) && NAttrNonResident(na)) {
		errno = EACCES;
		return -1;
	}

	/* If this is a compressed attribute it needs special treatment. */
	if (NAttrCompressed(na)) {
		errno = EOPNOTSUPP;
		return -1;
	}

	if ( !total_count || list_empty(head) ) {
		return 0;
	}

	/* If the write reaches beyond the end, extend the attribute. */
	old_data_size = na->data_size;
	if (pos + total_count > na->data_size) {
		if (__ntfs_attr_truncate(na, pos + total_count, FALSE)) {
			eo = errno;
			ntfs_log_trace("Attribute extend failed.\n");
			errno = eo;
			return -1;
		}
		need_to.undo_data_size = 1;
		ntfs_log_trace("NEW data_size 0x%llx \n", na->data_size);
	}
	old_initialized_size = na->initialized_size;
	/* If it is a resident attribute, write the data to the mft record. */
	if (!NAttrNonResident(na)) {
		char *val;

		ctx = ntfs_attr_get_search_ctx(na->ni, NULL);
		if (!ctx)
			goto err_out;
		if (ntfs_attr_lookup(na->type, na->name, na->name_len, 0,
				0, NULL, 0, ctx))
			goto err_out;
		val = (char*)ctx->attr + le16_to_cpu(ctx->attr->value_offset);
		if (val < (char*)ctx->attr || val +
				le32_to_cpu(ctx->attr->value_length) >
				(char*)ctx->mrec + vol->mft_record_size) {
			errno = EIO;
			goto err_out;
		}

		/* handle cache linearize */
		list_for_each(cur_item, head) {
			cur_cache = list_entry(cur_item, struct sc_cache_s, list);
			this_count = min(cur_cache->size, total_count);
			memcpy(val + pos, cur_cache->data, this_count);
			total_count -= this_count;
			pos += this_count; /* `pos' Adjusted ! */
			if (total_count == 0) {
				break;
			}
		}

		if (ntfs_mft_record_write(vol, ctx->ntfs_ino->mft_no,
				ctx->mrec)) {
			/*
			 * NOTE: We are in a bad state at this moment. We have
			 * dirtied the mft record but we failed to commit it to
			 * disk. Since we have read the mft record ok before,
			 * it is unlikely to fail writing it, so is ok to just
			 * return error here... (AIA)
			 */
			goto err_out;
		}
		ntfs_attr_put_search_ctx(ctx);
		/* return total count just written. */
		return (cachep->size);
	}

	total = 0;

	/* Handle writes beyond initialized_size. */
	if (pos + total_count > na->initialized_size) {
		/*
		 * Map runlist between initialized size and place we start
		 * writing at.
		 */
		if (ntfs_attr_map_runlist_range(na, na->initialized_size >>
					vol->cluster_size_bits,
					pos >> vol->cluster_size_bits))
			goto err_out;
		/* Set initialized_size to @pos + @total_count. */
		ctx = ntfs_attr_get_search_ctx(na->ni, NULL);
		if (!ctx)
			goto err_out;
		if (ntfs_attr_lookup(na->type, na->name, na->name_len, 0,
				0, NULL, 0, ctx))
			goto err_out;
		/* If write starts beyond initialized_size, zero the gap. */
		if (pos > na->initialized_size && ntfs_rl_fill_zero(vol,
				na->rl, na->initialized_size,
				pos - na->initialized_size))
			goto err_out;

		ctx->attr->initialized_size = cpu_to_sle64(pos + total_count);
		if (ntfs_mft_record_write(vol, ctx->ntfs_ino->mft_no,
				ctx->mrec)) {
			/*
			 * Undo the change in the in-memory copy and send it
			 * back for writing.
			 */
			ctx->attr->initialized_size =
					cpu_to_sle64(old_initialized_size);
			ntfs_mft_record_write(vol, ctx->ntfs_ino->mft_no,
					ctx->mrec);
			goto err_out;
		}
		na->initialized_size = pos + total_count;
		ntfs_attr_put_search_ctx(ctx);
		ctx = NULL;
		/*
		 * NOTE: At this point the initialized_size in the mft record
		 * has been updated BUT there is random data on disk thus if
		 * we decide to abort, we MUST change the initialized_size
		 * again.
		 */
		need_to.undo_initialized_size = 1;
	}

	/* Find the runlist element containing the vcn. */
	rl = ntfs_attr_find_vcn(na, pos >> vol->cluster_size_bits);
	if (!rl) {
		/*
		 * If the vcn is not present it is an out of bounds write.
		 * However, we already extended the size of the attribute,
		 * so getting this here must be an error of some kind.
		 */
		if (errno == ENOENT)
			errno = EIO;
		goto err_out;
	}

	/* iterate all caches from head */
	cur_item = head;
	b = NULL;
	this_count = 0;

	/*
	 * Scatter the data from the linear data buffer to the volume. Note, a
	 * partial final vcn is taken care of by the @total_count capping of write
	 * length.
	 */
	ofs = pos - (rl->vcn << vol->cluster_size_bits);
	for (; total_count; rl++, ofs = 0) {
		if (rl->lcn == LCN_RL_NOT_MAPPED) {
			rl = ntfs_attr_find_vcn(na, rl->vcn);
			if (!rl) {
				if (errno == ENOENT)
					errno = EIO;
				goto rl_err_out;
			}
			/* Needed for case when runs merged. */
			ofs = pos + total - (rl->vcn << vol->cluster_size_bits);
		}
		if (!rl->length) {
			errno = EIO;
			goto rl_err_out;
		}
		if (rl->lcn < (LCN)0) {
			LCN lcn_seek_from = -1;
			runlist *rlc;
			VCN cur_vcn, from_vcn;	
			if (rl->lcn != (LCN)LCN_HOLE) {
				errno = EIO;
				goto rl_err_out;
			}

			to_write = min(total_count, (rl->length <<
					vol->cluster_size_bits) - ofs);

			/* Instantiate the hole. */
			cur_vcn = rl->vcn;
			from_vcn = rl->vcn + (ofs >> vol->cluster_size_bits);
			
			ntfs_log_trace("Instantiate hole with vcn 0x%llx.\n",
					cur_vcn);
			
			/*
			 * Map whole runlist to be able update mapping pairs
			 * later.
			 */
			if (ntfs_attr_map_whole_runlist(na))
				goto err_out;
			/*
			 * Restore @rl, it probably get lost during runlist
			 * mapping.
			 */
			rl = ntfs_attr_find_vcn(na, cur_vcn);
			if (!rl) {
				ntfs_log_error("BUG! Failed to find run after "
						"mapping whole runlist. Please "
						"report to the %s.\n",
						NTFS_DEV_LIST);
				errno = EIO;
				goto err_out;
			}
			/*
			 * Search backwards to find the best lcn to start
			 * seek from.
			 */
			rlc = rl;
			while (rlc->vcn) {
				rlc--;
				if (rlc->lcn >= 0) {
					lcn_seek_from = rlc->lcn +
							(from_vcn - rlc->vcn);
					break;
				}
			}
			if (lcn_seek_from == -1) {
				/* Backwards search failed, search forwards. */
				rlc = rl;
				while (rlc->length) {
					rlc++;
					if (rlc->lcn >= 0) {
						lcn_seek_from = rlc->lcn -
							(rlc->vcn - from_vcn);
						if (lcn_seek_from < -1)
							lcn_seek_from = -1;
						break;
					}
				}
			}
			/* Allocate clusters to instantiate the hole. */
			rlc = ntfs_cluster_alloc(vol, from_vcn,
						((ofs + to_write - 1) >>
						vol->cluster_size_bits) + 1 +
						rl->vcn - from_vcn,
						lcn_seek_from, DATA_ZONE);
			if (!rlc) {
				eo = errno;
				ntfs_log_trace("Failed to allocate clusters "
						"for hole instantiating.\n");
				errno = eo;
				goto err_out;
			}
			/* Merge runlists. */
			rl = ntfs_runlists_merge(na->rl, rlc);
			if (!rl) {
				eo = errno;
				ntfs_log_trace("Failed to merge runlists.\n");
				if (ntfs_cluster_free_from_rl(vol, rlc)) {
					ntfs_log_trace("Failed to free just "
						"allocated clusters. Leaving "
						"inconsistent metadata. "
						"Run chkdsk\n");
				}
				errno = eo;
				goto err_out;
			}
			na->rl = rl;
			need_to.update_mapping_pairs = 1;
			if (update_from == -1)
				update_from = from_vcn;
			rl = ntfs_attr_find_vcn(na, cur_vcn);
			if (!rl) {
				/*
				 * It's definitely a BUG, if we failed to find
				 * @cur_vcn, because we missed it during
				 * instantiating of the hole.
				 */
				ntfs_log_error("BUG! Failed to find run after "
						"instantiating. Please report "
						"to the %s.\n", NTFS_DEV_LIST);
				errno = EIO;
				goto err_out;
			}
			/* If leaved part of the hole go to the next run. */
			if (rl->lcn < 0)
				rl++;
			/* Now LCN shoudn't be less than 0. */
			if (rl->lcn < 0) {
				ntfs_log_error("BUG! LCN is lesser than 0. "
						"Please report to the %s.\n",
						NTFS_DEV_LIST);
				errno = EIO;
				goto err_out;
			}
			/* Clear non-sparse region from @cur_vcn to @ofs. */
			if (ofs && ntfs_rl_fill_zero(vol, na->rl, cur_vcn <<
					vol->cluster_size_bits, ofs)) {
				goto err_out;
			}
			if (rl->vcn < cur_vcn) {
				/*
				 * Clusters that replaced hole are merged with
				 * previous run, so we need to update offset.
				 */
				ofs += (cur_vcn - rl->vcn) <<
					vol->cluster_size_bits;
			}
			if (rl->vcn > cur_vcn) {
				/*
				 * We left part of the hole, so update we need
				 * to update offset
				 */
				ofs -= (rl->vcn - cur_vcn) <<
					vol->cluster_size_bits;
			}
		}

_next_cache:
		if ( this_count == 0 ) {
			cur_item = cur_item->next;
			if ( cur_item == head ) {
				goto done;
			}
			cur_cache = list_entry(cur_item, struct sc_cache_s, list);
			/* make sure `cur_cache->size > 0 && total_count > 0' elsewhere. */
			this_count = min(cur_cache->size, total_count);
			b = (const u8*)cur_cache->data;
		}

		/* It is a real lcn, write it to the volume. */
		to_write = min(this_count, (rl->length << vol->cluster_size_bits) -
				ofs);

		/* Should only rl exhausted result in to_write == 0, or BUG. */
		if ( to_write == 0 ) {
			continue;
		}

retry:
		ntfs_log_trace("Writing 0x%llx bytes to vcn 0x%llx, lcn 0x%llx,"
				" ofs 0x%llx.\n", to_write, rl->vcn, rl->lcn,
				ofs);

		if (!NVolReadOnly(vol)) {
			s64 wpos = (rl->lcn << vol->cluster_size_bits) + ofs;
			int bsize = 4096; /* FIXME: Test whether we need
					     PAGE_SIZE here. Eg., on IA64. */
			/*
			 * Write 4096 size blocks if it's possible. This will
			 * cause the kernel not to seek and read disk blocks for
			 * filling the end of the buffer which increases write
			 * speed.
			 */
			if (vol->cluster_size >= bsize && !(ofs % bsize) &&
					(to_write % bsize) && ofs + to_write ==
					na->initialized_size) {
				char *cb;
				s64 rounded = (to_write + bsize - 1) &
					~(bsize - 1);

				cb = ntfs_malloc(rounded);
				if (!cb)
					goto err_out;
				memcpy(cb, b, to_write);
				memset(cb + to_write, 0, rounded - to_write);
				written = ntfs_pwrite(vol->dev, wpos, rounded, cb);
				if (written > to_write)
					written = to_write;
				free(cb);
			} else {
				written = ntfs_pwrite(vol->dev, wpos, to_write, b);
			}
		} else {
			written = to_write;
		}
		/* If everything ok, update progress counters and continue. */
		if (written > 0) {
			total += written;
			this_count -= written;
			total_count -= written;
			if ( total_count == 0 ) {
				goto done;
			}
			/* this runlist may still have space available ... */
			if ( this_count == 0 ) {
				/* adjust `ofs' in this runlist. */
				ofs += written;
				goto _next_cache;
			}
			/* this rl must exhaust, try next one. */
			b = (const u8*)b + written;
			continue;
		}
		/* If the syscall was interrupted, try again. */
		if (written == (s64)-1 && errno == EINTR)
			goto retry;
		if (!written)
			errno = EIO;
		goto rl_err_out;
	}

done:
	if (ctx)
		ntfs_attr_put_search_ctx(ctx);
	/* Update mapping pairs if needed. */
	if (need_to.update_mapping_pairs) {
		if (ntfs_attr_update_mapping_pairs(na, update_from)) {
			/* FIXME: We want rollback here. */
			ntfs_log_perror("%s(): Failed to update mapping pairs. "
					"Leaving inconsistent metadata. "
					"Run chkdsk!", __FUNCTION__);
			errno = EIO;
			return -1;
		}
	}
	/* Finally, return the number of bytes written. */
	return total;
rl_err_out:
	eo = errno;
	if (total) {
		if (need_to.undo_initialized_size) {
			if (pos + total > na->initialized_size)
				goto done;
			/*
			 * TODO: Need to try to change initialized_size. If it
			 * succeeds goto done, otherwise goto err_out. (AIA)
			 */
			errno = EOPNOTSUPP;
			goto err_out;
		}
		goto done;
	}
	errno = eo;
err_out:
	eo = errno;
	if (need_to.undo_initialized_size) {
		int err;

		err = 0;
		if (!ctx) {
			ctx = ntfs_attr_get_search_ctx(na->ni, NULL);
			if (!ctx)
				err = 1;
		} else
			ntfs_attr_reinit_search_ctx(ctx);
		if (ctx) {
			err = ntfs_attr_lookup(na->type, na->name,
					na->name_len, 0, 0, NULL, 0, ctx);
			if (!err) {
				na->initialized_size = old_initialized_size;
				ctx->attr->initialized_size = cpu_to_sle64(
						old_initialized_size);
				err = ntfs_mft_record_write(vol,
						ctx->ntfs_ino->mft_no,
						ctx->mrec);
			}
		}
		if (err) {
			/*
			 * FIXME: At this stage could try to recover by filling
			 * old_initialized_size -> new_initialized_size with
			 * data or at least zeroes. (AIA)
			 */
			ntfs_log_error("Eeek! Failed to recover from error. "
					"Leaving metadata in inconsistent "
					"state! Run chkdsk!\n");
		}
	}
	if (ctx)
		ntfs_attr_put_search_ctx(ctx);
	/* Update mapping pairs if needed. */
	if (need_to.update_mapping_pairs)
		ntfs_attr_update_mapping_pairs(na, update_from);
	/* Restore original data_size if needed. */
	if (need_to.undo_data_size && ntfs_attr_truncate(na, old_data_size))
		ntfs_log_trace("Failed to restore data_size.\n");
	errno = eo;
	return -1;
}

static int ntfs_cache_fuse_write(const char *path, const char *buf, size_t size,
		off_t offset, struct fuse_file_info *fi)
{
	NTFS_FUSE_GET_NA(fi);
	s64 res;

	if ( !na->write_cache ) {
		na->write_cache = ntfs_malloc(sizeof(struct ntfs_cache_s));
		ntfs_log_trace("init na <%p> write_cache <%p> \n", na, na->write_cache);
		if ( !na->write_cache ) {
			goto _ori_path; 
		}
		ntfs_cache_init1(na->write_cache);
		ntfs_cache_init2(na->write_cache, na);
	}
	if ( (res = ntfs_cache_do(na->write_cache, buf, size, offset)) == 0  ) {
		return size;
	}
	if ( res == -1 ) {
		/* 
		 * current buffer can not fit into cache, simplily flush 
		 * current cache and accept new cache. 
		 */
		res = ntfs_cache_flush(na->write_cache);
	} else {
		/* 
		 * there are no more caches available now. it's time to 
		 * reclaim some caches. the cache to flush can be any cache
		 * in cached list. And so we can prevent somebody from grabing
		 * caches but never release even if all other caches were exhaust. 
		 */
		res = ntfs_cache_flush_cached_list(1);
	}
	/*
	 * However, this error may be not the error occured when write this na,
	 * but it is during writing this na. so we'd better to tell user 
	 * something wrong happened :( 
	 * we have to face something un-perfect when using write cache.
	 */
	if ( res < 0 ) {
		goto _err;
	}
	if ( ntfs_cache_do(na->write_cache, buf, size, offset) == 0 ) {
		return size;
	}
	/* reclaim ALL caches so that next write can cached. */
	ntfs_cache_flush_cached_list(0);
_ori_path:
	/* original way. */
	return ( ntfs_fuse_write(path, buf, size, offset, fi) );
_err:
	return -errno;
}

#endif /* SC_CACHE */
