#include "renderer_frontend.h"
#include "renderer_backend.h"
#include  "Math/dlm.h"
#include "core/event_manager.h"
using namespace dlm;
#include "core/Dmemory.h"
#include "core/logger.h"
#include "core/Dstrings.h"
#include "systems/texture_system.h"
#include "systems/material_system.h"
#include "systems/shader_system.h"



typedef struct renderer_system_state{
   mat4 projection;
   mat4 view;
   vec3 camera_position;
   vec4 ambient_color;
   mat4 UI_projection;
   mat4 UI_view;
   float near_clip;
   float far_clip;
   bool active[MAX_SHADER_INSTANCE];
}renderer_system_state;

renderer_backend*backend=0;

renderer_system_state*state_ptr;

bool renderer_initialize(const char*application_name, struct platform_state*plat_state, int width, int height){
     backend = (renderer_backend*)Dallocate_memory(sizeof(renderer_backend), MEMORY_TAG_RENDERER);
     state_ptr =(renderer_system_state*)Dallocate_memory(sizeof(renderer_system_state), MEMORY_TAG_RENDERER);
     // allocating memory on the heap for the renderer
     create_renderer_backend(RENDERER_BACKEND_TYPE_VULKAN, plat_state, backend);
     if(!backend->initialize(backend,application_name,plat_state,  width,  height)){
        DFATAL("Renderer backend failed to intialize. shuting down the application");
        return false;
     }
      backend->frame_count=0;
      // world shader projection and view  
      state_ptr->projection =mat4_perspective(45.0f,900.f/600.f,0.1f,1000.f);
      state_ptr->far_clip =0.1f;
      state_ptr->ambient_color = vec4_create(0.25f, 0.25f,0.25f,1.0f);
      state_ptr->near_clip = 1000.0f;
      state_ptr->view =mat4_homogeneous_translation(0.0f,0.0f,4.0f);
      state_ptr->view= mat4_inverse(state_ptr->view);

      // UI shader projection and view 
    	// state_ptr->UI_projection = mat4_orthographic_proj(1.0f,-1.0f,1.0f,-1.0f,0.1f,100.0f);
      state_ptr->UI_projection = mat4_orthographic_proj(0.0f,(float)width,  (float)height, 0.0f, -100.0f, 100.0f);
      state_ptr->UI_view = dlm::mat4_inverse(dlm::mat4_identity());

      // ivalidate all active shaders 
      for(u32 i=0; i<MAX_SHADER_INSTANCE; i++){
        state_ptr->active[i] = false;
      }
     return true;
}

void renderer_shutdown(){
   backend->shutdown(backend);
   Dfree_memory(backend,MEMORY_TAG_RENDERER,sizeof(renderer_backend));
   renderer_backend_destroy(backend);
};

void renderer_on_resized(u16 width, u16 height){
   state_ptr->projection =mat4_perspective(45.0f,(float)width/(float)height,state_ptr->near_clip, state_ptr->far_clip);
   state_ptr->UI_projection = mat4_orthographic_proj(0.0f,(float)width,  (float)height, 0.0f, -100.0f, 100.0f);
   backend->resized(backend, width, height);
}

bool renderer_draw_frame(render_packet*packet){

   if(backend->begin_frame(backend,packet->delta_time)){
          // begin world renderpass
         if(!backend->begin_renderpass(backend,BUILTIN_RENDERPASS_WORLD)){
            DERROR("backend begin render pass for the world failed");
            return false;
         } 
        for(u32 i=0; i < MAX_SHADER_INSTANCE; i++){
             if(state_ptr->active[i]){
                  backend->update_global_state(state_ptr->projection,state_ptr->view, state_ptr->camera_position, state_ptr->ambient_color,0,packet->delta_time,i);
             }   
        }


         u32 count = packet->geometry_count;
         // for(u32 i = 0; i < count; i++){
         //    backend->draw_geometry(packet->geometries[i]);
         // }
         backend->draw_geometry(packet->geometries[0]);
     

         if(!backend->end_renderpass(backend,BUILTIN_RENDERPASS_WORLD)){
            DERROR("backend end render pass for the world failed");
            return false;
         }
         // begin UI renderpass
      //   if(!backend->begin_renderpass(backend,BUILTIN_RENDERPASS_UI)){
      //       DERROR("backend begin render pass for the UI failed");
      //       return false;
      //    } 

      //    backend->update_UI_global_state(state_ptr->UI_projection,state_ptr->UI_view,0,packet->delta_time);

      //    count = packet->UI_geometry_count;
      //    for(u32 i = 0; i < count; i++){
      //       backend->draw_geometry(packet->UI_geometries[i]);
      //    }

      //    if(!backend->end_renderpass(backend,BUILTIN_RENDERPASS_UI)){
      //       DERROR("backend end render pass for the UI failed");
      //       return false;
      //    }
         //end UI renderpass

         bool result = backend->end_frame(backend,packet->delta_time);
         if(!result){
            DERROR("renderer end frame failed the application is shuting down");
            return false;
         }
         backend->frame_count=backend->frame_count+1;
   };
  return true; 
}


void renderer_create_texture( const char*name, int width, int height, int channel_count, const u8*pixels, bool has_transparency, struct Texture*out_texture){
    backend->create_texture(name,width,height,channel_count,pixels,has_transparency,out_texture);
}

void renderer_destroy_texture(struct Texture*texture){
    backend->destroy_texture(texture);
}; 
bool renderer_create_material_shader(struct material_shader*shader){
    return backend->shader_create(shader);
}
bool renderer_acquire_shader_resources(struct material_shader*shader, u32*material_id){
     return backend->acquire_shader_resources(shader, material_id);  
};
bool renderer_release_shader_resources(struct material_shader*shader, u32 material_id){
    return backend->release_shader_resources(shader, material_id);   
};
bool renderer_create_geometry(geometry*geo_obj,u32 vertex_count,u32 vertex_size , const void*verticies, u32 index_size,u32 index_count, const void*indicies){
  return backend->create_geometry(geo_obj,vertex_count, vertex_size,verticies,index_size,index_count,indicies);
   
};

void draw_geometry(geometry_render_data data){
   backend->draw_geometry(data);
};

void renderer_destroy_geometry(geometry*geometry){
    backend->destroy_geometry(geometry);
};

void renderer_set_view(mat4 view, vec3 camera_position){
   state_ptr->view = view;
   state_ptr->camera_position = camera_position;
}
void renderer_activate_shader(u32 id, bool is_active){
  state_ptr->active[id]= is_active;
};



  