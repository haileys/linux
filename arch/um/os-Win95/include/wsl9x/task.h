#pragma once
#include "prelude.h"

typedef struct {
	uint32_t EDI;           // Client's EDI
	uint32_t ESI;           // Client's ESI
	uint32_t EBP;           // Client's EBP
	uint32_t res0;          // ESP at pushall
	uint32_t EBX;           // Client's EBX
	uint32_t EDX;           // Client's EDX
	uint32_t ECX;           // Client's ECX
	uint32_t EAX;           // Client's EAX
	uint32_t Error;         // Dword error code
	uint32_t EIP;           // EIP
	uint16_t CS;            // CS
	uint16_t res1;          //   (padding)
	uint32_t EFlags;        // EFLAGS
	uint32_t ESP;           // ESP
	uint16_t SS;            // SS
	uint16_t res2;          //   (padding)
	uint16_t ES;            // ES
	uint16_t res3;          //   (padding)
	uint16_t DS;            // DS
	uint16_t res4;          //   (padding)
	uint16_t FS;            // FS
	uint16_t res5;          //   (padding)
	uint16_t GS;            // GS
	uint16_t res6;          //   (padding)
	uint32_t Alt_EIP;
	uint16_t Alt_CS;
	uint16_t res7;
	uint32_t Alt_EFlags;
	uint32_t Alt_ESP;
	uint16_t Alt_SS;
	uint16_t res8;
	uint16_t Alt_ES;
	uint16_t res9;
	uint16_t Alt_DS;
	uint16_t res10;
	uint16_t Alt_FS;
	uint16_t res11;
	uint16_t Alt_GS;
	uint16_t res12;
} VMM_Client_Regs;


typedef struct {
    uint32_t VM_Status;         // VM status flags
    uint32_t High_Linear;       // Address of VM mapped high
    VMM_Client_Regs* Client_Pointer;
    uint32_t VMID;
    uint32_t Signature;
} VMM_VMCB;

#define VMSTAT_EXCLUSIVE_BIT    0x00    /* VM is exclusive mode */
#define VMSTAT_EXCLUSIVE        (1L << VMSTAT_EXCLUSIVE_BIT)
#define VMSTAT_BACKGROUND_BIT   0x01    /* VM runs in background */
#define VMSTAT_BACKGROUND       (1L << VMSTAT_BACKGROUND_BIT)
#define VMSTAT_CREATING_BIT 0x02    /* In process of creating */
#define VMSTAT_CREATING         (1L << VMSTAT_CREATING_BIT)
#define VMSTAT_SUSPENDED_BIT    0x03    /* VM not scheduled */
#define VMSTAT_SUSPENDED        (1L << VMSTAT_SUSPENDED_BIT)
#define VMSTAT_NOT_EXECUTEABLE_BIT 0x04 /* VM partially destroyed */
#define VMSTAT_NOT_EXECUTEABLE  (1L << VMSTAT_NOT_EXECUTEABLE_BIT)
#define VMSTAT_PM_EXEC_BIT  0x05    /* Currently in PM app */
#define VMSTAT_PM_EXEC          (1L << VMSTAT_PM_EXEC_BIT)
#define VMSTAT_PM_APP_BIT   0x06    /* PM app present in VM */
#define VMSTAT_PM_APP           (1L << VMSTAT_PM_APP_BIT)
#define VMSTAT_PM_USE32_BIT 0x07    /* PM app is 32-bit */
#define VMSTAT_PM_USE32         (1L << VMSTAT_PM_USE32_BIT)
#define VMSTAT_VXD_EXEC_BIT 0x08    /* Call from VxD */
#define VMSTAT_VXD_EXEC         (1L << VMSTAT_VXD_EXEC_BIT)
#define VMSTAT_HIGH_PRI_BACK_BIT 0x09   /* High pri background */
#define VMSTAT_HIGH_PRI_BACK    (1L << VMSTAT_HIGH_PRI_BACK_BIT)
#define VMSTAT_BLOCKED_BIT  0x0A    /* Blocked on semaphore */
#define VMSTAT_BLOCKED          (1L << VMSTAT_BLOCKED_BIT)
#define VMSTAT_AWAKENING_BIT    0x0B    /* Woke up after blocked */
#define VMSTAT_AWAKENING        (1L << VMSTAT_AWAKENING_BIT)
#define VMSTAT_PAGEABLEV86BIT   0x0C    /* part of V86 is pageable (PM app) */
#define VMSTAT_PAGEABLEV86_BIT  VMSTAT_PAGEABLEV86BIT
#define VMSTAT_PAGEABLEV86      (1L << VMSTAT_PAGEABLEV86BIT)
#define VMSTAT_V86INTSLOCKEDBIT 0x0D    /* Locked regardless of pager type */
#define VMSTAT_V86INTSLOCKED_BIT VMSTAT_V86INTSLOCKEDBIT
#define VMSTAT_V86INTSLOCKED    (1L << VMSTAT_V86INTSLOCKEDBIT)
#define VMSTAT_IDLE_TIMEOUT_BIT 0x0E    /* Scheduled by time-slicer */
#define VMSTAT_IDLE_TIMEOUT     (1L << VMSTAT_IDLE_TIMEOUT_BIT)
#define VMSTAT_IDLE_BIT         0x0F    /* VM has released time slice */
#define VMSTAT_IDLE             (1L << VMSTAT_IDLE_BIT)
#define VMSTAT_CLOSING_BIT  0x10    /* Close_VM called for VM */
#define VMSTAT_CLOSING          (1L << VMSTAT_CLOSING_BIT)
#define VMSTAT_TS_SUSPENDED_BIT 0x11    /* VM suspended by */
#define VMSTAT_TS_SUSPENDED     (1L << VMSTAT_TS_SUSPENDED_BIT)
#define VMSTAT_TS_MAXPRI_BIT    0x12    /* this is fgd_pri 10,000 internally*/
#define VMSTAT_TS_MAXPRI        (1L << VMSTAT_TS_MAXPRI_BIT)

