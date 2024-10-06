#include "geometry_system.h"
#include "Math/geometry_math_utils.h"
#include "core/logger.h"
#include "core/Dmemory.h"
#include "core/Dstrings.h"
#include "systems/material_system.h"
#include "Renderer/renderer_frontend.h"

typedef struct geometry_reference{
    u64 reference_count;
    geometry geometry;
    bool auto_release;
}geometry_reference;

typedef struct geometry_system_state{
  geometry_system_config config;
  geometry default_geometry;
  geometry default_geometry_2d;
  geometry_reference*registered_geometries;
}geometry_system_state;

static geometry_system_state*state_ptr = 0;

bool create_default_geometry(geometry_system_state*state);
bool create_geometry(geometry_system_state*state,geometry_config config, geometry*out_geometry);
void destroy_geometry(geometry_system_state* state, geometry*geometry);

bool geometry_system_initialize(u64*memory_requirement, void*state, geometry_system_config config){
    if(config.max_geometry_count == 0){
        DFATAL("geometry system initialize config max geometry count must be > 0");
        return false;
    }

    u64 struct_requirement = sizeof(geometry_system_state);
    u64 reference_array_requirement = sizeof(geometry_reference)*config.max_geometry_count;
    *memory_requirement = struct_requirement + reference_array_requirement;

    if(!state){
        return true;
    }
    state_ptr = (geometry_system_state*)state;
    state_ptr->config = config;
    void*array_block = (char*)state + struct_requirement;
    state_ptr->registered_geometries = (geometry_reference*)array_block;

    // invalidate the enteries 
    for(u32 i=0; i < state_ptr->config.max_geometry_count; i++){
        state_ptr->registered_geometries[i].geometry.id = INVALID_ID;
        state_ptr->registered_geometries[i].geometry.internal_id = INVALID_ID;
        state_ptr->registered_geometries[i].geometry.generation = INVALID_ID;

    }
    // if(!create_default_geometry(state_ptr)){
    //     DFATAL("failed to create default geometry.Application cannot continue");
    //     return false;
    // }
   return true;
};
void geometry_system_shutdown(void*state){
    geometry_system_state*geometry_state = (geometry_system_state*)state;
    u32 count = geometry_state->config.max_geometry_count;
    for(u32 i = 0; i < count; i++){
        if(geometry_state->registered_geometries[i].geometry.id != INVALID_ID){
            destroy_geometry(geometry_state,&geometry_state->registered_geometries[i].geometry);
        }
    }
    //destroy default geometry
    // destroy_geometry(geometry_state, &geometry_state->default_geometry);
    geometry_state = 0;
};


geometry*geometry_system_acquire_by_id(u32 id){
    if(id != INVALID_ID && state_ptr->registered_geometries[id].geometry.id != INVALID_ID){
        state_ptr->registered_geometries[id].reference_count++;
        return &state_ptr->registered_geometries[id].geometry;
    }
    DERROR("geometry_system_acquire_by_id cannot load invalid geometry id returning nullptr");
    return nullptr;
};


geometry* geometry_system_acquire_from_config(geometry_config config, bool auto_release){
    geometry*g = 0;
    for(u32 i=0; i < state_ptr->config.max_geometry_count; i++){
        if(state_ptr->registered_geometries[i].geometry.id == INVALID_ID){
            state_ptr->registered_geometries[i].auto_release = auto_release;
            state_ptr->registered_geometries[i].reference_count = 1;
            g = &state_ptr->registered_geometries[i].geometry;
            g->id = i;
            break;
        }
    }
    if(!g){
        DERROR("unable to obtain free slot for geometry. Adjust configuration to allow more space Returing nullptr");
        return nullptr;
    }

    if(!create_geometry(state_ptr,config,g)){
        DERROR("failed to create geometry, Returning nullptr");
        return nullptr;
    }
    return g;
};


