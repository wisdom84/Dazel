#version 450 
layout (location = 0) out vec4 outcolor;
// layout (location = 0) in int out_mode;

layout (set = 1,binding = 0) uniform local_uniform_object{
  vec4 diffuse_color;
  // vec4 shineness;
  // vec4 apply_material;
  // vec4 point_light_details;
}object_ubo;

// struct directional_light {
//   vec3 direction;
//   vec4 colour;
// };
// directional_light dir_light = {
//   vec3(-0.57735, -5.57735, 5.7735),
//   vec4 (0.8,0.8,0.8,1.0)
// };
// const int SAMPLER_DIFF = 0;
// const int SAMPLER_SPEC = 1;
layout (set = 1, binding = 1) uniform sampler2D texture_sampler;

layout (location = 1) in struct dto {
   vec2 text_cord;
  //  vec4 ambient;
  //  vec3 normal_dir;
  //  vec3 frag_position;
  //  vec3 view_position;
}in_dto;

// vec4 calculate_directional_light(directional_light light, vec3 normal, vec3 view_direction);

void main(){
//calculate_directional_light(dir_light, in_dto.normal_dir, view_direction)
  // vec3 view_direction = normalize(in_dto.view_position - in_dto.frag_position);
  outcolor = texture(texture_sampler,in_dto.text_cord); 
}

// vec4 calculate_directional_light(directional_light light, vec3 normal,  vec3 view_direction){
//   //  float diffuse_factor = max(dot(normal,-light.direction),0.0);// the max function  determines the maximum number between what is returned from the dot product and zero in this case if the value returned by the dot product is negative we use zero as the diffuse factor
   
//   // vec3 half_direction = normalize(view_direction - light.direction);

//   // float specular_factor = pow(max(dot(half_direction,normal),0.0),object_ubo.shineness.x);
//   // float specular_strength = 0.0000005;
//   // vec4 diff_samp = texture(texture_sampler[SAMPLER_DIFF],in_dto.text_cord);
//   // vec4 ambient = vec4(vec3(in_dto.ambient * light.colour * object_ubo.diffuse_color),diff_samp.a);
//   // vec4 diffuse = vec4(vec3(light.colour * diffuse_factor), diff_samp.a);
//   // vec4 specular = vec4(vec3( specular_factor*light.colour),diff_samp.a);
//   // diffuse *=  diff_samp;
//   // ambient *= diff_samp;
//   // specular *= vec4(texture(texture_sampler[SAMPLER_SPEC],in_dto.text_cord).rgb,diffuse.a);

//   vec4 diff_samp = texture(texture_sampler[SAMPLER_DIFF],in_dto.text_cord);

//   vec4 ambient = vec4(vec3(in_dto.ambient * light.colour),diff_samp.a);

//   vec3 Normal = normalize(normal);
//   vec3 lightDir = normalize(light.direction - in_dto.frag_position);

//   float diffuse_factor = max(dot(Normal,lightDir),0.0);
  
//   vec4 diffuse = vec4(vec3(diffuse_factor * light.colour ), diff_samp.a);
//   diffuse *= diff_samp;
//   ambient *= diff_samp;
//   vec3 reflectDir = reflect(-lightDir,Normal);
//   float specular_strength = 0.5;
//   float specular_factor = pow(max(dot(view_direction, reflectDir),0.0),object_ubo.shineness.x);
//   vec4 specular =vec4( specular_strength * vec3( specular_factor* light.colour),diff_samp.a) * vec4(texture(texture_sampler[SAMPLER_SPEC],in_dto.text_cord).rgb,diffuse.a); 
//   // vec4 object_color = vec4(1.0,0.5,0.31,1.0);
//   vec4 result = (ambient + diffuse + specular);

//   return(result);
// }
