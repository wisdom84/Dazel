#include "transform.h"

 transform transform_create(){
    transform t;
    transform_set_position_rotation_scale(&t,vec3_zero(),mat4_identity(),vec3_one());
    t.local = mat4_identity();
    t.parent = 0;
    return t;
 };

 transform transform_from_position(vec3 position){
   transform t;
   transform_set_position_rotation_scale(&t,position,mat4_identity(), vec3_one());
   t.local = mat4_identity();
   t.parent = 0;
   return t;
 };

 transform transform_from_rotation(mat4 rotation){
   transform t;
   transform_set_position_rotation_scale(&t,vec3_zero(),rotation, vec3_one());
   t.local = mat4_identity();
   t.parent = 0;
   return t;
 };

 transform transform_from_position_rotation(vec3 position, mat4 rotation){
   transform t;
   transform_set_position_rotation_scale(&t,position,rotation, vec3_one());
   t.local = mat4_identity();
   t.parent = 0;
   return t;
 };

 transform transform_from_position_rotation_scale(vec3 position, mat4 rotation, vec3 scale){
   transform t;
   transform_set_position_rotation_scale(&t,position,rotation, scale);
   t.local = mat4_identity();
   t.parent = 0;
   return t;
 };

 transform* transform_get_parent(transform*t){
   if(!t){
      return 0; 
   }
   return t->parent;
 };

 void transform_set_parent(transform*t, transform*parent){
   if(t){
    t->parent = parent;
   }
 };

 vec3 transform_get_position(const transform*t){
   return t->position;
 };

 vec3 transform_set_position(transform*t, vec3 position){
   t->position = position;
   t->is_dirty = true;
 };

 void transform_translate(transform*t, vec3 translation){
      t->position = Add_vec3(t->position,translation);
      t->is_dirty = true;
 };

 mat4 transform_get_rotation(const transform*t){
    return t->rotation;
 };

 void transform_set_rotation(transform*t, mat4 rotation){
     t->rotation = rotation;
     t->is_dirty = true;
 };

 void transform_rotate(transform*t,mat4 rotation){
     t->rotation = mat4_multiply(t->rotation,rotation);
     t->is_dirty = true;
 };

 vec3 transform_get_scale(const transform*t){
   return t->scale;
 };

 void transform_set_scale(transform*t, vec3 scale){
    t->scale = scale;
 };

 void transform_scale(transform*t, vec3 scale){
    t->scale = vec3_multiply(t->scale, scale);
    t->is_dirty = true; 
 };

 void transform_set_position_rotation(transform*t, vec3 position, mat4 rotation){
     t->position = position;
     t->rotation = rotation;
     t->is_dirty = true;
 };

 void transform_set_position_rotation_scale(transform*t, vec3 position, mat4 rotation,vec3 scale){
    t->position = position;
    t->rotation = rotation;
    t->scale = scale;
    t->is_dirty = true;
 };

 void transform_translate_rotate(transform*t, vec3 translation, mat4 rotation){
     t->position = Add_vec3(t->position, translation);
     t->rotation = mat4_multiply(t->rotation, rotation);
     t->is_dirty = true;
 }; 

 mat4 transform_get_local(transform*t){
    if(t){
        if(t->is_dirty){
            mat4 tr = mat4_multiply(t->rotation, mat4_homogeneous_translation(t->position.x,t->position.y,t->position.z));
            tr = mat4_multiply(mat4_homogenous_scale(t->scale.x,t->scale.y,t->scale.z,1.0f),tr);
            t->local = tr;
            t->is_dirty = false;
        }
        return t->local;
    }
    return mat4_identity();
 };

 mat4 transform_get_world(transform*t){
   if(t){
      mat4 l = transform_get_local(t);
      if(t->parent){
        mat4 p = transform_get_world(t->parent);
        return mat4_multiply(l,p);
      }
      return l;
   }
   return mat4_identity();
 };