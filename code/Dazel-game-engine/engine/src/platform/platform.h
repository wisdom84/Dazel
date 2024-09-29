#pragma once
#include "defines.h"
// platform state 
typedef struct platform_state{
   void*internal_state;
}platform_state;
#if PLATFORM_WINDOWS
#include<windowsx.h>
#include <windows.h>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_win32.h>
#include "Renderer/vulkan/vulkan_types.inl"
 typedef struct internal_state{
  HINSTANCE hinstance;
  HWND hwnd;
  VkSurfaceKHR surface;
}internal_state;
#endif
 bool platform_start_up(platform_state*plat_state, const char*application_name, int x, int y, int height,int width);

 void platform_shut_down(platform_state*plat_state);

 bool platform_pump_messages(platform_state*plat_state);
// how the platform handles memory with the application
EXP void*platform_allocate(u64 size, bool aligned);
void platform_free(void*block, char aligned);
void*platform_zero_memory(void*block, u64 size);
void*platform_copy_memory(void*des, const void*source, u64 size);
void*platform_set_memory(void*des, int value, u64 size);
void*platform_allocate_aligned_memory(u64 size, u64 aligned_size);
void platform_free_aligned_memory(void*block);
// the style the platform uses to write to the console 
 void platform_console_write(const char*message, u8 color);
 void platform_console_write_error(const char*message, u8 color);

// getting the time from the platform 
double get_absolute_time();
// how to request the platform to sleep 
void platform_sleep(u64 ms);

// extensions for vulkan support
// void platform_get_required_extensions(const char ***names);

// Suraface for API support
// void platform_set_surface(VkSurfaceKHR&surface);


