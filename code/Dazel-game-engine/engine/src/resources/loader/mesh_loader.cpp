#include "mesh_loader.h"
#include "core/logger.h"
#include "core/Dmemory.h"
#include "core/Dstrings.h"
#include "containers/Darray.h"
#include "resources/resources.inl"
#include "systems/resource_system.h"
#include "systems/geometry_system.h"
#include "Math/dlm.h"
#include "Math/geometry_math_utils.h"
#include "resources/loader/loader_utils.h"
#include "platform/file_system.h"

#include <stdio.h>

enum mesh_file_type{
 MESH_FILE_TYPE_NOT_FOUND,
 MESH_FILE_TYPE_DSM,
 MESH_FILE_TYPE_OBJ
};
typedef struct supported_mesh_file_type{
char*extension;
mesh_file_type type;
bool is_binary;
}supported_mesh_file_type;

typedef struct mesh_vertex_index_data{
 u32 position_index;
 u32 normal_index;
 u32 textcoord_index;
}mesh_vertex_index_data;

typedef struct mesh_face_data{
  mesh_vertex_index_data verticies[3];
}mesh_face_data;

typedef struct mesh_group_data{
   mesh_face_data*faces;
}mesh_group_data;


bool import_obj_file(file_handle*obj_file, const char*out_dsm_filename,geometry_config**out_geometry_darray);
void process_subobject(vec3*positions, vec3*normals,vec2*textcoords,mesh_face_data*faces,geometry_config*out_data);
bool import_obj_material_libary_file(const char* material_file_path);
bool load_dsm_file(file_handle*dsm_file,geometry_config**out_geometry_darray);
bool write_dsm_file(const char*path, const char*name, u32 geometry_count, geometry_config*geometries);
bool write_dmt_file(const char*material_file_path, material_config*config);

bool mesh_loader_load(struct resource_loader*self, const char* name, resource*out_resources){
    if(!self ||!name || !out_resources){
        return false;
    }

    char* format_str = "%s/%s/%s%s";
    file_handle f;
    #define SUPPORTED_FILE_TYPE_COUNT 2
         supported_mesh_file_type supported_file_types[SUPPORTED_FILE_TYPE_COUNT];
         supported_file_types[0] = (supported_mesh_file_type){".dsm",MESH_FILE_TYPE_DSM,true};
         supported_file_types[1] = (supported_mesh_file_type){".obj",MESH_FILE_TYPE_OBJ,false};

         char full_file_path[512];
         mesh_file_type type = MESH_FILE_TYPE_NOT_FOUND;
         for(u32 i=0; i < SUPPORTED_FILE_TYPE_COUNT; i++){
            string_format(full_file_path,format_str,resource_system_base_path(),self->type_path,name,supported_file_types[i].extension);
            if(filesystem_exists(full_file_path)){
                if(filesystem_open(full_file_path,FILE_MODE_READ,supported_file_types[i].is_binary,&f)){
                    type = supported_file_types[i].type;
                    break;
                }
            }
         }
         if(type == MESH_FILE_TYPE_NOT_FOUND){
            DERROR("unable to find mesh of supported type called '%s'.",name);
            return false;
         }
         out_resources->full_path = string_duplicate(full_file_path);
         geometry_config*resource_data = darray_create(geometry_config);

         bool result = false;
         switch (type){
            case MESH_FILE_TYPE_OBJ:{
                char dsm_file_name[512];
                string_format(dsm_file_name,"%s/%s/%s%s",resource_system_base_path(),self->type_path,name,".dsm");
                result = import_obj_file(&f,dsm_file_name,&resource_data);
                break;
            }
            case MESH_FILE_TYPE_DSM:
                 result = load_dsm_file(&f, &resource_data);
                 break;
            default:
            case MESH_FILE_TYPE_NOT_FOUND:
                 DERROR("unable to find mesh of supported type called '%s'", name);
                 result = false;
                 break;     
         }
         filesystem_close(&f);
         if(!result){
            DERROR("failed to process mesh file '%s'",full_file_path);
            darray_destroy_array(resource_data);
            out_resources->data = 0;
            out_resources->size_of_data = 0;
            return false;
         }
         out_resources->data = resource_data;
         out_resources->size_of_data = darray_get_length(resource_data);
         return true;
}

