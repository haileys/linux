#pragma once
#include <linux/types.h>
#include <wsl9x/entry.h>

extern const struct wsl9x_services* wsl9x_services;

void __init wsl9x_init_cpu(void);
void __init wsl9x_init_gdt(void);
void __init wsl9x_init_mmu(void);