void geometry_release(geometry*geometry){
    if(geometry && geometry->id != INVALID_ID){
        geometry_reference*ref = &state_ptr->registered_geometries[geometry->id];
        u32 id = geometry->id;
        if(ref->geometry.id == geometry->id){
            if(ref->reference_count > 0){
                ref->reference_count--;
            }
            if(ref->reference_count < 1 && ref->auto_release){
                destroy_geometry(state_ptr, &ref->geometry);
                ref->reference_count = 0;
                ref->auto_release = false;
            }
        }
        else{
            DFATAL("Geometry id mismatch.check registration logic the Dazel spec states that a geometry id must match when creating and destroying a geometry");
        }
       return;
    }
    DWARNING("geometry_system_acquire_by_id cannot release invalid geometry id. nothing was done");
};

geometry*geometry_system_get_default(){
  if(state_ptr){
    return &state_ptr->default_geometry;
  }
  DWARNING("call to default geometry before geometry system initialization or after shutdown");
  return nullptr;
}

geometry*geometry_system_get_default_2d(){
    if(state_ptr){
    return &state_ptr->default_geometry_2d;
  }
  DWARNING("call to default geometry before geometry system initialization or after shutdown");
  return nullptr;  
};

bool create_geometry(geometry_system_state*state,geometry_config config, geometry*out_geometry){
    if(!renderer_create_geometry(out_geometry,config.vertex_count, config.vertex_size, config.verticies, config.index_size,config.index_count, config.indicies)){
        state->registered_geometries[out_geometry->id].reference_count = 0;
        state->registered_geometries[out_geometry->id].auto_release = false;
        out_geometry->id = INVALID_ID;
        out_geometry->generation = INVALID_ID;
        out_geometry->internal_id = INVALID_ID;
        return false;
    }
    string_n_copy(out_geometry->name , config.name, GEOMETRY_NAME_MAX_LENGTH);
    //aquire material 
    if(string_length(config.material_name) > 0){
        out_geometry->material = material_system_acquire(config.material_name, config.apply_material);
        out_geometry->apply_material = config.apply_material;
        if(!out_geometry->material){
           out_geometry->material= material_system_get_default_material();
           out_geometry->apply_material = false;
        }
    }
    return true;
};

bool create_default_geometry(geometry_system_state*state){
     Vertex_3d verts[4];
     Dzero_memory(verts, sizeof(Vertex_3d) * 4);
     state->default_geometry.internal_id = INVALID_ID;
     state->default_geometry.id= INVALID_ID;

     verts[0].position.x = -0.5f;
     verts[0].position.y = -0.5f;
     verts[0].position.z = 0.0f;
     verts[0].texture_cord.u = 0.0f;
     verts[0].texture_cord.v = 0.0f;

     verts[1].position.x = 0.5f;
     verts[1].position.y = 0.5f;
     verts[1].position.z = 0.0f;
     verts[1].texture_cord.u = 1.0f;
     verts[1].texture_cord.v = 1.0f;

     verts[2].position.x = -0.5f;
     verts[2].position.y = 0.5f;
     verts[2].position.z = 0.0f;
     verts[2].texture_cord.u = 0.0f;
     verts[2].texture_cord.v = 1.0f;

     verts[3].position.x = 0.5f;
     verts[3].position.y = -0.5f;
     verts[3].position.z = 0.0f;
     verts[3].texture_cord.u = 1.0f;
     verts[3].texture_cord.v = 0.0f;

     u32 indicies[6] = {0, 1, 2, 0, 3, 1};

     // send the geometry data off to the renderer to process and draw 
     string_n_copy(state->default_geometry.name, DEFAULT_GEOMETRY_NAME,GEOMETRY_NAME_MAX_LENGTH);  
     if(!renderer_create_geometry(&state->default_geometry, 4,sizeof(Vertex_3d), verts, sizeof(u32), 6, indicies)){
        DFATAL("failed to create default geometry.Application cannot continue");
        return false;
     }
    state->default_geometry.material = material_system_get_default_material();

    Vertex_2d verts2d[4];
     Dzero_memory(verts, sizeof(Vertex_2d) * 4);
     verts2d[0].position.x = -0.5f;
     verts2d[0].position.y = -0.5f;
     verts2d[0].texture_cord.x = 0.0f;
     verts2d[0].texture_cord.y = 0.0f;

     verts2d[1].position.x = 0.5f;
     verts2d[1].position.y = 0.5f;
     verts2d[1].texture_cord.x = 1.0f;
     verts2d[1].texture_cord.y = 1.0f;

     verts2d[2].position.x = -0.5f;
     verts2d[2].position.y =  0.5f;
     verts2d[2].texture_cord.x = 0.0f;
     verts2d[2].texture_cord.y = 1.0f;

     verts2d[3].position.x = 0.5f;
     verts2d[3].position.y = -0.5f;
     verts2d[3].texture_cord.x = 1.0f;
     verts2d[3].texture_cord.y = 0.0f;

     u32 indicies2[6] = {2, 1, 0, 3, 0, 1};
    
    string_n_copy(state->default_geometry_2d.name, DEFAULT_GEOMETRY_NAME,GEOMETRY_NAME_MAX_LENGTH);  
     if(!renderer_create_geometry(&state->default_geometry_2d, 4, sizeof(Vertex_2d), verts2d, sizeof(u32), 6, indicies)){
        DFATAL("failed to create default  2d geometry.Application cannot continue");
        return false;
     }
    state->default_geometry_2d.material = material_system_get_default_material();
     return true;
};

