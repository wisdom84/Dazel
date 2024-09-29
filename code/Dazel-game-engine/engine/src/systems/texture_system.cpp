#include "texture_system.h"
#include "core/Dmemory.h"
#include "core/Dstrings.h"
#include "core/logger.h"
#include "containers/hashtable.h"
#include "Renderer/renderer_frontend.h"
#include "systems/resource_system.h"
typedef struct texture_system_state{
  texture_system_config config;

  Texture default_texture;

  //Array of registered textures
  Texture*registered_texture;

  //hash table for registered look ups

  hashtable registered_texture_table;

}texture_system_state;

typedef struct texture_reference{
   u64 reference_count;
   u32 handle;
   bool auto_release;
}texture_reference;

static texture_system_state * state_ptr = 0;


bool create_default_texture(texture_system_state*state);
void destroy_default_textures(texture_system_state*state);
bool load_texture(const char*texture_name, Texture*t);

void destroy_textures(Texture*texture);


bool texture_system_initialize(u64*memory_requirements,void*state,texture_system_config config){
   if(config.max_texture_count == 0){
    DFATAL("texture_system_initialize - config.max_texture_count must be > 0");
    return false;
   }
   u64  struct_requirement  = sizeof(texture_system_state);
   u64 array_requirement = sizeof(Texture)*config.max_texture_count;
   u64 hashtable_requirement = sizeof(texture_reference)*config.max_texture_count;
   *memory_requirements = struct_requirement + array_requirement + hashtable_requirement;

   if(!state){
     return true;
   }
   state_ptr = (texture_system_state*)state;
   state_ptr->config = config;

   // array block 
   void* array_block = (char*)state + struct_requirement;
   state_ptr->registered_texture =(Texture*) array_block;

   // hash table block
   void*hashtable_block = (char*)array_block + array_requirement;
   hash_table_create(sizeof(texture_reference), config.max_texture_count, hashtable_block, false,&state_ptr->registered_texture_table);

   // fill the hashtable with invalid id references 
   texture_reference invalid_ref;
   invalid_ref.auto_release = false;
   invalid_ref.handle = INVALID_ID;
   invalid_ref.reference_count = 0;
   hash_table_fill(&state_ptr->registered_texture_table, &invalid_ref);

   // invalidate all texture in the array
   u32 count = state_ptr->config.max_texture_count;
   for(u32 i = 0; i <count; i++){
    state_ptr->registered_texture[i].id = INVALID_ID;
    state_ptr->registered_texture[i].generation = INVALID_ID;
   }
  // set up defualt textures
   create_default_texture(state_ptr);
   return true;
};

void texture_system_shutdown(void*state){
   if(state_ptr){
    //Destroy all loaded textures
      for(u32 i=0; i < state_ptr->config.max_texture_count; i++){
        Texture*t = &state_ptr->registered_texture[i];
        if(t->generation != INVALID_ID){
            renderer_destroy_texture(t);
        }
      }
      destroy_default_textures(state_ptr);
      state_ptr= 0;
   }
};

Texture*texture_system_acquire(const char*name, bool auto_release){
   if(string_equal(name,DEFAULT_TEXTURE_NAME)){
    DWARNING("texture_system_acquire called for default texutre. use get_default_texture for texture 'default'");
    return &state_ptr->default_texture;
   }
   texture_reference ref;
   if(state_ptr && hash_table_get(&state_ptr->registered_texture_table, name,&ref)){
        if(ref.reference_count == 0){
            ref.auto_release = auto_release;
        }
        ref.reference_count++;
        if(ref.handle == INVALID_ID){
            u32 count =state_ptr->config.max_texture_count;
            Texture*t=0;
            for(u32 i = 0;  i < count; i++){
                if(state_ptr->registered_texture[i].id == INVALID_ID){
                    ref.handle = i;
                    t = &state_ptr->registered_texture[i];
                    string_n_copy(t->name,name, TEXTURE_NAME_MAX_LENGTH);
                    break;
                }
            }
            if(!t || ref.handle == INVALID_ID){
                DFATAL("texture_system_acquire - Texture system can not hold anymore textures adjust the configuration file");
                return nullptr;
            }
            //create a new texture
            if(!load_texture(name,t)){
                DERROR("failed to load texture '%s'", name);
                return nullptr;
            }
            t->id = ref.handle;
            DTRACE("texture '%s' does not exist. created and ref_count is now %i.", name , ref.reference_count);
        }
        else{
            DTRACE("Texture '%s' already exists, ref_count increased to %i.", name , ref.reference_count);
        }
        hash_table_set(&state_ptr->registered_texture_table, name,&ref);
        return  &state_ptr->registered_texture[ref.handle];
   }
   DERROR("texture_system_acquire failed to aquire texture '%s'. nullptr will be returned.", name);
   return nullptr;
};


