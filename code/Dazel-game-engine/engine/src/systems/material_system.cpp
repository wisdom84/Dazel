#include "material_system.h"
#include "memory/linear_allocator.h"
#include "core/logger.h"
#include "core/Dstrings.h"
#include "texture_system.h"
#include "shader_system.h"
#include "containers/hashtable.h"
#include "Renderer/renderer_frontend.h"
#include "core/Dmemory.h"
#include "resource_system.h"

typedef struct material_system_state{
  material_system_config system_config;
  materials default_material;
  materials*registered_materials;
  hashtable registered_material_tables;
}material_system_state;


typedef struct material_reference{
   u64 reference_count;
   u32 handle;
   bool auto_release;
}material_reference;

static material_system_state * state_ptr = 0;

bool create_default_material(material_system_state*state);
void destroy_material(materials*m);
bool load_material(material_config config, materials*m);
bool load_configuration_file(const char*name, material_config*out_config);



bool material_system_initialize(u64*memory_requirements,void*state,material_system_config config){
     if(config.max_material_count == 0){
    DFATAL("material_system_initialize - config.max_material_count must be > 0");
    return false;
   }
   u64  struct_requirement  = sizeof(material_system_state);
   u64 array_requirement = sizeof(materials)*config.max_material_count;
   u64 hashtable_requirement = sizeof(material_reference)*config.max_material_count;
   *memory_requirements = struct_requirement + array_requirement + hashtable_requirement;

   if(!state){
     return true;
   }
   state_ptr = (material_system_state*)state;
   state_ptr->system_config = config;

   // array block 
   void* array_block = (char*)state + struct_requirement;
   state_ptr->registered_materials =(materials*) array_block;

   // hash table block
   void*hashtable_block = (char*)array_block + array_requirement;
   hash_table_create(sizeof(material_reference), config.max_material_count, hashtable_block, false,&state_ptr->registered_material_tables);

   // fill the hashtable with invalid id references 
   material_reference invalid_ref;
   invalid_ref.auto_release = false;
   invalid_ref.handle = INVALID_ID;
   invalid_ref.reference_count = 0;
   hash_table_fill(&state_ptr->registered_material_tables, &invalid_ref);

   // invalidate all texture in the array
   u32 count = state_ptr->system_config.max_material_count;
   for(u32 i = 0; i <count; i++){
    state_ptr->registered_materials[i].id = INVALID_ID;
    state_ptr->registered_materials[i].generations = INVALID_ID;
    state_ptr->registered_materials[i].internal_id = INVALID_ID;
    Dzero_memory(&state_ptr->registered_materials[i].diffuse_color, sizeof(vec4));
    Dzero_memory(&state_ptr->registered_materials[i].diffuse_map, sizeof(texture_map));
   }
  // set up defualt textures
//   if(!create_default_material(state_ptr)){
//     DFATAL("failed to load up default material. material system booting out...");
//     return false;
//   };
   return true;
};


void material_system_shutdown(void*state){
   material_system_state*s = (material_system_state*)state;
   if(s){
    u32 count = s->system_config.max_material_count;
    for(u32 i =0; i < count; i++){
        if(s->registered_materials[i].id != INVALID_ID){
            destroy_material(&s->registered_materials[i]);
        }
    }
   }
   //destroying default material
//    destroy_material(&s->default_material);
   s=0;
};


materials*material_system_acquire(const char*name, bool apply_material){
    material_config config;
    if(!load_configuration_file(name,&config))
    {
        DFATAL("failed to laod materials configuration file. material system booting out");
        return nullptr;
    };

   return material_system_acquire_from_config(config, apply_material);

};

