#version 450 
layout (location = 0) out vec4 outcolor;

layout (location = 0) flat in int in_mode;

layout (set = 1,binding = 0) uniform local_uniform_object{
  vec4 diffuse_color;
  float shineness;
  int sample_count_less;
}object_ubo;

struct directional_light {
  vec3 direction;
  vec4 colour;
};
directional_light dir_light = {
  vec3(-0.57735, -0.57735, -0.57735),
  vec4 (0.8,0.8,0.8,1.0)
};

struct point_light {
 vec3 position;
 vec4 colour;
 float constant;
 float linear;
 float quadratic;
};
 
point_light p_light_0 = {
  vec3(-5.5,0.0,-5.5),
  vec4(0.0,1.0,0.0,1.0),
  1.0,
  0.35,
  0.44
};

point_light p_light_1 = {
  vec3(5.5,0.0,-5.5),
  vec4(1.0,0.0,0.0,1.0),
  1.0,
  0.35,
  0.44
};

const int SAMPLER_DIFF = 0;
const int SAMPLER_SPEC = 1;
const int SAMPLER_NORM = 2;
const int SAMPLER_LESS = 1;
layout (set = 1, binding = 1) uniform sampler2D texture_sampler[3];

layout (location = 1) in struct dto {
   vec2 text_cord;
   vec4 ambient;
   vec3 normal_dir;
   vec3 frag_position;
   vec3 view_position;
   vec4 tangent;
}in_dto;

mat3 TBN;

vec4 calculate_directional_light(directional_light light,vec3 normal);

vec4 calculate_point_light(point_light p_light, vec3 normal);

void main(){
  vec3 normal = in_dto.normal_dir;

  vec3 tangent = in_dto.tangent.xyz;

  tangent = (tangent - dot(tangent, normal) * normal);

  vec3 bitangent = cross(in_dto.normal_dir,in_dto.tangent.xyz) * in_dto.tangent.w;

  TBN = mat3(tangent,bitangent,normal);
  vec3 local_normal;
  if(object_ubo.sample_count_less == 0){
     local_normal = 2.0 * texture(texture_sampler[SAMPLER_NORM], in_dto.text_cord).rgb - 1.0;
  }
  else{
      local_normal = 2.0 * texture(texture_sampler[SAMPLER_LESS], in_dto.text_cord).rgb - 1.0;
  } 
  normal = normalize(TBN * local_normal);

  if(in_mode == 0 || in_mode == 1){
      outcolor = calculate_directional_light(dir_light, normal);

      outcolor += calculate_point_light(p_light_0,normal);
      outcolor += calculate_point_light(p_light_1,normal);
  }
  else if(in_mode == 2){
      outcolor = vec4(abs(normal),1.0);
  }
 
}

vec4 calculate_point_light(point_light p_light, vec3 normal){

  vec3 view_direction = normalize(in_dto.view_position - in_dto.frag_position);

  vec3 light_direction = normalize(p_light.position - in_dto.frag_position);
  
  float diff = max(dot(normal,light_direction),0.0);

  vec3 reflect_direction = reflect(-light_direction,normal);

  float spec = pow(max(dot(view_direction,reflect_direction),0.0),object_ubo.shineness);

  float distance = length(p_light.position - in_dto.frag_position);

  float attenuation = 1.0 / (p_light.constant + p_light.linear * distance + p_light.quadratic *(distance * distance));

  vec4 ambient = in_dto.ambient;

  vec4 diffuse = p_light.colour * diff;

  vec4 specular = p_light.colour * spec;

  if(in_mode == 0){

    vec4 diff_samp = texture(texture_sampler[SAMPLER_DIFF],in_dto.text_cord);

    diffuse *= diff_samp;

    ambient *= diff_samp;
    if(object_ubo.sample_count_less == 0){
        specular *=  vec4(texture(texture_sampler[SAMPLER_SPEC], in_dto.text_cord).rgb,diffuse.a);
    }

  }

  ambient *= attenuation;
  
  diffuse *= attenuation;

  specular *= attenuation;

   vec4 result = ambient + diffuse;
   if(object_ubo.sample_count_less == 0){
      result += specular;
   }
  return (result);

}


vec4 calculate_directional_light(directional_light light,vec3 normal){

      vec3 light_dir = normalize(-light.direction);

      float diffuse_factor = max(dot(normal,light_dir),0.0);
    
      vec4  diffuse_sample = texture(texture_sampler[SAMPLER_DIFF],in_dto.text_cord);

      vec4 ambient =  vec4(vec3 (object_ubo.diffuse_color * in_dto.ambient), diffuse_sample.a);

      vec4 diffuse = vec4(vec3(light.colour * diffuse_factor), diffuse_sample.a);
  
 

    // specular lighting
      vec4 specular_sample;
    if(object_ubo.sample_count_less == 0){
       specular_sample = texture(texture_sampler[SAMPLER_SPEC],in_dto.text_cord);
    }
    vec3 view_direction = normalize(in_dto.view_position - in_dto.frag_position);

    // calculate the half direction 
   vec3 half_direction =  normalize(view_direction - light.direction);

   float specular_factor = pow(max(dot(half_direction,normal),0.0),object_ubo.shineness);

   vec4 specular = vec4(vec3(light.colour * specular_factor),diffuse_sample.a);

   if(in_mode == 0){
       ambient *= diffuse_sample;

      diffuse *= diffuse_sample;

       if(object_ubo.sample_count_less == 0){
               specular *= vec4(specular_sample.rgb,diffuse.a); 
       }  
   }
   vec4 result = ambient + diffuse;
   if(object_ubo.sample_count_less == 0){
             result += specular;
   }
     return (result);    
}

















