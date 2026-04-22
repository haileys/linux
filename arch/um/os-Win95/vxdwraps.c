#include <linux/compiler_types.h>
#include <wsl9x.h>
#include <wsl9x/vmm.h>
#include <wsl9x/descriptor.h>
#include <wsl9x/mem.h>
#include <wsl9x/task.h>
#include <wsl9x/time.h>

#define VXD_INT_(service) "int $0x20\n" ".long " #service "\n"
#define VXD_INT(service) VXD_INT_(service)

#define JUMP_FLAG 0x8000

#define DEVICE_ID(d) ((d) << 16)

#define VMM_DEVICE 	DEVICE_ID(0x0001)
#define VTD_DEVICE 	DEVICE_ID(0x0005)
#define WSL9X_DEVICE	DEVICE_ID(0x8381)

#define VMM_CALL(service) VXD_INT(VMM_DEVICE | (service))

#define DEF_VXD_JUMP(device, service) { __asm__ volatile (VXD_INT(JUMP_FLAG | (device) | (service))); }
#define DEF_VMM_JUMP(service) DEF_VXD_JUMP(VMM_DEVICE, service)
#define DEF_WSL9X_JUMP(service) DEF_VXD_JUMP(WSL9X_DEVICE, service)

__naked void WSL9X_Debug_Print(const char* str, size_t len)
	DEF_WSL9X_JUMP(WSL9X__DEBUG_PRINT)

__naked void WSL9X_Debug_Printf(const char* str, ...)
	DEF_WSL9X_JUMP(WSL9X__DEBUG_PRINTF)

__naked void WSL9X_Log_Info(const char* str, size_t len)
	DEF_WSL9X_JUMP(WSL9X__LOG_INFO)

__naked void WSL9X_Log_Warn(const char* str, size_t len)
	DEF_WSL9X_JUMP(WSL9X__LOG_WARN)

__naked u32 VMM_SetDescriptor(uint16_t selector, VMM_VMCB* vm, uint32_t desc_hi, uint32_t desc_lo, uint32_t flags)
	DEF_VMM_JUMP(0x007c)

__naked u64 VMM_BuildDescriptorDWORDs(uint32_t base, uint32_t limit, uint8_t type, uint32_t size)
	DEF_VMM_JUMP(0x007a)

__naked u64 VMM_Allocate_GDT_Selector(uint32_t desc_hi, uint32_t desc_lo, uint32_t flags)
	DEF_VMM_JUMP(0x0076)

__naked __noreturn void VMM_Fatal_Error(const char* msg, enum Fatal_Error_Flags flags)
	DEF_VMM_JUMP(0x00be)

__naked uint32_t VMM_PageReserve(uint32_t virt_pfn, uint32_t npages, uint32_t flags)
	DEF_VMM_JUMP(0x011d)

__naked uint32_t VMM_PageCommit(uint32_t pagenum, uint32_t npages, uint32_t pager, uint32_t pagerdata, uint32_t flags)
	DEF_VMM_JUMP(0x011e)

__naked uint32_t VMM_PageDecommit(uint32_t virt_pfn, uint32_t npages, uint32_t flags)
	DEF_VMM_JUMP(0x011f)

__naked uint32_t VMM_PageCommitPhys(uint32_t pagenum, uint32_t npages, uint32_t phys, uint32_t flags)
	DEF_VMM_JUMP(0x0128)

__naked uint32_t VMM_PageModifyPermissions(uint32_t pagenum, uint32_t npages, uint32_t perm_and, uint32_t perm_or)
	DEF_VMM_JUMP(0x0133)

__naked uint32_t VMM_CopyPageTable(uint32_t pagenum, uint32_t npages, uint32_t* buffer, uint32_t flags)
	DEF_VMM_JUMP(0x0061)

__naked HCONTEXT VMM_ContextCreate(void)
	DEF_VMM_JUMP(0x0123)

__naked HCONTEXT VMM_ContextSwitch(HCONTEXT new_context)
	DEF_VMM_JUMP(0x0132)

__naked u64 VTD_Get_Real_Time(void)
	DEF_VXD_JUMP(VTD_DEVICE, 0x0007)

__naked u64 VTD_Get_Date_And_Time(void)
	DEF_VXD_JUMP(VTD_DEVICE, 0x0008)

HTHREAD VMM_Get_Cur_Thread_Handle(void)
{
	HTHREAD out;
	__asm__ volatile(VMM_CALL(0x0108)
		: "=D"(out));
	return out;
}

bool VMM_Call_When_Thread_Switched(void* callback)
{
	int carry;
	__asm__ volatile(VMM_CALL(0x0104)
		: "=@ccc"(carry)
		: "S"(callback));
	return !carry;
}

void VMM_Get_Machine_Info(struct VMM_Machine_Info* info)
{
	u32 eax, ebx, ecx, edx;
	__asm__ volatile(VMM_CALL(0x00bb)
		: "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx));

	info->msdos_ver_major = eax >> 8;
	info->msdos_ver_minor = eax & 0xff;
	info->msdos_oem_serial = ebx >> 8;
	info->machine_model = ebx;
	info->machine_type_flags = ebx >> 16;
	info->sys_config_params = ecx;
	info->equipment_flags = edx;
}