void destroy_geometry(geometry_system_state* state, geometry*geometry){
    if(state && geometry){
       renderer_destroy_geometry(geometry);
       if(!string_equal(geometry->name, DEFAULT_GEOMETRY_NAME)){
            state->registered_geometries[geometry->id].reference_count = 0;
            state->registered_geometries[geometry->id].auto_release = false;
       }
       geometry->internal_id = INVALID_ID;
       geometry->generation = INVALID_ID;
       geometry->id = INVALID_ID;
    }
    string_empty(geometry->name);
    // release the material 
    if(geometry->material && string_length(geometry->material->name)> 0){
        material_system_release(geometry->material->name);
        geometry->material = nullptr;
    }
};
geometry_config geometry_system_plane_config(float width, float height, u32 x_segment_count, u32 y_segment_count, float tile_x, float tile_y, const char*name, const char* material_name, bool apply_material){
    if(width == 0){
      DWARNING("width must be non-zero defaulting to one");
      width = 1.0f;
    }
    if(height == 0){
           DWARNING("height must be non-zero defaulting to one");
      height = 1.0f;
    }
    if(x_segment_count < 1){
        DWARNING("x_segment_count must be a positive number Defaulting to one");
        x_segment_count = 1;
    }
    if(y_segment_count < 1){
        DWARNING("y_segment_count must be a positive number Defaulting to one");
        y_segment_count = 1;
    }
    if(tile_x == 0){
        DWARNING("tile_x  must be a positive number Defaulting to one"); 
        tile_x = 1.0f;
    } 
    if(tile_y == 0){
        DWARNING("tile_y  must be a positive number Defaulting to one"); 
        tile_y = 1.0f;
    }
    geometry_config config;
    config.apply_material = apply_material;
    config.vertex_count = x_segment_count * y_segment_count * 4;
    config.vertex_size = sizeof(Vertex_3d);
    config.verticies =(Vertex_3d*)Dallocate_memory(sizeof(Vertex_3d)*config.vertex_count, MEMORY_TAG_ARRAY);
    config.index_count = x_segment_count *y_segment_count * 6;
    config.index_size = sizeof(Vertex_2d);
    config.indicies = (u32*)Dallocate_memory(sizeof(u32)*config.index_count,MEMORY_TAG_ARRAY);
    string_n_copy(config.name, name, GEOMETRY_NAME_MAX_LENGTH);

    float seg_width = width/x_segment_count;
    float seg_height = height/y_segment_count;
    float half_width = width * 0.5f;
    float half_height = height * 0.5f;
    for(u32 y = 0; y < y_segment_count; y++){
        for(u32 x= 0; x < x_segment_count; x++){
            //Generate verticies
            float min_x = (x * seg_width) - half_width;
            float min_y = (y * seg_height) - half_height;
            float max_x = min_x + seg_width;
            float max_y = min_y + seg_height;
            float min_uvx = (x / (float)x_segment_count) * tile_x;
            float min_uvy = (y / (float )y_segment_count) * tile_y;
            float max_uvx = ((x + 1)/(float)x_segment_count)*tile_x;
            float max_uvy = ((y + 1)/ (float)y_segment_count) * tile_y;

            u32 v_offset = ((y * x_segment_count) + x) * 4;
            Vertex_3d* v0 = &((Vertex_3d*)config.verticies)[v_offset + 0];
            Vertex_3d* v1 = &((Vertex_3d*)config.verticies)[v_offset + 1];
            Vertex_3d* v2 = &((Vertex_3d*)config.verticies)[v_offset + 2];
            Vertex_3d* v3 = &((Vertex_3d*)config.verticies)[v_offset + 3];


            v0->position.x = min_x;
            v0->position.y = min_y;
            v0->texture_cord.x = min_uvx;
            v0->texture_cord.y = min_uvy;

            v1->position.x = max_x;
            v1->position.y = max_y;
            v1->texture_cord.x = max_uvx;
            v1->texture_cord.y = max_uvy;

            v2->position.x = min_x;
            v2->position.y = max_y;
            v2->texture_cord.x = min_uvx;
            v2->texture_cord.y = max_uvy;

            v3->position.x = max_x;
            v3->position.y = min_y;
            v3->texture_cord.x = max_uvx;
            v3->texture_cord.y = min_uvy;

            //Generate indicies
            u32 i_offset = ((y * x_segment_count) + x )*6;
            ((u32*)config.indicies)[i_offset + 0] = v_offset + 0;
            ((u32*)config.indicies)[i_offset + 1] = v_offset + 1;
            ((u32*)config.indicies)[i_offset + 2] = v_offset + 2;
            ((u32*)config.indicies)[i_offset + 3] = v_offset + 0;
            ((u32*)config.indicies)[i_offset + 4] = v_offset + 3;
            ((u32*)config.indicies)[i_offset + 5] = v_offset + 1;

        }

    }
    if(name && string_length(name) > 0){
        string_n_copy(config.name , name , GEOMETRY_NAME_MAX_LENGTH);
    }else{
        string_n_copy(config.name, DEFAULT_GEOMETRY_NAME, GEOMETRY_NAME_MAX_LENGTH);
    }

    if(material_name && string_length(material_name) > 0){
        string_n_copy(config.material_name,material_name,MATERIAL_NAME_MAX_LENGTH);
    }else{
        string_n_copy(config.material_name,DEFAULT_MATERIAL_NAME, MATERIAL_NAME_MAX_LENGTH);
    }
  return config;
};

