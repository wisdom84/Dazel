#pragma once 
#include "defines.h"
#include "Math/dlm.h"
using namespace dlm;

typedef struct transform{
 vec3 position;
 mat4 rotation;
 vec3 scale;
 bool is_dirty;
 mat4 local;
 struct transform* parent;
}transform;

EXP transform transform_create();

EXP transform transform_from_position(vec3 position);

EXP transform transform_from_rotation(mat4 rotation);

EXP transform transform_from_position_rotation(vec3 position, mat4 rotation);

EXP transform transform_from_position_rotation_scale(vec3 position, mat4 rotation, vec3 scale);

EXP transform* transform_get_parent(transform*t);

EXP void transform_set_parent(transform*t, transform*parent);

EXP vec3 transform_get_position(const transform*t);

EXP vec3 transform_set_position(transform*t, vec3 position);

EXP void transform_translate(transform*t, vec3 translation);

EXP mat4 transform_get_rotation(const transform*t);

EXP void transform_set_rotation(transform*t, mat4 rotation);

EXP void transform_rotate(transform*t,mat4 rotation);

EXP vec3 transform_get_scale(const transform*t);

EXP void transform_set_scale(transform*t, vec3 scale);

EXP void transform_scale(transform*t, vec3 scale);

EXP void transform_set_position_rotation(transform*t, vec3 position, mat4 rotation);

EXP void transform_set_position_rotation_scale(transform*t, vec3 position, mat4 rotation,vec3 scale);

EXP void transform_translate_rotate(transform*t, vec3 translation, mat4 rotation);

EXP mat4 transform_get_local(transform*t);

EXP mat4 transform_get_world(transform*t);





