#pragma once 

#include "resources/resources.inl"

typedef struct resource_system_config{
   u32 max_loader_count;
   const char*asset_base_path;
}resource_system_config;

typedef struct resource_loader{
  u32 id;
  resource_types  type;
  const char* custom_type;
  const char* type_path;
  bool(*load)(struct resource_loader*self, const char*name, resource*out_resource);
  void(*unload)(struct resource_loader*self, resource*resource);
}resource_loader;

bool resource_system_intialize(u64*memory_requirement,void*state, resource_system_config config);
void resource_system_shutdown(void*state);

EXP bool resource_system_register_loader(resource_loader loader);
EXP bool resource_system_load(const char*name, resource_types type, resource*out_resource);
EXP bool resource_system_load_custom(const char* name, const char* custom_type, resource*out_resource);

EXP void resource_system_unload(resource*resource);

EXP const char* resource_system_base_path();

