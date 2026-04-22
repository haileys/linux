#include "linux/stat.h"
#include <linux/fs.h>
#include <wsl9x.h>
#include <wsl9x/ifsmgr.h>
#include "hostfs.h"

int stat_file(const char *path, struct hostfs_stat *p, int fd)
{
	int attr = IFSMgr_GetFileAttributes(path);
	if (attr < 0) {
		return attr;
	}

	int file = IFSMgr_OpenCreateFile(IFS_OPEN_READONLY, 0, IFS_OPEN_IF_EXIST, path);
	if (file < 0) {
		return file;
	}

	p->ino = get_next_ino();
	p->nlink = 1;
	p->uid = 0;
	p->gid = 0;

	p->mode = S_IRUGO;
	if (!(attr & FILE_ATTRIBUTE_READONLY)) {
		p->mode |= S_IWUGO;
	}
	if (attr & FILE_ATTRIBUTE_ARCHIVE) {
		p->mode |= S_IXUGO;
	}
}

extern int access_file(char *path, int r, int w, int x)
{
	unimplemented();
}

extern int open_file(char *path, int r, int w, int append)
{
	unimplemented();
}

extern void *open_dir(char *path, int *err_out)
{
	unimplemented();
}

extern void seek_dir(void *stream, unsigned long long pos)
{
	unimplemented();
}

extern char *read_dir(void *stream, unsigned long long *pos_out,
		      unsigned long long *ino_out, int *len_out,
		      unsigned int *type_out)
{
	unimplemented();
}

extern void close_file(void *stream)
{
	unimplemented();
}

extern int replace_file(int oldfd, int fd)
{
	unimplemented();
}

extern void close_dir(void *stream)
{
	unimplemented();
}

extern int read_file(int fd, unsigned long long *offset, char *buf, int len)
{
	unimplemented();
}

extern int write_file(int fd, unsigned long long *offset, const char *buf,
		      int len)
{
	unimplemented();
}

extern int lseek_file(int fd, long long offset, int whence)
{
	unimplemented();
}

extern int fsync_file(int fd, int datasync)
{
	unimplemented();
}

extern int file_create(char *name, int mode)
{
	unimplemented();
}

extern int set_attr(const char *file, struct hostfs_iattr *attrs, int fd)
{
	unimplemented();
}

extern int make_symlink(const char *from, const char *to)
{
	unimplemented();
}

extern int unlink_file(const char *file)
{
	unimplemented();
}

extern int do_mkdir(const char *file, int mode)
{
	unimplemented();
}

extern int hostfs_do_rmdir(const char *file)
{
	unimplemented();
}

extern int do_mknod(const char *file, int mode, unsigned int major,
		    unsigned int minor)
{
	unimplemented();
}

extern int link_file(const char *to, const char *from)
{
	unimplemented();
}

extern int hostfs_do_readlink(char *file, char *buf, int size)
{
	unimplemented();
}

extern int rename_file(char *from, char *to)
{
	unimplemented();
}

extern int rename2_file(char *from, char *to, unsigned int flags)
{
	unimplemented();
}

extern int do_statfs(char *root, long *bsize_out, long long *blocks_out,
		     long long *bfree_out, long long *bavail_out,
		     long long *files_out, long long *ffree_out,
		     void *fsid_out, int fsid_size, long *namelen_out)
{
	unimplemented();
}