void mesh_loader_unload(struct resource_loader*self, resource*resource){
    u32 count = darray_get_length(resource->data);
    for(u32 i =0; i < count; i++){
        geometry_config*config = &((geometry_config*)resource->data)[i];
        geometry_system_config_dispose(config);
    }
    darray_destroy_array(resource->data);
    resource->data = 0;
    resource->size_of_data = 0;
}

bool load_dsm_file(file_handle*dsm_file,geometry_config**out_geometry_darray){
//   geometry_config*geometry_darray = *out_geometry_darray;
  //version
  u64 written = 0;
  u16 version = 0x0001U;
  filesystem_read(dsm_file,sizeof(u16),&version,&written); 

  //name  length
//   u32 name_length;
//   filesystem_read(dsm_file,sizeof(u32),&name_length,&written);
//    char name[512];
//   filesystem_read(dsm_file, sizeof(char)*name_length,name,&written);

  // geometry_count
  u32 geometry_count = 0;
  filesystem_read(dsm_file,sizeof(u32),&geometry_count,&written);

  DDEBUG("reading from dsm file with geometry count of %u",geometry_count);
  // write each geometry
  for(u32 i=0; i < geometry_count; i++){
    geometry_config g;
    // vertex size and vertex count
    filesystem_read(dsm_file, sizeof(u32),&g.vertex_size,&written);
    filesystem_read(dsm_file, sizeof(u32),&g.vertex_count,&written);
    g.verticies = Dallocate_memory(g.vertex_count*g.vertex_size,MEMORY_TAG_ARRAY);
    filesystem_read(dsm_file, g.vertex_count * g.vertex_size, g.verticies,&written);

    // index size and vertex count
    filesystem_read(dsm_file, sizeof(u32),&g.index_size,&written);
    filesystem_read(dsm_file, sizeof(u32),&g.index_count,&written);
    g.indicies = Dallocate_memory(g.index_count*g.index_size,MEMORY_TAG_ARRAY);
    filesystem_read(dsm_file, g.index_count * g.index_size, g.indicies,&written);

    // geometry_name 
    // u32 g_name_length;
    // filesystem_read(dsm_file, sizeof(u32), &g_name_length,&written);
    // filesystem_read(dsm_file, sizeof(char)*g_name_length, g.name, &written);
    // material_name
    u32 m_name_length;
    filesystem_read(dsm_file, sizeof(u32), &m_name_length,&written);
    filesystem_read(dsm_file, sizeof(char)*m_name_length, g.material_name, &written);
    
    filesystem_read(dsm_file,sizeof(vec3),&g.min_extent,&written);
    filesystem_read(dsm_file,sizeof(vec3),&g.max_extent,&written);
    filesystem_read(dsm_file,sizeof(vec3),&g.center,&written);   
    darray_push(*out_geometry_darray,g,geometry_config);
  }
  filesystem_close(dsm_file);
  return true;
};
bool write_dsm_file(const char*path, const char*name, u32 geometry_count, geometry_config*geometries){
  if(filesystem_exists(path)){
    DWARNING("file %s already exists and will be overwritten");
    return false;
  }
  file_handle f; 
  if(!filesystem_open(path, FILE_MODE_WRITE,true,&f)){
    DERROR("Unable to open file : %s", name);
    return false;
  };
  //version
  u64 written = 0;
  u16 version =0x0001U;
  filesystem_write(&f, sizeof(u16), &version,&written); 

  //name  length
//   u32 name_length = string_length(name) + 1;
//   filesystem_write(&f,sizeof(u32),&name_length,&written);

//   filesystem_write(&f, sizeof(char)*name_length,name,&written);

  // geometry_count
  filesystem_write(&f,sizeof(u32),&geometry_count,&written);

  // write each geometry
  for(u32 i=0; i < geometry_count; i++){
    geometry_config*g = &geometries[i];
    // vertex size and vertex count
    filesystem_write(&f, sizeof(u32),&g->vertex_size,&written);
    filesystem_write(&f, sizeof(u32),&g->vertex_count,&written);
    filesystem_write(&f, g->vertex_count * g->vertex_size, g->verticies,&written);

    // index size and vertex count
    filesystem_write(&f, sizeof(u32),&g->index_size,&written);
    filesystem_write(&f, sizeof(u32),&g->index_count,&written);
    filesystem_write(&f, g->index_count * g->index_size, g->indicies,&written);

    // geometry_name 
    // u32 g_name_length = string_length(g->name) + 1;
    // filesystem_write(&f, sizeof(u32), &g_name_length,&written);
    // filesystem_write(&f, sizeof(char)*g_name_length, g->name, &written);
    // material_name
    u32 m_name_length = string_length(g->material_name) + 1;
    filesystem_write(&f, sizeof(u32), &m_name_length,&written);
    filesystem_write(&f, sizeof(char)*m_name_length, g->material_name, &written);
    
    filesystem_write(&f,sizeof(vec3),&g->min_extent,&written);
    filesystem_write(&f,sizeof(vec3),&g->max_extent,&written);
    filesystem_write(&f,sizeof(vec3),&g->center,&written);   
  }
  filesystem_close(&f);
  return true;
};