materials*material_system_acquire_from_config(material_config config, bool apply_material){
    if(string_equal(config.name,DEFAULT_TEXTURE_NAME)){
        DWARNING("matrial_system_acquire called for default material. use get_default_material for matrial 'default'");
        return &state_ptr->default_material;
    }
   material_reference ref;
   if(state_ptr && hash_table_get(&state_ptr->registered_material_tables, config.name,&ref)){
        if(ref.reference_count == 0){
            ref.auto_release = config.auto_release;
        }
        ref.reference_count++;
        if(ref.handle == INVALID_ID){
                u32 count =state_ptr->system_config.max_material_count;
                materials*m=0;
                for(u32 i = 0;  i < count; i++){
                    if(state_ptr->registered_materials[i].id == INVALID_ID){
                        ref.handle = i;
                        m = &state_ptr->registered_materials[i];
                        break;
                    }
                }
                if(!m || ref.handle == INVALID_ID){
                    DFATAL("material_system_acquire - material system can not hold anymore materials  adjust the configuration file");
                    return nullptr;
                }
                //create a new texture
                if(!load_material(config,m)){
                    DERROR("failed to load texture '%s'", config.name);
                    return nullptr;
                }
                if(m->generations == INVALID_ID){
                    m->generations = 0;
                }else{
                    m->generations++;
                }
            m->id = ref.handle;
            DTRACE("material '%s' does not exist. created and ref_count is now %i.", config.name , ref.reference_count);
        }
        else{
         
            // DTRACE("material '%s' already exists, ref_count increased to %i.", config.name , ref.reference_count);
        }
        // m->apply_material = apply_material;
        hash_table_set(&state_ptr->registered_material_tables, config.name,&ref);
        return  &state_ptr->registered_materials[ref.handle];
   }
   DERROR("material_system_acquire failed to aquire material '%s'. nullptr will be returned.", config.name);
   return nullptr;
};

void material_system_release(const char*name){
    
   if(string_equal(name, DEFAULT_MATERIAL_NAME)){
      return;
   }

   material_reference ref;
 
   if(hash_table_get(&state_ptr->registered_material_tables,name,&ref)){
        if(ref.handle == INVALID_ID){
            DWARNING("Tried to release a non-existent material: %s", name);
            return;
        }
        ref.reference_count--;
        char dst_name_copy[512];
        materials*temp = &state_ptr->registered_materials[ref.handle];
        if((const char *)temp->name){ // temporary getting the name of the material
                string_n_copy(dst_name_copy, temp->name, TEXTURE_NAME_MAX_LENGTH);
                temp=0;
            }
      if(ref.reference_count == 0 &&  ref.auto_release){
            materials*m = &state_ptr->registered_materials[ref.handle];
            destroy_material(m);
            Dzero_memory(m, sizeof(materials));
            m->id = INVALID_ID;
            m->generations = INVALID_ID;

            //Reset reference
            ref.handle = INVALID_ID;
            ref.auto_release = false;
            DTRACE("Released material '%s' material unloaded because reference count = 0 and auto_release is true.", dst_name_copy);
        }
        else{
                  if(ref.auto_release){
                                    DTRACE("Released material '%s'. now has a refernce count of '%i'auto_release is true.", name ,ref.reference_count);
                  }
                  else{
                                 DTRACE("Released material '%s'. now has a refernce count of '%i'auto_release is false.", name ,ref.reference_count);
                  }
        }

        // update the  hashtable 
        hash_table_set(&state_ptr->registered_material_tables, name,&ref);
       return;
   }
  DERROR("material_system_release failed to release material '%s'.", name); 
};

