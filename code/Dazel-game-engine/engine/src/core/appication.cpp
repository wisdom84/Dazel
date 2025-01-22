#include "application.h"
#include "platform/platform.h"
#include "logger.h"
#include "game_types.h"
#include "core/Dmemory.h"
#include "Dstrings.h"
#include "core/event_manager.h"
#include "containers/free_list.h"
#include "input.h"
#include "clock.h"
#include "Renderer/renderer_frontend.h"
#include "systems/texture_system.h"
#include "systems/shader_system.h"
#include "systems/material_system.h"
#include "systems/geometry_system.h"
#include "systems/resource_system.h"
#include "memory/linear_allocator.h"
#include <iostream>
#include "Math/dlm.h"
#include <stdio.h>
#include <stdlib.h>
#include <direct.h>
#include <limits.h>
#include "containers/Darray.h"
#include "Math/transform.h"
#include "Math/geometry_math_utils.h"
typedef struct application_state{
 linear_allocator system_allocators;  
 game*game_inst;
 clock_time clock;  
 bool is_running;
 bool is_suspended;
 platform_state platform;
 u32 width;
 u32 height;
 bool on_resize;
 double last_time;
 //TODO:: we will make use of our linear allocator to allocate memory for our other systems 

 //texture system state 
 u64 memory_requirements_texture_system;
 void*texture_system_state;

 // material system state
 u64 memory_requirements_material_system;
 void* material_system_state;

 // geometry system state
 u64 memory_requirements_geometry_system;
 void* geometry_system_state;

 geometry*test_geometry;
 geometry*test_ui_geometry;
 geometry*light_cube;
 mesh*meshes;

 mesh meshes_1[10];
 u32 mesh_count;
 // resource system state
 u64 memory_requirements_resource_system;
 void* resource_system_state;

 // shader system
 u64 memory_requirements_material_shader_system;
 void*shader_system_state;
}application_state;

static bool intialized = false;

static application_state app_state;
bool application_on_event(u16 code, void*sender, void*listener,event_context data);
bool application_on_key(u16 code, void*sender, void*listener,event_context data);
bool application_on_resize(u16 code, void*sender, void*listener, event_context data);
bool event_on_debug(u16 code, void*sender, void*listener, event_context data);