bool import_obj_file(file_handle*obj_file, const char*out_dsm_filename,geometry_config**out_geometry_darray){
    vec3*positions = darray_reservered(16384,vec3);

    vec3*normal = darray_reservered(16384,vec3);

    vec2*text_coord = darray_reservered(16384,vec2);

    mesh_group_data*groups = darray_reservered(4,mesh_group_data);

    char material_file_name[512] = "";
    char name[512];
    u8 current_material_name_count  = 0;
    char material_names[32][64];

    char line_buff[512] = "";
    char*p = &line_buff[0];
    u64 line_length = 0;

    char prev_first_chars[2] = {0,0};

    while(true){
        if(!filesystem_read_line(obj_file, 511,&p,&line_length)){
            break;
        }
        // if the line is blank 
        if(line_length < 1){
            continue;
        }
        char first_char = line_buff[0];

        switch(first_char){
            case '#':
                continue;
            case 'v':{
                        char second_char = line_buff[1];
                        switch (second_char)
                        {
                            case ' ':{
                                vec3 pos;
                                char t[2];
                                sscanf(
                                    line_buff,
                                    "%S %f %f %f",
                                    t,
                                    &pos.x,
                                    &pos.y,
                                    &pos.z
                                );
                                darray_push(positions,pos,vec3);
                            }break;
                            case 'n':{
                                vec3 nom;
                                char t[2];
                                sscanf(
                                    line_buff,
                                    "%S %f %f %f",
                                    t,
                                    &nom.x,
                                    &nom.y,
                                    &nom.z
                                );
                                darray_push(normal,nom,vec3);
                            }break;
                            case 't':{
                                vec2 text_cord;
                                char t[2];
                                sscanf(
                                    line_buff,
                                    "%S %f %f",
                                    t,
                                    &text_cord.x,
                                    &text_cord.y
                                );
                                darray_push(text_coord,text_cord,vec2);
                            }break;
                            }
            }break;
            case 's':{

            }break;
            case 'f':{
                  mesh_face_data face;
                  char t[2];
                  u64 normal_count = darray_get_length(normal);
                  u64 text_coord_count = darray_get_length(text_coord);
                  u64 position_count = darray_get_length(positions);
                  //this is if the obj file does not contain normal or textcoord 
                  if(normal_count == 0 || text_coord_count == 0){
                    sscanf(
                        line_buff,
                        "%s %d %d %d",
                        t,
                        &face.verticies[0].position_index,
                        &face.verticies[1].position_index,
                        &face.verticies[2].position_index
                    );
                  }
                  else{
                      sscanf(
                        line_buff,
                        "%s %d/%d/%d %d/%d/%d %d/%d/%d",
                        t,
                        &face.verticies[0].position_index,
                        &face.verticies[0].textcoord_index,
                        &face.verticies[0].normal_index,

                        &face.verticies[1].position_index,
                        &face.verticies[1].textcoord_index,
                        &face.verticies[1].normal_index,

                        &face.verticies[2].position_index,
                        &face.verticies[2].textcoord_index,
                        &face.verticies[2].normal_index

                      );
                  }
                  
                  u64 group_index = darray_get_length(groups) -1;
                  darray_push(groups[group_index].faces,face,mesh_face_data);
            }break;  
            case 'm':{
                char substr[7];
                sscanf(
                    line_buff,
                    "%s %s",
                    substr,
                    material_file_name
                );
            }break; 
            case 'u':{
                mesh_group_data new_group;
                new_group.faces = darray_reservered(16384,mesh_face_data);
                darray_push(groups,new_group,mesh_group_data);

                char t[8];
                sscanf(line_buff, "%s %s",t,material_names[current_material_name_count]);
                current_material_name_count++;
            }break;
            case 'g':{
                u64 group_count = darray_get_length(groups);
                for(u64 i =0; i < group_count; i++){
                    geometry_config new_data = {};
                    string_n_copy(new_data.name, name, 255);
                    if(i < 0){
                        string_append_int(new_data.name, new_data.name,i);
                    }
                    string_n_copy(new_data.material_name,material_names[i], 255);

                    process_subobject(positions,normal, text_coord,groups[i].faces,&new_data);
                    new_data.vertex_count = darray_get_length(new_data.verticies);
                    new_data.vertex_size = sizeof(Vertex_3d);
                    new_data.index_count = darray_get_length(new_data.indicies);
                    new_data.index_size = sizeof(u32);

                    darray_push(*out_geometry_darray, new_data, geometry_config);

                    darray_destroy_array(groups[i].faces);
                    Dzero_memory(material_names[i], 64);

                }

                current_material_name_count = 0;
                darray_clear(groups);
                Dzero_memory(name, 512);

                char t[2];
                sscanf(line_buff, "%s %s", t, name);
            }break;   
       }
       prev_first_chars[1] = prev_first_chars[0];
       prev_first_chars[0] = first_char;
       Dzero_memory(line_buff, sizeof(char)*512);
    }

     u64 group_count = darray_get_length(groups);
     for(u64 i =0; i < group_count; i++){
        geometry_config new_data = {};
        string_n_copy(new_data.name, name, 255);
        if(i < 0){
            string_append_int(new_data.name, new_data.name,i);
        }
        string_n_copy(new_data.material_name,material_names[i], 255);

        process_subobject(positions,normal, text_coord,groups[i].faces,&new_data);
        new_data.vertex_count = darray_get_length(new_data.verticies);
        new_data.vertex_size = sizeof(Vertex_3d);
        new_data.index_count = darray_get_length(new_data.indicies);
        new_data.index_size = sizeof(u32);

        darray_push(*out_geometry_darray, new_data, geometry_config);

        darray_destroy_array(groups[i].faces);
    }

    darray_destroy_array(groups);
    darray_destroy_array(positions);
    darray_destroy_array(normal);
    darray_destroy_array(text_coord);
    if(string_length(material_file_name) > 0){
        char full_material_path[512];
        // string_directory_from_path(full_material_path,out_dsm_filename);
        // string_append_string(full_material_path,full_material_path,material_file_name);
        string_format(full_material_path,"%s/%s/%s", resource_system_base_path(), "materials",material_file_name);
        if(!import_obj_material_libary_file(full_material_path)){
            DERROR("Error reading obj material file");
        }
    }

    u32 count = darray_get_length(*out_geometry_darray);
    for(u64 i=0; i < count; i++){
        geometry_config*g = &((*out_geometry_darray)[i]);
        DDEBUG("Geometry de- duplication process starting on geometry object named %s .....", g->name);

        u32 new_vert_count = 0;
        Vertex_3d* unique_verts = 0;
        geometry_deduplicate_verticies(g->vertex_count,(Vertex_3d*)g->verticies,g->index_count,(u32*)g->indicies,&new_vert_count,&unique_verts);

        darray_destroy_array(g->verticies);

        g->verticies = unique_verts;
        g->vertex_count = new_vert_count;

        u32*indices = (u32*)Dallocate_memory(sizeof(u32)*g->index_count,MEMORY_TAG_ARRAY);
        Dmemory_copy(indices, g->indicies, sizeof(u32)*g->index_count);
        darray_destroy_array(g->indicies);
        g->indicies = indices;
    }

   return write_dsm_file(out_dsm_filename,name,count, *out_geometry_darray);
}

