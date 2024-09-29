#pragma once

#include "defines.h"

#include "resources/resources.inl"
using namespace dlm;

#define DEFAULT_MATERIAL_NAME "default"

typedef struct  material_system_config{
  u32 max_material_count;
}material_system_config;

bool material_system_initialize(u64*memory_requirements,void*state,material_system_config config);
void material_system_shutdown(void*state);
materials*material_system_acquire(const char*name,bool apply_material);
void material_system_release(const char*name);
materials*material_system_acquire_from_config(material_config config,bool apply_material);
materials*material_system_get_default_material();