bool application_create(game*game_inst){
     if(intialized){
       DERROR("application create more than once");
       return false;  
     }
    u64 system_allocator_total_size = 64 * 1024 * 1024; //64 mb
    linear_allocator_create(system_allocator_total_size,0, &app_state.system_allocators);


     // initializing our sub system
     intialize_logging();
     input_intialize();
    DFATAL("A test message %f %d",3.14f);
    DWARNING("A test message %f",6.14f);
    DINFO("A test message %f",7.14f);
    DTRACE("A test message %f",8.14f);
    DDEBUG("A test message %f",9.14f);
    DERROR("A test message %f",8.19f);
    
 

  
    if(!event_intialize()){
      DFATAL("could not initialize the event systme application could not start up");
      return false;
    }
    app_state.game_inst = game_inst;

    app_state.is_running = true;

    app_state.is_suspended = false;

    event_register(EVENT_CODE_APPLICATION_QUIT, 0,application_on_event);
    event_register (EVENT_CODE_KEY_PRESSED, 0,application_on_key);
    event_register(EVENT_CODE_KEY_RELEASED,0,application_on_key);
    event_register(EVENT_CODE_RESIZED, 0,application_on_resize);
    event_register(EVENT_CODE_DEBUG0, 0,event_on_debug);

    
    if(!platform_start_up(&app_state.platform,
                        app_state.game_inst->app_config.application_name,
                        app_state.game_inst->app_config.x,
                        app_state.game_inst->app_config.y,
                        app_state.game_inst->app_config.height,
                        app_state.game_inst->app_config.width)){
                       return false;
    }
   // initialize our resource system
   resource_system_config resource_config;
   resource_config.max_loader_count = 32;
   const char*base_path = "C:\\Dazel_project\\Dazel\\code\\Dazel-game-engine\\bin";
   if(_chdir(base_path) != 0){
      DINFO("sorry could not change the directory path to the bin foleder");
   }
   resource_config.asset_base_path = "assets";
   resource_system_intialize(&app_state.memory_requirements_resource_system, 0, resource_config );
   app_state.resource_system_state = linear_allocator_allocate(&app_state.system_allocators,app_state.memory_requirements_resource_system);
   if(!resource_system_intialize(&app_state.memory_requirements_resource_system, app_state.resource_system_state,resource_config)){
    DFATAL("failed to initialize resource system can not continue application");
    return false;
   } 
    app_state.width = app_state.game_inst->app_config.width; 
    app_state.height = app_state.game_inst->app_config.height; 
    app_state.on_resize = false;
    if(!renderer_initialize(game_inst->app_config.application_name, &app_state.platform, app_state.game_inst->app_config.width,   app_state.game_inst->app_config.height)){
      DFATAL("could not startup the renderer application will shutdown");
      return false;
   }
   // intialize our texture system
   texture_system_config texture_system_config;
   texture_system_config.max_texture_count = 1024;
   texture_system_initialize(&app_state.memory_requirements_texture_system, 0, texture_system_config );
   app_state.texture_system_state = linear_allocator_allocate(&app_state.system_allocators,app_state.memory_requirements_texture_system);
   if(!texture_system_initialize(&app_state.memory_requirements_texture_system, app_state.texture_system_state,texture_system_config)){
    DFATAL("failed to initialize texture system can not continue application");
    return false;
   }
   // intialize our shader_system
   shader_system_config shader_system_config;
   shader_system_config.max_shaders = 1024;
   shader_system_initialize( shader_system_config,&app_state.memory_requirements_material_shader_system,0);
   app_state.shader_system_state = linear_allocator_allocate(&app_state.system_allocators,app_state.memory_requirements_material_shader_system);
   if(!shader_system_initialize(shader_system_config,&app_state.memory_requirements_material_shader_system,app_state.shader_system_state)){
    DFATAL("failed to initialize shader_system can not continue application");
    return false;
   }

   // intialize our material system
   material_system_config matrials_system_configuration;
   matrials_system_configuration.max_material_count = 1024;
   material_system_initialize(&app_state.memory_requirements_material_system, 0, matrials_system_configuration );
   app_state.material_system_state = linear_allocator_allocate(&app_state.system_allocators,app_state.memory_requirements_material_system);
   if(!material_system_initialize(&app_state.memory_requirements_material_system, app_state.material_system_state,matrials_system_configuration)){
    DFATAL("failed to initialize material system can not continue application");
    return false;
   }

   //initialize our geometry system
   geometry_system_config geometry_system_configuration;
   geometry_system_configuration.max_geometry_count = 1024;
   geometry_system_initialize(&app_state.memory_requirements_geometry_system, 0, geometry_system_configuration );
   app_state.geometry_system_state = linear_allocator_allocate(&app_state.system_allocators,app_state.memory_requirements_geometry_system);
   if(!geometry_system_initialize(&app_state.memory_requirements_geometry_system, app_state.geometry_system_state,geometry_system_configuration)){
    DFATAL("failed to initialize geometry system can not continue application");
    return false;
   }
   //temp code
  // geometry_config g_config =geometry_system_generate_cube_config(10.0f, 10.0f,10.0f,1.0f,1.0f,"test_cube","test_materials",true);
  // app_state.test_geometry=geometry_system_acquire_from_config(g_config,true);
  // // free allocated data on the heap
  // Dfree_memory(g_config.verticies,MEMORY_TAG_ARRAY,sizeof(Vertex_3d)*g_config.vertex_count);
  // Dfree_memory(g_config.indicies,MEMORY_TAG_ARRAY,sizeof(u32)*g_config.index_count);
//temp code 
//temp code 

// app_state.meshes = darray_create(mesh);
app_state.mesh_count = 0;
mesh*cube_mesh = &app_state.meshes_1[app_state.mesh_count];
cube_mesh->geometry_count = 1;
cube_mesh->geometries = (geometry**)Dallocate_memory(sizeof(geometry*)*cube_mesh->geometry_count, MEMORY_TAG_ARRAY);
geometry_config g_config_1 =geometry_system_generate_cube_config(10.0f, 10.0f,10.0f,1.0f,1.0f,"test_cube","test_materials",true);
cube_mesh->geometries[0] =geometry_system_acquire_from_config(g_config_1,true);
cube_mesh->transform = transform_create();
app_state.mesh_count++;
// darray_push(app_state.meshes,cube_mesh,mesh);
Dfree_memory(g_config_1.verticies,MEMORY_TAG_ARRAY,sizeof(Vertex_3d)*g_config_1.vertex_count);
Dfree_memory(g_config_1.indicies,MEMORY_TAG_ARRAY,sizeof(u32)*g_config_1.index_count);

mesh*cube_mesh_2 = &app_state.meshes_1[app_state.mesh_count];
cube_mesh_2->geometry_count = 1;
cube_mesh_2->geometries = (geometry**)Dallocate_memory(sizeof(geometry*)*cube_mesh_2->geometry_count, MEMORY_TAG_ARRAY);
geometry_config g_config_2 =geometry_system_generate_cube_config(5.0f, 5.0f,5.0f,1.0f,1.0f,"test_cube_2","test_materials",true);
cube_mesh_2->geometries[0] =geometry_system_acquire_from_config(g_config_2,true);
cube_mesh_2->transform = transform_from_position((vec3){10.0f,0.0f,1.0f});
transform_set_parent(&cube_mesh_2->transform,&cube_mesh->transform);
app_state.mesh_count++;
// darray_push(app_state.meshes,cube_mesh,mesh);
Dfree_memory(g_config_2.verticies,MEMORY_TAG_ARRAY,sizeof(Vertex_3d)*g_config_2.vertex_count);
Dfree_memory(g_config_2.indicies,MEMORY_TAG_ARRAY,sizeof(u32)*g_config_2.index_count);


mesh*cube_mesh_3 = &app_state.meshes_1[app_state.mesh_count];
cube_mesh_3->geometry_count = 1;
cube_mesh_3->geometries = (geometry**)Dallocate_memory(sizeof(geometry*)*cube_mesh_3->geometry_count, MEMORY_TAG_ARRAY);
geometry_config g_config_3 =geometry_system_generate_cube_config(2.0f, 2.0f,2.0f,1.0f,1.0f,"test_cube_3","test_materials",true);
cube_mesh_3->geometries[0] =geometry_system_acquire_from_config(g_config_3,true);
cube_mesh_3->transform = transform_from_position((vec3){5.0f,0.0f,1.0f});
transform_set_parent(&cube_mesh_3->transform,&cube_mesh_2->transform);
app_state.mesh_count++;
// darray_push(app_state.meshes,cube_mesh,mesh);
Dfree_memory(g_config_3.verticies,MEMORY_TAG_ARRAY,sizeof(Vertex_3d)*g_config_3.vertex_count);
Dfree_memory(g_config_3.indicies,MEMORY_TAG_ARRAY,sizeof(u32)*g_config_3.index_count);

//temp code 
mesh*car_mesh = &app_state.meshes_1[app_state.mesh_count];
resource car_mesh_resource;
if(!resource_system_load("falcon",RESOURCE_TYPE_STATIC_MESH,&car_mesh_resource)){
  DERROR("failed to load car mesh resource");
}
else{
  geometry_config * configs = (geometry_config*)car_mesh_resource.data;
  car_mesh->geometry_count =car_mesh_resource.size_of_data;
  car_mesh->geometries = (geometry**)Dallocate_memory(sizeof(geometry*)*car_mesh->geometry_count,MEMORY_TAG_ARRAY);
  for(u32 i =0; i < car_mesh->geometry_count; i++){
    geometry_config*c = &configs[i];
    geometry_generate_tangent(c->vertex_count, (Vertex_3d*)c->verticies, c->index_count, (u32*)c->indicies);
    car_mesh->geometries[i] = geometry_system_acquire_from_config(configs[i],true);
  }
  // car_mesh->transform = transform_from_position((vec3){15.0f, 0.0f,1.0f});
  car_mesh->transform = transform_from_position_rotation((vec3){15.0f,0.0f,1.0f},mat4_homogeneous_rotation("x",180.0f));
  resource_system_unload(&car_mesh_resource);
  app_state.mesh_count++;
}


mesh*sponza_mesh = &app_state.meshes_1[app_state.mesh_count];
resource sponza_mesh_resource;
if(!resource_system_load("sponza",RESOURCE_TYPE_STATIC_MESH,&sponza_mesh_resource)){
  DERROR("failed to load sponza  mesh resource");
}
else{
  geometry_config * configs = (geometry_config*)sponza_mesh_resource.data;
  sponza_mesh->geometry_count =sponza_mesh_resource.size_of_data;
  sponza_mesh->geometries = (geometry**)Dallocate_memory(sizeof(geometry*)*sponza_mesh->geometry_count,MEMORY_TAG_ARRAY);
  for(u32 i =0; i < sponza_mesh->geometry_count; i++){
    geometry_config*c = &configs[i];
    geometry_generate_tangent(c->vertex_count, (Vertex_3d*)c->verticies, c->index_count, (u32*)c->indicies);
    sponza_mesh->geometries[i] = geometry_system_acquire_from_config(configs[i],true);
  }
  // car_mesh->transform =
  sponza_mesh->transform = transform_from_position_rotation_scale(vec3_zero(),mat4_homogeneous_rotation_xyz("yxz",90.0f,90.0f,0.0f) ,vec3_create(0.05f,0.05f,0.05f));
  resource_system_unload(&sponza_mesh_resource);
  app_state.mesh_count++;
}







  // test ui geometry
  geometry_config ui_config;
  ui_config.apply_material = true;
  ui_config.vertex_size = sizeof(Vertex_2d);
  ui_config.vertex_count = 4;
  ui_config.index_size = sizeof(u32);
  ui_config.index_count = 6;

  string_n_copy(ui_config.name, "test_ui_geometry",GEOMETRY_NAME_MAX_LENGTH);
  string_n_copy(ui_config.material_name, "test_ui_materials", MATERIAL_NAME_MAX_LENGTH);

  const float f = 512.0f;
  Vertex_2d uiverts[4];
  uiverts[0].position.x = 0.0f;
  uiverts[0].position.y = 0.0f;
  uiverts[0].texture_cord.x = 0.0f;
  uiverts[0].texture_cord.y = 0.0f;

  uiverts[1].position.x = f * 0.00136f;
  uiverts[1].position.y = f * 0.00136f;
  uiverts[1].texture_cord.x = 1.0f;
  uiverts[1].texture_cord.y = 1.0f;

  uiverts[2].position.x = 0.0f;
  uiverts[2].position.y = f * 0.00136f;
  uiverts[2].texture_cord.x = 0.0f;
  uiverts[2].texture_cord.y = 1.0f;

  uiverts[3].position.x = f * 0.00136f;
  uiverts[3].position.y = 0.0f;
  uiverts[3].texture_cord.x = 1.0f;
  uiverts[3].texture_cord.y = 0.0f;

  ui_config.verticies = uiverts;

  // indicies 
  u32 ui_indicies[6] = {2, 1, 0, 3, 0, 1};
  ui_config.indicies = ui_indicies;
  // //test_ui_geometry

  // app_state.test_ui_geometry = geometry_system_acquire_from_config(ui_config,true);
  //  // temp code


    if(!app_state.game_inst->initialize(app_state.game_inst)){
        DFATAL("failed to intialize the game instance ");
        return false;
    }  

   app_state.game_inst->on_resize(app_state.game_inst, app_state.width,app_state.height);    
   intialized = true; 
   return true;
 };