void process_subobject(vec3*positions, vec3*normals,vec2*textcoords,mesh_face_data*faces,geometry_config*out_data){
    out_data->indicies = darray_create(u32);
    out_data->verticies = darray_create(Vertex_3d);
    bool extent_set = false;
    Dzero_memory(&out_data->min_extent, sizeof(vec3));
    Dzero_memory(&out_data->max_extent, sizeof(vec3));

    u64 face_count = darray_get_length(faces);
    u64 normal_count = darray_get_length(normals);
    u64 text_coord_count = darray_get_length(textcoords);

    bool skip_normals = false;
    bool skip_tex_coord = false;

    if(normal_count == 0){
        DWARNING("No normal are present in this model");
        skip_normals = true;
    }
    if(text_coord_count == 0){
        DWARNING("No texture coordinates are present in this model");
        skip_tex_coord = true;
    }

    for(u64 f=0; f < face_count; f++){
        mesh_face_data face = faces[f];
        for(u64 i =0; i < 3; i++){
            mesh_vertex_index_data index_data = face.verticies[i];
            darray_push(out_data->indicies, (u32)(i + (f*3)),u32);

            Vertex_3d vert;

            vec3 pos = positions[index_data.position_index-1];
            vert.position = pos;

            if(pos.x < out_data->min_extent.x || !extent_set){
                out_data->min_extent.x = pos.x;
            }
            if(pos.y < out_data->min_extent.y || !extent_set){
                out_data->min_extent.y = pos.y;
            }
           if(pos.z < out_data->min_extent.z || !extent_set){
                out_data->min_extent.z = pos.z;
            }


            if(pos.x > out_data->max_extent.x || !extent_set){
                out_data->max_extent.x = pos.x;
            }
            if(pos.y > out_data->max_extent.y || !extent_set){
                out_data->max_extent.y = pos.y;
            }
           if(pos.z > out_data->max_extent.z || !extent_set){
                out_data->max_extent.z = pos.z;
            }

            extent_set = true;

            if(skip_normals){
                vert.normal = vec3_create(0.0,0.0,1.0);
            }else{
                vert.normal = normals[index_data.normal_index-1];
            }

          if(skip_tex_coord){
                vert.texture_cord = vec2_zero();
            }else{
                vert.texture_cord = textcoords[index_data.textcoord_index-1];
            
           }
           darray_push(out_data->verticies,vert,Vertex_3d);

    }
  }
  for(u8 i=0; i < 3; i++){
    out_data->center.elements[i] =(out_data->min_extent.elements[i] + out_data->max_extent.elements[i])/2.0f;
  }
//   geometry_generate_tangent(out_data->vertex_count, (Vertex_3d*)out_data->verticies,out_data->index_count, (u32*)out_data->indicies);
};

