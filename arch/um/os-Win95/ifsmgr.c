#include <linux/err.h>
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

#define DO_FILEIO(wrap_err) \
	do { \
		if (!IFSMgr_Ring0_FileIO(&regs)) { \
			return wrap_err(translate_err(regs.eax)); \
		} \
	} while (0)

#define INT_ERR(x) x

HFILE IFSMgr_OpenCreateFile(u16 mode, u16 create_attrs, u8 open_flag, const char* path)
{
	struct ifs_regs regs = {
		.eax = R0_OPENCREATFILE,
		.ebx = mode,
		.ecx = create_attrs,
		.edx = open_flag,
		.esi = (u32)path,
	};

	DO_FILEIO(ERR_PTR);

	return (HFILE)regs.eax;
}

int IFSMgr_CloseFile(HFILE file)
{
	struct ifs_regs regs = {
		.eax = R0_CLOSEFILE,
		.ebx = (u32)file,
	};

	DO_FILEIO(INT_ERR);

	return 0;
}

ssize_t IFSMgr_ReadFile(HFILE file, u32 len, u32 offset, u8* buff)
{
	struct ifs_regs regs = {
		.eax = R0_READFILE,
		.ebx = (u32)file,
		.ecx = len,
		.edx = offset,
		.esi = (u32)buff,
	};

	DO_FILEIO(INT_ERR);

	return regs.ecx;
}

int IFSMgr_GetFileSize(HFILE file, u32* size)
{
	struct ifs_regs regs = {
		.eax = R0_GETFILESIZE,
		.ebx = (u32)file,
	};

	DO_FILEIO(INT_ERR);

	*size = regs.eax;
	return 0;
}

int IFSMgr_GetFileAttributes(const char* path)
{
	struct ifs_regs regs = {
		.eax = R0_FILEATTRIBUTES,
		.esi = (u32)path,
	};

	DO_FILEIO(INT_ERR);

	return regs.ecx;
}
