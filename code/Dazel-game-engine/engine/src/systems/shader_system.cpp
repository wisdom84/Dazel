#include "shader_system.h"
#include "Renderer/renderer_frontend.h"
#include "systems/resource_system.h"
static shader_system_state *state_ptr = 0;
bool load_configuration_file(const char*shader_name,material_shader_config*config);
void string_data_tranfer(char data[][128], char dst[][128]);
bool shader_system_initialize(shader_system_config config, u64*memory_requirement, void*memory){
   if(config.max_shaders == 0){
    DERROR("can not stand up the shader system with zero material type");
    return false;
   }
   u64 state_requirement = sizeof(shader_system_state);
   u64 hashtable_requirements = sizeof(material_shader_reference)*config.max_shaders;

   if(!memory){
      *memory_requirement = state_requirement + hashtable_requirements;
      return true;
   }
   //  memory = Dallocate_memory((state_requirement+hashtable_requirements), MEMORY_TAG_SHADER);
    state_ptr = (shader_system_state*)memory;
    void*hash_table_block =(char*)memory+state_requirement;
    hash_table_create(sizeof(material_shader_reference), config.max_shaders,hash_table_block, false,&state_ptr->material_shader_reference);
    state_ptr->max_shader_count = config.max_shaders;
    // invalidating the id and generation
    for(u32 i=0; i < config.max_shaders; i++){
        state_ptr->shaders[i].id = INVALID_ID; 
        state_ptr->shaders[i].state = SHADER_STATE_NOT_CREATED;
    }
    // filling the hash table with invalid ids
    material_shader_reference shader_refernce;
    shader_refernce.generation = INVALID_ID;
    shader_refernce.is_in_use = false;
    shader_refernce.reference_id =  INVALID_ID;
    hash_table_fill(&state_ptr->material_shader_reference, &shader_refernce);
    return true;
}

bool shader_system_shutdown(shader_system_state*state_ptr){
     if(!state_ptr){
        DERROR("trying to shutdown the shader system state before initialization");
        return false;
     } 
    for(u32 i= 0; i < state_ptr->max_shader_count; i++){
        if(state_ptr->shaders[i].id != INVALID_ID){
           shader_system_destroy_shader(state_ptr->shaders[i].id, state_ptr->shaders[i].material_id);
        } 
    }
    state_ptr = nullptr;
    return true;
}
bool shader_system_acquire_shader(const char*shader_name, u32*id){
   if(!state_ptr){
      DERROR("tried to acquire shader resources before initialization of shader system");
      return false;
   }
   // check if the shader already exist  or has already been generated
   material_shader_reference reference; 
  if(!hash_table_get(&state_ptr->material_shader_reference,shader_name,&reference)){
    DERROR(" hash_table_get failed to get a slot");
    return false;
  };
  if(reference.reference_id != INVALID_ID){
   *id = reference.reference_id;
  }
  else{
      // load a configuration file for the shader 
         material_shader_config config; 
         if(!load_configuration_file(shader_name,&config)){
            DERROR("failed to load shader info");
            return false;
         };

         // check for a free shader slot
         material_shader*shader_obj=0; 
         for(u32 i= 0; i < state_ptr->max_shader_count; i++){
            if(state_ptr->shaders[i].id == INVALID_ID){
               shader_obj = &state_ptr->shaders[i];
               reference.reference_id = i;
               *id = i;
                break;
            };
         }
         if(!shader_obj){
            DERROR("no free slots for new shaders request more memory storage");
            return false;
         }
         shader_obj->config = config;
         string_n_copy(shader_obj->name, config.shader_name, SHADER_NAME_MAX_LENGTH);
         shader_obj->use_instance = config.use_instance;
         shader_obj->use_push_constants = config.use_local;
         shader_obj->id = reference.reference_id;
       if(!renderer_create_material_shader(shader_obj)){
         DERROR(" renderer failed to create material shader");
         return false;
       }
       shader_obj->is_active = true;
       reference.is_in_use = true;
       hash_table_set(&state_ptr->material_shader_reference,shader_name,&reference); 
       renderer_activate_shader(shader_obj->id, shader_obj->is_active); 
       shader_obj->state = SHADER_STATE_CREATED;
  }
   
   return true;
};

bool load_configuration_file(const char*shader_name,material_shader_config*config){
  // load up the resources 
  resource shader_config;
 if(!resource_system_load(shader_name,RESOURCE_TYPE_SHADER,&shader_config)){
   DERROR("failed to load shader config file for shader");
   return false;
 };
  material_shader_config*data = (material_shader_config*)shader_config.data;
  config->attribute_count = data->attribute_count;
  config->stage_count = data->stage_count;
  config->uniform_count = data->uniform_count;
  config->use_instance = data->use_instance;
  config->use_local = data->use_local;
  config->sampler_count = data->sampler_count;
  string_n_copy(config->shader_name, data->shader_name, SHADER_NAME_MAX_LENGTH);
  string_copy(config->renderpass, data->renderpass);
  string_data_tranfer(config->attribute_names, data->attribute_names);
  string_data_tranfer(config->attribute_type, data->attribute_type);
  string_data_tranfer(config->uniform_names, data->uniform_names);
  string_data_tranfer(config->uniform_scope, data->uniform_scope);
  string_data_tranfer(config->uniform_type, data->uniform_type);
  string_data_tranfer(config->sampler_name, data->sampler_name);
  string_data_tranfer(config->sampler_scope, data->sampler_scope);
  string_data_tranfer(config->stage_names, data->stage_names); 
  return true;
}

void string_data_tranfer(char dst[][128], char data[][128]){
    int index = 0;
    char*p = data[index];
    char*t = dst[index];
    if(p){
       while(*p){
            if(!p || !t){
               DINFO("end of array: prevention from going out of bounds");
               return;
            }
         string_n_copy(dst[index],p,128);   
         index++;
         p = data[index];
         t = dst[index];
       }
    }  
 }
 void shader_system_get_shader_by_id(u32 id, material_shader*shader){
   *shader = state_ptr->shaders[id];
 };

 bool shader_system_create_shader(u32 shader_id, u32*material_id){
  if (renderer_acquire_shader_resources(&state_ptr->shaders[shader_id], material_id)){
      state_ptr->shaders[shader_id].material_id = *material_id;
      state_ptr->shaders[shader_id].state = SHADER_STATE_INITIALIZED;
    return true;
  };
  return false;  
 };


 bool shader_system_destroy_shader(u32 shader_id, u32 material_id){
   if(!state_ptr){
      DWARNING("shader system not intialized can not destroy shader");
      return false;
   }
  material_shader*shader_obj =&state_ptr->shaders[shader_id];
  if(shader_obj){
          // check if shader is already destroyed 
          if(shader_obj->id == INVALID_ID){
            return true;
          }
         if(!renderer_release_shader_resources(shader_obj,material_id)){
               DERROR("wrong shader id could not realse rsources of this shader: %s", shader_obj->name);
               return false;
         };
            material_shader_reference reference;
            hash_table_get(&state_ptr->material_shader_reference, shader_obj->name,&reference);
            reference.is_in_use = false;
            reference.reference_id = INVALID_ID;
            hash_table_set(&state_ptr->material_shader_reference,shader_obj->name,&reference);
            shader_obj->internal_data = nullptr;
            Dzero_memory(shader_obj,sizeof(material_shader));
            shader_obj->id =INVALID_ID;
            shader_obj->state = SHADER_STATE_NOT_CREATED; 
            return true;
  }
  else{
   DWARNING("destroying a non existent shader");
   return false;
  }
  return true;  
 }