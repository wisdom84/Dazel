#include "defines.h"
#include "core/Dmemory.h"
#include "core/Dstrings.h"
#include "containers/hashtable.h"
#include "Renderer/renderer_types.inl"
#include "core/logger.h"
#include "resources/resources.inl"



#define MAX_SHADER_INSTANCE 1024
typedef struct shader_system_config{
   u64 max_shaders;  
}shader_system_config;

enum shader_state{
   SHADER_STATE_NOT_CREATED,
   SHADER_STATE_CREATED,
   SHADER_STATE_UNINTIALIZED,
   SHADER_STATE_INITIALIZED
};
typedef struct material_shader_reference{
    u64 reference_id;
    bool is_in_use;
    u32 generation;
}material_shader_reference;

typedef struct material_shader
{
    char name [512];
    u64 id;
    u32 material_id;
    bool use_instance;
    bool use_push_constants;
    bool is_active;
    shader_state state; 
    void*internal_data;
    material_shader_config config;
} material_shader;


typedef struct shader_system_state
{
    hashtable material_shader_reference;
    u64 max_shader_count;
    material_shader shaders[MAX_SHADER_INSTANCE];
} shader_system_state;




bool shader_system_initialize(shader_system_config config, u64*memory_requirement, void*memory);
bool shader_system_shutdown(shader_system_state*state_ptr);
bool shader_system_acquire_shader(const char*shader_name, u32*id);
void shader_system_get_shader_by_id(u32 id, material_shader*shader);
bool shader_system_create_shader(u32 shader_id, u32*material_id);
bool shader_system_destroy_shader(u32 shader_id, u32 material_id); 


