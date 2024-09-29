#pragma once
#include <typeinfo>
#include "defines.h"

#define DARRY_RESIZE_FACTOR 2

#define DARRY_DEFUALT_CAPACITY 1

enum{
    DARRAY_CAPACITY,
    DARRAY_LENGTH,
    DARRAY_STRIDE,
    DARRAY_FIELD_LENGTH
};

EXP void* _d_array_create(u64 length, u64 stride);

EXP void _d_destroy_array(void*array);

EXP void* _d_array_reszie(void* array);

EXP  void* _d_array_push(void*array,    void* value_ptr); 

EXP void _d_array_pop(void*array, void*des);

EXP void  _d_array_pop_at(void*array, u64 index, void*des);

EXP void* _d_array_insert_at(void*array, u64 index,  void*value_ptr);

EXP u64 _d_array_field_get(void* array, u64 field);

EXP void _d_array_field_set(void* array, u64 field,u64 value);

EXP u64 _d_array_get_size(void*array);

 u64 darray_length(void* array);

 u64 darray_stride(void* array);

 u64 darray_capacity(void* array);

#define darray_create(type)             ( (type*)_d_array_create(DARRY_DEFUALT_CAPACITY, sizeof(type)) )

#define darray_reservered(capacity,type) ( (type*)_d_array_create(capacity, sizeof(type)) )
 
#define darray_destroy_array(array)              _d_destroy_array(array)

#define darray_resize(array,type)                    { array = (type*)_d_array_reszie(array);}

#define darray_push(array, value, type)       ( {auto temp=value; array = (type*)_d_array_push(array, &temp);} )

#define darray_pop(array, des)                   _d_array_pop(array, des)

#define darray_pop_at(array, index, des)         _d_array_pop_at( array, index, des)

#define darray_insert_at(array, index, value, type)  ( { auto temp=value;  array = (type*)_d_array_insert_at(array, index, &temp);} ) 

#define darray_get_length(array)   _d_array_field_get(array, DARRAY_LENGTH) 

#define darray_get_capacity(array)   _d_array_field_get(array, DARRAY_CAPACITY) 

#define darray_get_stride(array)   _d_array_field_get(array, DARRAY_STRIDE)

#define darray_set_length(array, value)    _d_array_field_set(array, DARRAY_LENGTH,value)

#define darray_size(array)  _d_array_get_size(array)

#define darray_clear(array)         _d_array_field_set(array,DARRAY_LENGTH,0);
      



