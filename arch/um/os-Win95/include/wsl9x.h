#pragma once

typedef struct VMM_TCB* VMM_THREAD_HANDLE;

void VMMTerminateThread(VMM_THREAD_HANDLE thread);

void VMM_lstrcpyn(char* dst, const char* src, size_t size);

void WSL9x_Printk(const char* str, size_t len);
void WSL9x_Log_Info(const char* str, size_t len);
void WSL9x_Log_Warn(const char* str, size_t len);
