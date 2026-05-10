#pragma once
#include <linux/types.h>

typedef struct IFS_Handle* HFILE;

#define IFS_OPEN_READONLY  0
#define IFS_OPEN_WRITEONLY 1
#define IFS_OPEN_READWRITE 2

#define IFS_FAIL_IF_EXIST 	(0x00)
#define IFS_OPEN_IF_EXIST 	(0x01)
#define IFS_REPLACE_IF_EXIST 	(0x02)
#define IFS_CREATE	 	(0x10)
HFILE IFSMgr_OpenCreateFile(u16 mode, u16 create_attrs, u8 action, const char* path);
int IFSMgr_CloseFile(HFILE file);

ssize_t IFSMgr_ReadFile(HFILE file, u32 len, u32 offset, u8* buff);

int IFSMgr_GetFileSize(HFILE file, u32* size);

#define FILE_ATTRIBUTE_READONLY     0x01    /* read-only file */
#define FILE_ATTRIBUTE_HIDDEN       0x02    /* hidden file */
#define FILE_ATTRIBUTE_SYSTEM       0x04    /* system file */
#define FILE_ATTRIBUTE_LABEL        0x08    /* volume label */
#define FILE_ATTRIBUTE_DIRECTORY    0x10    /* subdirectory */
#define FILE_ATTRIBUTE_ARCHIVE      0x20    /* archived file/directory */
int IFSMgr_GetFileAttributes(const char* path);

struct ifs_regs {
	u32 eax;
	u32 ebx;
	u32 ecx;
	u32 edx;
	u32 esi;
};

bool IFSMgr_Ring0_FileIO(struct ifs_regs* regs);