float angle= 20.0f;
 bool application_run(){
    clock_start(&app_state.clock);
    clock_update(&app_state.clock);
    app_state.last_time = app_state.clock.elapsed_time;
    double running_time = 0.0f;
    u64 frame_count = 0;
    double target_frame_seconds = 1.0f/60;
    DINFO(get_memory_usage());
    while(app_state.is_running){
          
           if(!platform_pump_messages(&app_state.platform)){
             app_state.is_running=false;
           };
           if(!app_state.is_suspended){
               clock_update(&app_state.clock);
               double current_elapsed_time = app_state.clock.elapsed_time;
               double delta_time = (current_elapsed_time - app_state.last_time);
               double frame_start_time = get_absolute_time();
                if(!app_state.game_inst->update(app_state.game_inst, (float)delta_time)){
                    DFATAL("Game update failed shuting down");
                    app_state.is_running=false;
                    break;
                };
                if(!app_state.game_inst->render(app_state.game_inst, (float)delta_time)){
                    DFATAL("Game render failed shuting down");
                    app_state.is_running=false;
                    break;
                };
                // TODO: temp code 
                //  angle += 10.0f*delta_time; 
                //  geometry_render_data test_render[1];
                //  test_render[0].geo_obj = app_state.test_geometry;
                //  // mat4_homogeneous_rotation("x", 40.0f) * mat4_homogeneous_rotation("y",angle)* mat4_transponse(mat4_homogeneous_translation(-4.0f,0.0f,0.0f));
                //  test_render[0].model = mat4_identity()* mat4_homogeneous_rotation("y", angle); 
                //  geometry_render_data test_ui_render;
                //  test_ui_render.model = mat4_homogeneous_translation(0.0f,0.0f,0.0f);
                //  test_ui_render.geo_obj = app_state.test_ui_geometry;

                 render_packet packet = {};
                 packet.delta_time = delta_time;
                 packet.geometry_count = 0;
                //  u32 mesh_count = darray_get_length(app_state.meshes);
                 packet.geometries = darray_create(geometry_render_data);
                 if (app_state.mesh_count > 0){
                          mat4 rotation_matrix = mat4_homogeneous_rotation("y", angle* delta_time);
                          transform_rotate(&app_state.meshes_1[0].transform,rotation_matrix);
                    
                          if(app_state.mesh_count > 1){

                             transform_rotate(&app_state.meshes_1[1].transform,rotation_matrix);
                          }
                          if(app_state.mesh_count > 2){
                             transform_rotate(&app_state.meshes_1[2].transform,rotation_matrix);
                          }
                          // if(app_state.mesh_count > 3){
                          //      transform_rotate(&app_state.meshes_1[3].transform,rotation_matrix);
                          // }
                          for(u32 i = 0; i < app_state.mesh_count; i++){
                              mesh*m = &app_state.meshes_1[i];
                              for(u32 j = 0; j < m->geometry_count; j++){
                                geometry_render_data data;
                                data.geo_obj = m->geometries[j];
                                //app_state.meshes[i].model;
                                data.model = transform_get_world(&m->transform);
                                darray_push(packet.geometries,data,geometry_render_data);
                                packet.geometry_count++;  
                              }
                          }
                          // packet.geometry_count = darray_get_length(packet.geometries);
                 }
                //  else{
                //   packet.geometry_count = 0;
                //   packet.geometries = 0;
                //  }
                //  packet.geometry_count = 1;
                //  packet.geometries = test_render;
                //  packet.UI_geometry_count = 1;
                //  packet.UI_geometries = &test_ui_render;

                // TODO: end temp code
                  if(app_state.on_resize){
                      renderer_on_resized(app_state.width,app_state.height);
                      app_state.on_resize = false;
                  }
                renderer_draw_frame(&packet);

                if(packet.geometries){
                  darray_destroy_array(packet.geometries);
                  packet.geometries = 0;
                }
                // find how long it took for our frame to be displayed
                double frame_end_time =get_absolute_time();
                double frame_elapsed_time = frame_end_time - frame_start_time;
                running_time = running_time + frame_elapsed_time;
                double remaining_seconds = target_frame_seconds-frame_elapsed_time;
                if(remaining_seconds > 0){
                  u64 remaining_seconds_ms = (remaining_seconds * 1000);
                  bool limit_frames = false;
                  if(remaining_seconds_ms >0 && limit_frames){
                    platform_sleep(remaining_seconds_ms -1);
                  }
                 frame_count=frame_count+1;
                }
                input_update(delta_time);
                //update last time 
                app_state.last_time= current_elapsed_time;
           }
        }
    app_state.is_running=false;

    event_unregister(EVENT_CODE_APPLICATION_QUIT, 0,application_on_event);
    event_unregister (EVENT_CODE_KEY_PRESSED, 0,application_on_key);
    event_unregister(EVENT_CODE_KEY_RELEASED,0,application_on_key);
    event_unregister(EVENT_CODE_RESIZED,0,application_on_resize);
    event_unregister(EVENT_CODE_DEBUG0, 0,event_on_debug);

    geometry_system_shutdown(app_state.geometry_system_state);
    material_system_shutdown(app_state.material_system_state);
    shader_system_shutdown((shader_system_state*)app_state.shader_system_state);
    texture_system_shutdown(app_state.texture_system_state);
    renderer_shutdown();
    event_shutdown();
    input_shutdown();
    resource_system_shutdown(app_state.resource_system_state);
    platform_shut_down(&app_state.platform);
  return true;
 };
 // event handlers

 void application_get_framebuffer(u64*width, u64*height){
  *width = app_state.width;
  *height = app_state.height;
 };

 bool application_on_event(u16 code, void*sender, void*listener,event_context data){
  switch(code){
     case EVENT_CODE_APPLICATION_QUIT:
          DINFO("Application was prompted to quit");
          app_state.is_running = false;
          return true;
  }
  return false;
 }


 bool application_on_resize(u16 code, void*sender, void*listener, event_context data){
  DINFO("in application on resize");
     if(code == EVENT_CODE_RESIZED){
        u16 width = (u16)data.u32_byte[0];
        u16 height =(u16)data.u32_byte[1];

        //check if window is minimized 
        if(width == 0 || height == 0){
          DINFO("window is minimized suspending application");
          app_state.is_suspended = true;
          return true;
        }else{
          if(app_state.is_suspended){
            DINFO("window restored, resuming application");
            app_state.is_suspended = false;
           
          }
          app_state.game_inst->on_resize(app_state.game_inst,width,height);
          app_state.on_resize = true;
          app_state.width  = (u32)width;
          app_state.height = (u32)height;
          // renderer_on_resized(width, height);
        }
     }
     return true;
 }
