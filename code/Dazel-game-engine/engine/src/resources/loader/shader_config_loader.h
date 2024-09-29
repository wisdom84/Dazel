#pragma once 

#include "systems/resource_system.h"
typedef void(*PFN_string_proccess)(char*buffer_type,const char*characters,material_shader_config*config);
resource_loader shader_config_loader_create();

 void sub_division(char*buffer_data_type,char*buffer, char symbols[1], char current_sysbol, u64 index, u64 bounds, PFN_string_proccess func, material_shader_config*config);

void string_process( char*buffer_type,const char*characters,material_shader_config*config);

