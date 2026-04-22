#include <linux/errno.h>
#include <linux/panic.h>
#include <wsl9x/ifsmgr.h>

#define R0_OPENCREATFILE        0xD500  /* Open/Create a file */
#define R0_OPENCREAT_IN_CONTEXT 0xD501  /* Open/Create file in current context */
#define R0_READFILE             0xD600  /* Read a file, no context */
#define R0_WRITEFILE            0xD601  /* Write to a file, no context */
#define R0_READFILE_IN_CONTEXT  0xD602  /* Read a file, in thread context */
#define R0_WRITEFILE_IN_CONTEXT 0xD603  /* Write to a file, in thread context */
#define R0_CLOSEFILE            0xD700  /* Close a file */
#define R0_GETFILESIZE          0xD800  /* Get size of a file */
#define R0_FINDFIRSTFILE        0x4E00  /* Do a LFN FindFirst operation */
#define R0_FINDNEXTFILE         0x4F00  /* Do a LFN FindNext operation */
#define R0_FINDCLOSEFILE        0xDC00  /* Do a LFN FindClose operation */
#define R0_FILEATTRIBUTES       0x4300  /* Get/Set Attributes of a file */
#define R0_RENAMEFILE           0x5600  /* Rename a file */
#define R0_DELETEFILE           0x4100  /* Delete a file */
#define R0_LOCKFILE             0x5C00  /* Lock/Unlock a region in a file */
#define R0_GETDISKFREESPACE     0x3600  /* Get disk free space */
#define R0_READABSOLUTEDISK     0xDD00  /* Absolute disk read */
#define R0_WRITEABSOLUTEDISK    0xDE00  /* Absolute disk write */

typedef struct IFS_Handle* HFILE;

#define MAX_OPEN_FILES 64
static HFILE ifs_open_handles[MAX_OPEN_FILES];

static int find_free_fd(void)
{
	for (int i = 1; i < MAX_OPEN_FILES; i++) {
		if (ifs_open_handles[i] == NULL) {
			return i;
		}
	}

	return -EMFILE;
}

static int new_handle(HFILE handle)
{
	int fd = find_free_fd();
	if (fd < 0) {
		// TODO close handle
		return fd;
	}

	ifs_open_handles[fd] = handle;
	return fd;
}

static HFILE get_handle(int fd)
{
	HFILE handle = ifs_open_handles[fd];
	if (handle) {
		return handle;
	}
	panic("ifsmgr: bad fd: %d", fd);
}

static int translate_err(u16 code)
{
	switch (code) {
	case 1:
		return -ENOSYS;
	case 2:
	case 3:
		return -ENOENT;
	case 4:
		return -EMFILE;
	case 5:
		return -EPERM;
	case 6:
		return -EBADF;
	default:
		panic("unknown error code: %d", code);
	}
}

#define DO_FILEIO \
	do { \
		if (!IFSMgr_Ring0_FileIO(&regs)) { \
			return translate_err(regs.eax); \
		} \
	} while (0)

int IFSMgr_OpenCreateFile(u16 mode, u16 create_attrs, u8 action, const char* path)
{
	struct ifs_regs regs = {
		.eax = R0_OPENCREATFILE,
		.ebx = mode,
		.ecx = create_attrs,
		.edx = action,
		.esi = (u32)path,
	};

	DO_FILEIO;

	return new_handle((HFILE)regs.eax);
}

int IFSMgr_CloseFile(int fd)
{
	struct ifs_regs regs = {
		.eax = R0_CLOSEFILE,
		.ebx = (u32)get_handle(fd),
	};

	DO_FILEIO;

	return 0;
}

ssize_t IFSMgr_ReadFile(int fd, u32 len, u32 offset, u8* buff)
{
	struct ifs_regs regs = {
		.eax = R0_READFILE,
		.ebx = (u32)get_handle(fd),
		.ecx = len,
		.edx = offset,
		.esi = (u32)buff,
	};

	DO_FILEIO;

	return regs.ecx;
}

int IFSMgr_GetFileSize(int fd, u32* size)
{
	struct ifs_regs regs = {
		.eax = R0_GETFILESIZE,
		.ebx = (u32)get_handle(fd),
	};

	DO_FILEIO;

	*size = regs.eax;
	return 0;
}

int IFSMgr_GetFileAttributes(const char* path)
{
	struct ifs_regs regs = {
		.eax = R0_FILEATTRIBUTES,
		.esi = (u32)path,
	};

	DO_FILEIO;

	return regs.ecx;
}