bool import_obj_material_libary_file(const char* material_file_path){
    DDEBUG("Importing obj .mtl file '%s'",material_file_path);

    file_handle mtl_file;
    if(!filesystem_open(material_file_path, FILE_MODE_READ, false, &mtl_file)){
       DERROR("Unable to open mtl file: '%s'",material_file_path);
       return false;
    }

    material_config current_config;
    Dzero_memory(&current_config, sizeof(current_config));

    bool hit_name = false;
    char*line = 0;
    char line_buffer[512];
    char*p = &line_buffer[0];
    u64 line_length = 0;
    while(true){
      if(!filesystem_read_line(&mtl_file, 511, &p, &line_length)){
        break;
      } 
      line = string_trim(line_buffer);
      line_length = string_length(line);

      if(line_length < 1){
         continue;
      } 

      char first_char = line[0];
      switch (first_char){
        case '#':
          continue;
        case 'K':{
                char second_char = line[1];
                switch (second_char)
                {
                    case 'a':
                    case 'd':{
                        char t[2];
                        sscanf(
                            line,
                            "%s %f %f %f",
                            t,
                            &current_config.diffuse_color.r,
                            &current_config.diffuse_color.g,
                            &current_config.diffuse_color.b
                        );
                    current_config.diffuse_color.a = 1.0f;
                    }break;
                    case 's':{
                        char t[2];
                        float spec_rubbish = 0.0f;
                        sscanf(
                            line,
                            "%s %f %f %f",
                            t,
                            &spec_rubbish,
                            &spec_rubbish,
                            &spec_rubbish
                        );
                    }break;
                }
        }break; 
       case 'N': {
            char second_char = line[1];
            switch (second_char)
            {
            case 'S':{
               char t[2];
               sscanf(line,"%s %f",t,&current_config.shineness);
            }break;
            }
       }break;
       case 'm':{
          char substr[10];
          char texture_file_name[512];
          sscanf(
            line,
            "%s %s",
             substr,
             texture_file_name
          );
          if(string_nequali(substr, "map_Kd",6)){
            string_filename_no_extension_from_path(current_config.diffuse_map_name,texture_file_name);
          }else if(string_nequali(substr,"map_Ks",6)){
              string_filename_no_extension_from_path(current_config.specular_map_name,texture_file_name);
          }
          else if (string_nequali(substr,"map_bump",8) || string_nequali(substr,"map_Disp",8)){
             string_filename_no_extension_from_path(current_config.normal_map_name,texture_file_name);
          }
       }break;
       case 'b':{
            char substr[10];
            char texture_file_name[512];
            sscanf(
                line,
                "%s %s",
                substr,
                texture_file_name
            );
            if(string_nequali(substr,"bump",4)){
                string_filename_no_extension_from_path(current_config.normal_map_name,texture_file_name);
            }
       }break;
       case 'n':{
            char substr[10];
            char material_name[512];
            sscanf(
                line,
                "%s %s",
                substr,
                material_name
            ); 
           
            if(string_nequali(substr,"newmtl",6)){
                if(hit_name){
                    if(!write_dmt_file(material_file_path,&current_config)){
                        DERROR("Unable to write dmt file");
                        return false;
                    }

                    Dzero_memory(&current_config,sizeof(current_config));
                }
               string_n_copy(current_config.shader_name,"built_in_shader",SHADER_NAME_MAX_LENGTH);
                if(current_config.shineness == 0.0f){
                    current_config.shineness = 8.0f;
                }
                hit_name = true;
                string_n_copy(current_config.name,material_name,MATERIAL_NAME_MAX_LENGTH);
            }
        }
      } 
    }
    
//   string_n_copy(current_config.shader_name,"built_in_shader",SHADER_NAME_MAX_LENGTH);
//    if(current_config.shineness == 0.0f){
//       current_config.shineness = 8.0f;
//     }  
 if(!write_dmt_file(material_file_path,&current_config)){
        DERROR("Unable to write dmt file");
        return false;
    }
    filesystem_close(&mtl_file);
    return true;
};