#define VMSTAT_USE32_MASK   (VMSTAT_PM_USE32 | VMSTAT_VXD_EXEC)


typedef struct {
	uint32_t Flags;             // 0x00  Thread status flags
	uint32_t Reserved1;         // 0x04  Used internally by VMM
	uint32_t Reserved2;         // 0x08  Used internally by VMM
	uint32_t Signature;	    // 0x0c
	VMM_Client_Regs* ClientPtr; // 0x10  Client registers of thread
	VMM_VMCB* VMHandle;         // 0x14  VM that thread is part of
	uint16_t ThreadId;          // 0x18  Unique Thread ID
	uint16_t PMLockOrigSS;      // 0x1a  Original SS:ESP before lock stack
	uint32_t PMLockOrigESP;     // 0x1c
	uint32_t PMLockOrigEIP;     // 0x20  Original CS:EIP before lock stack
	uint32_t PMLockStackCount;  // 0x24
	uint16_t PMLockOrigCS;      // 0x28
	uint16_t PMPSPSelector;     // 0x2a
	uint32_t ThreadType;        // 0x2c  dword passed to VMMCreateThread
	uint16_t pad1;              // 0x30  reusable; for dword align
	uint8_t  pad2;              // 0x32  reusable; for dword align
	uint8_t  extErrLocus;       // 0x33  extended error Locus
	uint16_t extErr;            // 0x34  extended error Code
	uint8_t  extErrAction;      //            "   "   Action
	uint8_t  extErrClass;       //            "   "   Class
	uint32_t extErrPtr;         //            "   pointer
} VMM_TCB;

#define THFLAG_SUSPENDED_BIT        0x03   // Thread not scheduled
#define THFLAG_SUSPENDED                   (1L << THFLAG_SUSPENDED_BIT)
#define THFLAG_NOT_EXECUTEABLE_BIT  0x04   // Thread partially destroyed
#define THFLAG_NOT_EXECUTEABLE             (1L << THFLAG_NOT_EXECUTEABLE_BIT)
#define THFLAG_THREAD_CREATION_BIT  0x08   // Thread in status nascendi
#define THFLAG_THREAD_CREATION             (1L << THFLAG_THREAD_CREATION_BIT)
#define THFLAG_THREAD_BLOCKED_BIT   0x0A   // Blocked on semaphore
#define THFLAG_THREAD_BLOCKED              (1L << THFLAG_THREAD_BLOCKED_BIT)
#define THFLAG_RING0_THREAD_BIT     0x1C   // thread runs only at ring 0
#define THFLAG_RING0_THREAD	           (1L << THFLAG_RING0_THREAD_BIT)
#define THFLAG_CHARSET_BITS     0x10   // Default character set
#define THFLAG_CHARSET_MASK        (3L << THFLAG_CHARSET_BITS)
#define THFLAG_ANSI            (0L << THFLAG_CHARSET_BITS)
#define THFLAG_OEM             (1L << THFLAG_CHARSET_BITS)
#define THFLAG_UNICODE             (2L << THFLAG_CHARSET_BITS)
#define THFLAG_RESERVED            (3L << THFLAG_CHARSET_BITS)
#define THFLAG_EXTENDED_HANDLES_BIT 0x12   // Thread uses extended file handles
#define THFLAG_EXTENDED_HANDLES            (1L << THFLAG_EXTENDED_HANDLES_BIT)


typedef VMM_TCB* VMM_THREAD_HANDLE;

void VMMTerminateThread(VMM_THREAD_HANDLE thread);

void VMM_Save_Client_State(VMM_Client_Regs* regs);
void VMM_Restore_Client_State(const VMM_Client_Regs* regs);

void VMM_Begin_Critical_Section(uint32_t flags);
void VMM_End_Critical_Section(void);
