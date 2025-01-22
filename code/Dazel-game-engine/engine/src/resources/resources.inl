#pragma once
#include "defines.h"
#include "Math/dlm.h"
#include "Math/transform.h"
using namespace dlm;
#define  MATERIAL_NAME_MAX_LENGTH 512

#define  TEXTURE_NAME_MAX_LENGTH 512

#define   GEOMETRY_NAME_MAX_LENGTH 512

#define SHADER_NAME_MAX_LENGTH 512

#define MAX_ATTRIBUTE_COUNT 20
#define MAX_UNIFORM_COUNT 20
#define MAX_SAMPLER_COUNT 20
#define MAX_STAGE_NAME_LENGTH 128
#define MAX_ATTRIBUTE_TYPE_NAME_LENGTH 128
#define MAX_UNIFORM_TYPE_NAME_LENGTH 128
#define MAX_SAMPLER_TYPE_NAME_LENGTH 128

#define MAX_MATERIAL_TEXTURE_MAPS 3

enum resource_types{
  RESOURCE_TYPE_TEXT,
  RESOURCE_TYPE_BINARY,
  RESOURCE_TYPE_IMAGE,
  RESOURCE_TYPE_MATERIAL,
  RESOURCE_TYPE_STATIC_MESH,
  RESOURCE_TYPE_CUSTOM,
  RESOURCE_TYPE_SHADER
};

typedef struct resource{
    u32 loader_id;
    const char* name;
    char* full_path;
    u64 size_of_data;
    void*data;
}resource;

typedef struct image_resource_data{
    u8*pixels;
    u32 channel_count;
    u32 width;
    u32 height;
}image_resource_data;

enum material_type{
  MATERIAL_TYPE_WORLD,
  MATERIAL_TYPE_LIGHT,
  MATERIAL_TYPE_UI
};

typedef struct material_config{
    char name[MATERIAL_NAME_MAX_LENGTH];
    material_type type;
    bool auto_release;
    vec4 diffuse_color;
    char diffuse_map_name[TEXTURE_NAME_MAX_LENGTH]; // becomes the texture name 
    char specular_map_name[TEXTURE_NAME_MAX_LENGTH]; 
    char normal_map_name[TEXTURE_NAME_MAX_LENGTH]; 
    char shader_name[SHADER_NAME_MAX_LENGTH];
    float shineness;
    int sample_count_less;
}material_config;

typedef struct Texture{
   u32 id;
   u32 width;
   u32 height;
   u8 channel_count;
   bool has_transparency;
   u32 generation;
  char name[TEXTURE_NAME_MAX_LENGTH];
   void*internal_data;
}Texture;

enum texture_use{
  TEXTURE_USE_UNKNOWN = 0x00,
  TEXTURE_USE_MAP_DIFFUSE = 0x01,
  TEXTURE_USE_MAP_SPECULAR = 0x02,
  TEXTURE_USE_MAP_NORMAL = 0x03 
};

typedef struct texture_map{
  Texture*texture;
  texture_use use;
}texture_map;

typedef struct materials{
  u32 id;
  u32 shader_id;
  u32 bound_instance_id;
  u32 internal_id;
  material_type type;
  u32 generations;
  vec4 diffuse_color;
  float shineness;
  int sample_count_less;
  // TODO: to phase this method out 
  texture_map diffuse_map;
  texture_map specular_map;
  texture_map normal_map;
  // TODO:
  texture_map texture_maps[MAX_MATERIAL_TEXTURE_MAPS];
  char name[MATERIAL_NAME_MAX_LENGTH];
  bool apply_material;
  u32 renderer_frame_number;
}materials;

typedef struct geometry{
 u32 id;
 u32 internal_id;
 char name[GEOMETRY_NAME_MAX_LENGTH];
 u32 generation;
 materials*material;
 bool apply_material;
}geometry;

typedef struct mesh{
 u16 geometry_count;
 geometry**geometries;
 transform transform;
 mat4 model;
}mesh;

typedef struct material_shader_config{
    bool use_instance;
    bool use_local;
    u32 stage_count;
    u32 attribute_count;
    u32 uniform_count;
    u32 sampler_count;
    char renderpass[128];
    char shader_name[SHADER_NAME_MAX_LENGTH];
    char stage_names[4][MAX_STAGE_NAME_LENGTH];
    char stage_file_names[4][MAX_STAGE_NAME_LENGTH];
    char attribute_names[MAX_ATTRIBUTE_COUNT][MAX_ATTRIBUTE_TYPE_NAME_LENGTH];
    char attribute_type[MAX_ATTRIBUTE_COUNT][MAX_ATTRIBUTE_TYPE_NAME_LENGTH];
    char uniform_names[MAX_UNIFORM_COUNT][MAX_UNIFORM_TYPE_NAME_LENGTH];
    char uniform_type[MAX_UNIFORM_COUNT][MAX_UNIFORM_TYPE_NAME_LENGTH];
    char uniform_scope[MAX_UNIFORM_COUNT][MAX_UNIFORM_TYPE_NAME_LENGTH];
    char sampler_name[MAX_SAMPLER_COUNT][MAX_UNIFORM_TYPE_NAME_LENGTH];
    char sampler_scope[MAX_SAMPLER_COUNT][MAX_UNIFORM_TYPE_NAME_LENGTH];
}material_shader_config;