bool write_dmt_file(const char*material_file_path, material_config*config){
    char*format_str = "%s/%s%s";
    file_handle f;
    char directory[320];
    string_directory_from_path(directory,material_file_path);

    char full_file_path[512];
    string_format(full_file_path, format_str,directory,config->name,".dmt");
    if(!filesystem_open(full_file_path,FILE_MODE_WRITE,false,&f)){
        DERROR("Error opening material file for writing '%s'", full_file_path);
        return false;
    }
    DDEBUG("Writing .dmt file '%s'....",full_file_path);
    char line_buffer[512];
    filesystem_write_line(&f,"#material file");
    filesystem_write_line(&f,"");
    filesystem_write_line(&f,"version=0.1");
    string_format(line_buffer, "name=%s",config->name);
    filesystem_write_line(&f,line_buffer);
    string_format(line_buffer, "diffuse_color=%.6f %.6f %.6f %.6f",config->diffuse_color.r,config->diffuse_color.g,config->diffuse_color.b,config->diffuse_color.a);
    filesystem_write_line(&f,line_buffer);
    string_format(line_buffer, "shineness=%f", config->shineness);
    filesystem_write_line(&f,line_buffer);
    u32 sample_count = 0;
    if(config->diffuse_map_name[0]){
        string_format(line_buffer, "diffuse_map_name=%s", config->diffuse_map_name);
        filesystem_write_line(&f,line_buffer);
        sample_count++;
    }
    if(config->specular_map_name[0]){
        string_format(line_buffer, "specular_map_name=%s", config->specular_map_name);
        filesystem_write_line(&f,line_buffer);
        sample_count++;
    }
    if(config->normal_map_name[0]){
        string_format(line_buffer, "normal_map_name=%s", config->normal_map_name);
        filesystem_write_line(&f,line_buffer);
        sample_count++;
    }
    string_format(line_buffer,"shader_name=%s",config->shader_name);
    char buffer[512];
    if(sample_count < 3){
        config->sample_count_less = 1;
        string_format(buffer, "sample_count_less=%i",config->sample_count_less);
        filesystem_write_line(&f,buffer);
    }
    else{
        config->sample_count_less = 0;
        string_format(buffer, "sample_count_less=%i",config->sample_count_less);  
        filesystem_write_line(&f,buffer);
    }
    filesystem_write_line(&f,line_buffer);

    filesystem_close(&f);
    return true;
};

resource_loader mesh_resource_loader_create(){
    resource_loader loader;
    loader.type = RESOURCE_TYPE_STATIC_MESH;
    loader.custom_type = 0;
    loader.load = mesh_loader_load;
    loader.unload = mesh_loader_unload;
    loader.type_path = "models";
    return loader;
}