#include "resource_system.h"
#include "core/Dstrings.h"
#include "core/Dmemory.h"
#include "core/logger.h"
//known loaders 
#include "resources/loader/image_loader.h"
#include "resources/loader/material_loader.h"
#include "resources/loader/binary_loader.h"
#include "resources/loader/text_loader.h"
#include "resources/loader/shader_config_loader.h"
#include "resources/loader/mesh_loader.h"

typedef struct resource_system_state{
    resource_system_config config;
    resource_loader*registered_loader;
}resource_system_state;

static resource_system_state*state_ptr = 0;

bool load(const char* name, resource_loader*loader, resource*out_resource);


bool resource_system_intialize(u64*memory_requirement,void*state, resource_system_config config){
      if(config.max_loader_count == 0){
        DFATAL("resource system initialize config max loader count must be > 0");
        return false;
    }

    u64 struct_requirement = sizeof(resource_system_state);
    u64 resource_loader_array_requirement = sizeof(resource_loader)*config.max_loader_count;
    *memory_requirement = struct_requirement + resource_loader_array_requirement;

    if(!state){
        return true;
    }
    state_ptr = (resource_system_state*)state;
    state_ptr->config = config;
    void*array_block = (char*)state + struct_requirement;
    state_ptr->registered_loader = (resource_loader*)array_block;

    // invalidate all loaders
    u32 count = config.max_loader_count;
    for(u32 i = 0; i < count; i++){
        state_ptr->registered_loader[i].id = INVALID_ID;
    }
    // register known loaders 
    resource_system_register_loader(material_loader_create());
    resource_system_register_loader(image_resource_loader_create());
    resource_system_register_loader(binary_resource_loader_create());
    resource_system_register_loader(text_resource_loader_create());
    resource_system_register_loader(shader_config_loader_create());
    resource_system_register_loader(mesh_resource_loader_create());
    DINFO("Resource system initialized with base path '%s'", config.asset_base_path);

    return true;
};
void resource_system_shutdown(void*state){
    if(state_ptr){
        state_ptr = 0;
    }
};

bool resource_system_register_loader(resource_loader loader){
   if(state_ptr){
        u32 count = state_ptr->config.max_loader_count;
        // checking if the loader already exists
        for(u32 i= 0; i < count; i++){
            resource_loader* load = &state_ptr->registered_loader[i];
            if(load->id != INVALID_ID){
                 if(load->type == loader.type){
                     DERROR("resource_system_register_loader - Loader of type %d already exists and will not be registered", loader.type);
                     return false;
                 }else if(loader.custom_type && string_length(loader.custom_type) > 0 && string_equal(load->custom_type, loader.custom_type)){
                    DERROR("resource_system_register_loader - Loader of  custom type %d already exists and will not be registered", loader.custom_type);
                    return false;
                 }
            }
        }
        for(u32 i = 0; i < count; i++){
            if(state_ptr->registered_loader[i].id == INVALID_ID){
                state_ptr->registered_loader[i] = loader;
                state_ptr->registered_loader[i].id = i;
                DTRACE("loader registered successfully");
                return true;
            }
        }
   }
   return false;
};
 bool resource_system_load(const char*name, resource_types type, resource*out_resource){
    if(state_ptr && type != RESOURCE_TYPE_CUSTOM){
        u32 count = state_ptr->config.max_loader_count;
        for(u32 i = 0; i < count; i++){
            resource_loader*l = &state_ptr->registered_loader[i];
            if(l->id != INVALID_ID && l->type == type){
                return load(name,l,out_resource);
            }
        }
    }
    out_resource->loader_id = INVALID_ID;
    DERROR("tried to load resource with invalid type or tried loading resources before resource_system_intialization");
    return false;
 };

 bool resource_system_load_custom(const char* name, const char* custom_type, resource*out_resource){
     if(state_ptr && custom_type && string_length(custom_type)>0){
        u32 count = state_ptr->config.max_loader_count;
        for(u32 i = 0; i < count; i++){
            resource_loader*l = &state_ptr->registered_loader[i];
            if(l->id != INVALID_ID && l->type ==RESOURCE_TYPE_CUSTOM && string_equal(custom_type, l->custom_type)){
                void*test;
                return load(name,l,out_resource);
            }
        }
    }
    out_resource->loader_id = INVALID_ID;
    DERROR("tried to load resource with custom invalid type or tried loading resources before resource_system_intialization");
    return false;
 };

 void resource_system_unload(resource*resource){
       if(state_ptr && resource){
         if(resource->loader_id != INVALID_ID){
            resource_loader*l = &state_ptr->registered_loader[resource->loader_id];
            if(l->id != INVALID_ID && l->unload){
                l->unload(l, resource);
            }
         }
       }
 };

 const char* resource_system_base_path(){
    if(state_ptr){
        return state_ptr->config.asset_base_path;
    }
    DERROR("resource_system_base_path called before initialization of the resource system returing an empty string");
    return "";
 };

 
bool load(const char* name, resource_loader*loader, resource*out_resource){
     if(!name || !loader || !loader->load || !out_resource){
        out_resource->loader_id = INVALID_ID;
        return false;
     }
     out_resource->loader_id = loader->id;
     return loader->load(loader,name,out_resource);
};

