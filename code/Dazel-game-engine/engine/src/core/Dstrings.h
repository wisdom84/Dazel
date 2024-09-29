#pragma once 
#include "defines.h"
#include "Math/dlm.h"
using namespace dlm;

EXP char* string_duplicate(const char*str);

EXP u64 string_length(const char*str);

EXP bool string_equal(const char* str0, const char* str1);

EXP void string_format(char*formated_des, const char*format,...);

EXP char* string_copy(char*dst, const char*source);

EXP char* string_n_copy(char*dst, const char*source, int length);

EXP char* string_trim(char*str);

EXP void string_mid(char*dst, const char*source, int start, int length);

EXP int string_index_of(char* str, char c);

EXP bool string_to_vec4(char*str, vec4*out_vec4);


EXP bool string_to_vec3(char*str, vec3*out_vec4);


EXP bool string_to_vec2(char*str, vec2*out_vec4);

EXP bool string_to_float(char*str, float*f);

EXP bool string_to_double(char*str, double*f);

EXP bool string_to_i32 (char*str, int*i);

EXP bool string_to_i64 (char*str, i64*i);

EXP bool string_to_i16 (char*str, i16*i);

EXP bool string_to_u32(char*str, u32*i);

EXP bool string_to_u64(char*str, u64*i);

EXP bool string_to_u16(char*str, u16*i);

EXP bool string_to_u32(char*str, u32*i);

EXP bool string_to_u8(char*str, u8*i);

EXP bool string_to_bool(char* str, bool*b);

EXP void string_empty(char*str);

EXP bool char_isspace(char c);