static int choice = 1;
static int choice_2 = 0;
bool event_on_debug(u16 code, void*sender, void*listener, event_context data){
      const char*names[3]{
           "test_materials",
           "orange_lines_materials",
           "paving2_materials"    
      };
      // const char*spec_names[3]{
      //   "orange_lines_512_SPEC",
      //   "cobblestone_SPEC",
      //   "paving2_SPEC"
      // };
      const char*old_name = names[choice];
      choice++;
      choice %= 3;
    //   const char*old_name_spec = spec_names[choice_2];
    //   choice_2++;
    //   choice_2 %=3;
    if(app_state.meshes_1){
      app_state.meshes_1[0].geometries[0]->material= material_system_acquire(names[choice],true);
      if(!app_state.meshes_1[0].geometries[0]->material){
          DWARNING("event on debug no material falling to default material");
      }
    }
    // if(app_state.test_geometry){
    //     app_state.test_geometry->material->diffuse_map.texture = texture_system_acquire(names[choice],true);
    //     if(!app_state.test_geometry->material->diffuse_map.texture){
    //       DWARNING("event on debug no texture falling to default texture");
    //       app_state.test_geometry->material->diffuse_map.texture = texture_system_get_defualt_texture();
    //     }
    //     // texture_system_release(old_name);
    //   }

    //  if(app_state.test_geometry){
    //     app_state.test_geometry->material->specular_map.texture = texture_system_acquire(spec_names[choice_2],true);
    //     if(!app_state.test_geometry->material->specular_map.texture){
    //       DWARNING("event on debug no texture falling to default texture");
    //       app_state.test_geometry->material->specular_map.texture = texture_system_get_defualt_texture();
    //     }
    //     // texture_system_release(old_name);
    //   }

    //   if(app_state.test_geometry){
    //     app_state.test_geometry->material->normal_map.texture = texture_system_acquire("normal",true);
    //     if(!app_state.test_geometry->material->normal_map.texture){
    //       DWARNING("event on debug no texture falling to default texture");
    //       app_state.test_geometry->material->normal_map.texture = texture_system_get_defualt_texture();
    //     }
    //     // texture_system_release(old_name);
    //   }

   return true;
 };

bool application_on_key(u16 code, void*sender, void*listener,event_context data){
    if(code == EVENT_CODE_KEY_PRESSED){
      u16 key_code = data.u16_bytes[0];
      if(key_code == KEY_ESC){
        event_context data {};
        event_fire(EVENT_CODE_APPLICATION_QUIT,0,data);
        return true;
      }
      else if(key_code == KEY_C){
        DDEBUG(" Explicit C key pressed");
      }
      else if(key_code == KEY_T){
        event_context context_data;
        event_fire(EVENT_CODE_DEBUG0,0,context_data);
      }
      else{
        DDEBUG("'%C' key pressed  in window ", key_code);
      }
    }
    else if(code == EVENT_CODE_KEY_RELEASED){
      u16 key_code = data.u16_bytes[0];
      if(key_code == KEY_B){
            DDEBUG(" Explicit B key released");
      }
      else{
         DDEBUG("'%C' key released  in window ", key_code);
      }
    }
    return false;
 }


 