bool load_material(material_config config, materials*m){
    Dzero_memory(m,sizeof(materials));
    //name
    string_n_copy(m->name, config.name,MATERIAL_NAME_MAX_LENGTH);
     m->type = config.type;
    //diffuse color
    m->diffuse_color = config.diffuse_color;
    m->shineness = config.shineness;


    if(string_length(config.diffuse_map_name) > 0){
        m->diffuse_map.use = TEXTURE_USE_MAP_DIFFUSE;
        m->diffuse_map.texture = texture_system_acquire(config.diffuse_map_name,true);
        string_n_copy(m->diffuse_map.texture->name, config.diffuse_map_name, TEXTURE_NAME_MAX_LENGTH);
        if(!m->diffuse_map.texture){
            DWARNING("unable to load texture %s for material %s, using default.", config.diffuse_map_name,m->name);
            m->diffuse_map.texture= texture_system_get_defualt_texture();
        }
    }
    else{
        m->diffuse_map.use = TEXTURE_USE_UNKNOWN;
        m->diffuse_map.texture = nullptr;
    }

    if(string_length(config.specular_map_name) > 0){
        m->specular_map.use = TEXTURE_USE_MAP_SPECULAR;
        m->specular_map.texture = texture_system_acquire(config.specular_map_name,true);
        string_n_copy(m->specular_map.texture->name, config.specular_map_name, TEXTURE_NAME_MAX_LENGTH);
        if(!m->specular_map.texture){
            DWARNING("unable to load texture %s for material %s, using default.", config.specular_map_name,m->name);
            m->specular_map.texture= texture_system_get_defualt_texture();
        }
    }
    else{
        m->specular_map.use = TEXTURE_USE_UNKNOWN;
        m->specular_map.texture = nullptr;
    }

    if(string_length(config.normal_map_name) > 0){
        m->normal_map.use = TEXTURE_USE_MAP_NORMAL;
        m->normal_map.texture = texture_system_acquire(config.normal_map_name,true);
        string_n_copy(m->normal_map.texture->name, config.normal_map_name, TEXTURE_NAME_MAX_LENGTH);
        if(!m->normal_map.texture){
            DWARNING("unable to load texture %s for material %s, using default.", config.normal_map_name,m->name);
            m->normal_map.texture= texture_system_get_defualt_texture();
        }
    }
    else{
        m->normal_map.use = TEXTURE_USE_UNKNOWN;
        m->normal_map.texture = nullptr;
    }
    

    if(!shader_system_acquire_shader(config.shader_name, &m->shader_id)){
        DERROR("shader_system_acquire_shader failed to acquire a shader for the material");
        return false;
    }
    if(!shader_system_create_shader(m->shader_id,&m->bound_instance_id)){
        DERROR("shader_system_failed to create shader aquired by the material");
        return false;
    }

    
  return true;
}
void destroy_material(materials*m){
    DTRACE("Destroying materials %s",m->name);
    if(m->diffuse_map.texture){
        texture_system_release(m->diffuse_map.texture->name);
    }

    // renderer_destroy_material(m);
    shader_system_destroy_shader(m->shader_id,m->id);
    Dzero_memory(m, sizeof(materials));
    m->id = INVALID_ID;
    m->internal_id = INVALID_ID;
    m->generations = INVALID_ID;
};

bool create_default_material(material_system_state*state){
    Dzero_memory(&state_ptr->default_material, sizeof(materials));
    state->default_material.id= INVALID_ID;
    state->default_material.generations = INVALID_ID;
    state->default_material.internal_id = INVALID_ID;
    string_n_copy(state->default_material.name,DEFAULT_MATERIAL_NAME,MATERIAL_NAME_MAX_LENGTH);
    state->default_material.diffuse_color = vec4_create(1.0f,1.0f,1.0f,1.0f);// white
    state->default_material.diffuse_map.use = TEXTURE_USE_MAP_DIFFUSE;
    state->default_material.diffuse_map.texture = texture_system_get_defualt_texture();
    // if(!renderer_create_material(&state->default_material)){
    //     DFATAL("failed to acquire renderer resources for default material Application booting out ");
    //     return false;
    // }

    if(!shader_system_acquire_shader("built_in_shader", &state_ptr->default_material.shader_id)){
        DERROR("shader_system_acquire_shader failed to acquire a shader for the material");
        return false;
    }
    if(!shader_system_create_shader(state_ptr->default_material.shader_id,&state_ptr->default_material.id)){
        DERROR("shader_system_failed to create shader aquired by the material");
        return false;
    }

   return true;
};


bool load_configuration_file(const char*name, material_config*out_config){
    resource material;
    if(!resource_system_load(name, RESOURCE_TYPE_MATERIAL, &material)){
        DERROR("failed to load material configuration file");
        return false;
    }
    material_config*configuration = (material_config*)material.data;
    string_n_copy(out_config->name, configuration->name, MATERIAL_NAME_MAX_LENGTH);
    string_n_copy(out_config->diffuse_map_name, configuration->diffuse_map_name, TEXTURE_NAME_MAX_LENGTH);
    string_n_copy(out_config->specular_map_name, configuration->specular_map_name, TEXTURE_NAME_MAX_LENGTH);
    string_n_copy(out_config->normal_map_name, configuration->normal_map_name, TEXTURE_NAME_MAX_LENGTH);
    string_n_copy(out_config->shader_name, configuration->shader_name, SHADER_NAME_MAX_LENGTH);
    out_config->diffuse_color = configuration->diffuse_color;
    out_config->shineness = configuration->shineness;
    out_config->auto_release = true;
    out_config->type = configuration->type;
    resource_system_unload(&material);
    return true;
};

materials*material_system_get_default_material(){
    if(state_ptr){
        return &state_ptr->default_material;
    }
  DWARNING("call to default material without initialization of material system or call to default meterial after shutdown of material system returning nullptr");
  return nullptr;
}

