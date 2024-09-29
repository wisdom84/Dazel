#pragma once 
#include "defines.h"
#include "Renderer/renderer_types.inl"

typedef struct texture_system_config{
  u32 max_texture_count;
}texture_system_config;

#define DEFAULT_TEXTURE_NAME "default"

bool texture_system_initialize(u64*memory_requirements,void*state,texture_system_config config);
void texture_system_shutdown(void*state);

Texture*texture_system_acquire(const char*name, bool auto_release);
void texture_system_release(const char*name);

Texture*texture_system_get_defualt_texture();

bool load_texture(const char*texture_name, Texture*t);