geometry_config geometry_system_generate_cube_config(float width, float height, float depth, float tile_x, float tile_y, const char*name, const char*material_name,bool apply_material){
       if(width == 0){
      DWARNING("width must be non-zero defaulting to one");
      width = 1.0f;
    }
    if(height == 0){
           DWARNING("height must be non-zero defaulting to one");
      height = 1.0f;
    }
    if(tile_x == 0){
        DWARNING("tile_x  must be a positive number Defaulting to one"); 
        tile_x = 1.0f;
    } 
    if(tile_y == 0){
        DWARNING("tile_y  must be a positive number Defaulting to one"); 
        tile_y = 1.0f;
    }

    geometry_config config;
    config.apply_material = apply_material;
    config.vertex_count = 4*6;
    config.vertex_size = sizeof(Vertex_3d);
    config.verticies =(Vertex_3d*)Dallocate_memory(sizeof(Vertex_3d)*config.vertex_count, MEMORY_TAG_ARRAY);
    config.index_count = 6 * 6;
    config.index_size = sizeof(Vertex_2d);
    config.indicies = (u32*)Dallocate_memory(sizeof(u32)*config.index_count,MEMORY_TAG_ARRAY);
    string_n_copy(config.name, name, GEOMETRY_NAME_MAX_LENGTH);

    float half_width = width*0.5f;
    float half_height = height * 0.5f;
    float half_depth = depth * 0.5f;
    float min_x = - half_width;
    float min_y = - half_height;
    float min_z = - half_depth;
    float max_x = half_width;
    float max_y = half_height;
    float max_z = half_depth;
    float min_uvx = 0.0f;
    float min_uvy = 0.0f;
    float max_uvx = tile_x;
    float max_uvy = tile_y;

    Vertex_3d verts[24];
    // front face 
    verts[(0 * 4) + 0].position = (vec3){min_x, min_y, max_z};
    verts[(0 * 4) + 1].position = (vec3){max_x, max_y, max_z};
    verts[(0 * 4) + 2].position = (vec3){min_x, max_y, max_z};
    verts[(0 * 4) + 3].position = (vec3){max_x, min_y, max_z};
    verts[(0 * 4) + 0].texture_cord = (vec2){min_uvx, min_uvy};
    verts[(0 * 4) + 1].texture_cord = (vec2){max_uvx, max_uvy};
    verts[(0 * 4) + 2].texture_cord = (vec2){min_uvx, max_uvy};
    verts[(0 * 4) + 3].texture_cord = (vec2){max_uvx, min_uvy};
    verts[(0 * 4) + 0].normal = (vec3){0.0f,0.0f,1.0f};
    verts[(0 * 4) + 1].normal = (vec3){0.0f,0.0f,1.0f};
    verts[(0 * 4) + 2].normal = (vec3){0.0f,0.0f,1.0f};
    verts[(0 * 4) + 3].normal = (vec3){0.0f,0.0f,1.0f};

    // back face 
    verts[(1 * 4) + 0].position = (vec3){max_x, min_y, min_z};
    verts[(1 * 4) + 1].position = (vec3){min_x, max_y, min_z};
    verts[(1 * 4) + 2].position = (vec3){max_x, max_y, min_z};
    verts[(1 * 4) + 3].position = (vec3){min_x, min_y, min_z};
    verts[(1 * 4) + 0].texture_cord = (vec2){min_uvx, min_uvy};
    verts[(1 * 4) + 1].texture_cord = (vec2){max_uvx, max_uvy};
    verts[(1 * 4) + 2].texture_cord = (vec2){min_uvx, max_uvy};
    verts[(1 * 4) + 3].texture_cord = (vec2){max_uvx, min_uvy};
    verts[(1 * 4) + 0].normal = (vec3){0.0f,0.0f,-1.0f};
    verts[(1 * 4) + 1].normal = (vec3){0.0f,0.0f,-1.0f};
    verts[(1 * 4) + 2].normal = (vec3){0.0f,0.0f,-1.0f};
    verts[(1 * 4) + 3].normal = (vec3){0.0f,0.0f,-1.0f};


    // left face 
    verts[(2 * 4) + 0].position = (vec3){min_x, min_y, min_z};
    verts[(2 * 4) + 1].position = (vec3){min_x, max_y, max_z};
    verts[(2 * 4) + 2].position = (vec3){min_x, max_y, min_z};
    verts[(2 * 4) + 3].position = (vec3){min_x, min_y, max_z};
    verts[(2 * 4) + 0].texture_cord = (vec2){min_uvx, min_uvy};
    verts[(2 * 4) + 1].texture_cord = (vec2){max_uvx, max_uvy};
    verts[(2 * 4) + 2].texture_cord = (vec2){min_uvx, max_uvy};
    verts[(2 * 4) + 3].texture_cord = (vec2){max_uvx, min_uvy};
    verts[(2 * 4) + 0].normal = (vec3){-1.0f,0.0f,0.0f};
    verts[(2 * 4) + 1].normal = (vec3){-1.0f,0.0f,0.0f};
    verts[(2 * 4) + 2].normal = (vec3){-1.0f,0.0f,0.0f};
    verts[(2 * 4) + 3].normal = (vec3){-1.0f,0.0f,0.0f};

    // right face 
    verts[(3 * 4) + 0].position = (vec3){max_x, min_y, max_z};
    verts[(3 * 4) + 1].position = (vec3){max_x, max_y, min_z};
    verts[(3 * 4) + 2].position = (vec3){max_x, max_y, max_z};
    verts[(3 * 4) + 3].position = (vec3){max_x, min_y, min_z};
    verts[(3 * 4) + 0].texture_cord = (vec2){min_uvx, min_uvy};
    verts[(3 * 4) + 1].texture_cord = (vec2){max_uvx, max_uvy};
    verts[(3 * 4) + 2].texture_cord = (vec2){min_uvx, max_uvy};
    verts[(3 * 4) + 3].texture_cord = (vec2){max_uvx, min_uvy};
    verts[(3 * 4) + 0].normal = (vec3){1.0f,0.0f,0.0f};
    verts[(3 * 4) + 1].normal = (vec3){1.0f,0.0f,0.0f};
    verts[(3 * 4) + 2].normal = (vec3){1.0f,0.0f,0.0f};
    verts[(3 * 4) + 3].normal = (vec3){1.0f,0.0f,0.0f};


    // bottom face 
    verts[(4 * 4) + 0].position = (vec3){max_x, min_y, max_z};
    verts[(4 * 4) + 1].position = (vec3){min_x, min_y, min_z};
    verts[(4 * 4) + 2].position = (vec3){max_x, min_y, min_z};
    verts[(4 * 4) + 3].position = (vec3){min_x, min_y, max_z};
    verts[(4 * 4) + 0].texture_cord = (vec2){min_uvx, min_uvy};
    verts[(4 * 4) + 1].texture_cord = (vec2){max_uvx, max_uvy};
    verts[(4 * 4) + 2].texture_cord = (vec2){min_uvx, max_uvy};
    verts[(4 * 4) + 3].texture_cord = (vec2){max_uvx, min_uvy};
    verts[(4 * 4) + 0].normal = (vec3){0.0f,-1.0f,0.0f};
    verts[(4 * 4) + 1].normal = (vec3){0.0f,-1.0f,0.0f};
    verts[(4 * 4) + 2].normal = (vec3){0.0f,-1.0f,0.0f};
    verts[(4 * 4) + 3].normal = (vec3){0.0f,-1.0f,0.0f};


    // top face 
    verts[(5 * 4) + 0].position = (vec3){min_x, max_y, max_z};
    verts[(5 * 4) + 1].position = (vec3){max_x, max_y, min_z};
    verts[(5 * 4) + 2].position = (vec3){min_x, max_y, min_z};
    verts[(5 * 4) + 3].position = (vec3){max_x, max_y, max_z};
    verts[(5 * 4) + 0].texture_cord = (vec2){min_uvx, min_uvy};
    verts[(5 * 4) + 1].texture_cord = (vec2){max_uvx, max_uvy};
    verts[(5 * 4) + 2].texture_cord = (vec2){min_uvx, max_uvy};
    verts[(5 * 4) + 3].texture_cord = (vec2){max_uvx, min_uvy};
    verts[(5 * 4) + 0].normal = (vec3){0.0f,1.0f,0.0f};
    verts[(5 * 4) + 1].normal = (vec3){0.0f,1.0f,0.0f};
    verts[(5 * 4) + 2].normal = (vec3){0.0f,1.0f,0.0f};
    verts[(5 * 4) + 3].normal = (vec3){0.0f,1.0f,0.0f};
    Dmemory_copy(config.verticies, verts, config.vertex_size*config.vertex_count);

    for(u32 i = 0; i < 6; i++){
        u32 v_offset = i * 4;
        u32 i_offset = i * 6;
        ((u32*)config.indicies)[i_offset + 0] = v_offset + 0;
        ((u32*)config.indicies)[i_offset + 1] = v_offset + 1;
        ((u32*)config.indicies)[i_offset + 2] = v_offset + 2;
        ((u32*)config.indicies)[i_offset + 3] = v_offset + 0;
        ((u32*)config.indicies)[i_offset + 4] = v_offset + 3;
        ((u32*)config.indicies)[i_offset + 5] = v_offset + 1;
    }
    geometry_generate_normals(config.vertex_count, (Vertex_3d*)config.verticies,config.index_count, (u32*)config.indicies);
    geometry_generate_tangent(config.vertex_count, (Vertex_3d*)config.verticies,config.index_count, (u32*)config.indicies);

    if(name && string_length(name) > 0){
        string_n_copy(config.name , name , GEOMETRY_NAME_MAX_LENGTH);
    }else{
        string_n_copy(config.name, DEFAULT_GEOMETRY_NAME, GEOMETRY_NAME_MAX_LENGTH);
    }

    if(material_name && string_length(material_name) > 0){
        string_n_copy(config.material_name,material_name,MATERIAL_NAME_MAX_LENGTH);
    }else{
        string_n_copy(config.material_name,DEFAULT_MATERIAL_NAME, MATERIAL_NAME_MAX_LENGTH);
    }
    return config;
}