void texture_system_release(const char*name){

   if(string_equal(name, DEFAULT_TEXTURE_NAME)){
      return;
   }

   texture_reference ref;

   if(hash_table_get(&state_ptr->registered_texture_table,name,&ref)){
        if(ref.handle == INVALID_ID){
            DWARNING("Tried to release a non-existent texture: %s", name);
            return;
        }
        // temporary copy the name of the texture 
        char dst_name_copy[512];
       Texture*temp = &state_ptr->registered_texture[ref.handle];
        if((const char *)temp->name){
                string_n_copy(dst_name_copy, temp->name, TEXTURE_NAME_MAX_LENGTH);
                temp=0;
         }
        ref.reference_count--;
      if(ref.reference_count == 0 &&  ref.auto_release){
            Texture*t = &state_ptr->registered_texture[ref.handle];
            renderer_destroy_texture(t);
            Dzero_memory(t, sizeof(Texture));
            t->id = INVALID_ID;
            t->generation = INVALID_ID;

            //Reset reference
            ref.handle = INVALID_ID;
            ref.auto_release = false;
            DTRACE("Released texture '%s' Texture unloaded because reference count = 0 and auto_release is true.", dst_name_copy);
        }
        else{
                  if(ref.auto_release){
                                    DTRACE("Released texture '%s'. now has a refernce count of '%i'auto_release is true.", dst_name_copy ,ref.reference_count);
                  }
                  else{
                                 DTRACE("Released texture '%s'. now has a refernce count of '%i'auto_release is false.", dst_name_copy ,ref.reference_count);
                  }
        }

        // update the  hashtable 
        hash_table_set(&state_ptr->registered_texture_table, name,&ref);
       return;
   }
  DERROR("texture_system_release failed to release texture '%s'.", name); 
};

Texture*texture_system_get_defualt_texture(){
    if(state_ptr){
        return &state_ptr->default_texture;
    }
    DERROR("get_default_texture called before texture system initialization! NULL pointer returned");
    return nullptr;
};


bool create_default_texture(texture_system_state*state){
      DTRACE("creating default texture");
      const u32 tex_dimension = 256;
      const u32 channel = 4;
      const u32 pixel_count = tex_dimension * tex_dimension;
      u8 pixels[pixel_count*channel];
      Dmemory_set(sizeof(u8)*pixel_count*channel,pixels,255);
      //Each pixel
      for(u64 row = 0; row <tex_dimension; row++){
         for(u64 column=0; column < tex_dimension; column++){
            u64 index = (row *tex_dimension)+column;
            u64 index_bpp = index *channel;
            if(row%2){
               if(column%2){
                  pixels[index_bpp + 0] = 0;
                  pixels[index_bpp + 1] = 0;
               }
            }else{
               if(!(column%2)){
                  pixels[index_bpp + 0] = 0;
                  pixels[index_bpp + 1] = 0;
               }
            }
         }
      }
      renderer_create_texture("default",
      tex_dimension,
      tex_dimension,
      4,
      pixels,
      false,
      &state_ptr->default_texture);
      state_ptr->default_texture.generation =INVALID_ID;
      string_n_copy(state_ptr->default_texture.name, DEFAULT_TEXTURE_NAME, TEXTURE_NAME_MAX_LENGTH); 
    return true;
}

bool load_texture(const char*texture_name, Texture*t){
   resource image;
   Dzero_memory(&image, sizeof(resource));
   if(!resource_system_load(texture_name, RESOURCE_TYPE_IMAGE,&image)){
      DERROR("failed to load image resource");
      return false;
   }
   image_resource_data*resource_data = (image_resource_data*)image.data;
   Texture temp_texture;
   temp_texture.width = resource_data->width;
   temp_texture.height = resource_data->height;
   temp_texture.channel_count = resource_data->channel_count;

   u32 current_generation = t->generation;
   t->generation = INVALID_ID;
   u64 total_size = temp_texture.width * temp_texture.height * temp_texture.channel_count;
   bool has_transparency = false;
   for(u64 i=0; i <total_size; i+=temp_texture.channel_count){
      u8 a = resource_data->pixels[i + 3];
      if(a < 255){
         has_transparency = true;
         break;
      }
   }
   temp_texture.has_transparency= has_transparency;
   renderer_create_texture( texture_name, temp_texture.width,temp_texture.height, temp_texture.channel_count,resource_data->pixels,has_transparency,&temp_texture);
   Texture old = *t;
   *t = temp_texture;
   renderer_destroy_texture(&old);
   if(current_generation == INVALID_ID){
        t->generation = 0;
    }else{
       t->generation = current_generation + 1;
     }
   resource_system_unload(&image);  
   return true;  
}


void destroy_default_textures(texture_system_state*state){
    if(state){
        renderer_destroy_texture(&state->default_texture);
    }
}

void destroy_textures(Texture*texture){
   if(texture){
      renderer_destroy_texture(texture);
      texture->generation = INVALID_ID;
      texture->id = INVALID_ID;
      Dzero_memory(texture, sizeof(Texture));
   }
}