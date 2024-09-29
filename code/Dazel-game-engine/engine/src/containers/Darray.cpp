#include "Darray.h"
#include "core/Dmemory.h"
#include <core/logger.h>
#include <iostream>

 void* _d_array_create(u64 length, u64 stride){
    u64 header_size = DARRAY_FIELD_LENGTH * sizeof(u64);// 24 bytes of memory;
    u64 array_size = length * stride;
    u64 * new_array = (u64*)Dallocate_memory(header_size+array_size,MEMORY_TAG_ARRAY);
    Dmemory_set(header_size+array_size, new_array, 0);
    new_array[DARRAY_CAPACITY]=length;
    new_array[DARRAY_LENGTH]=0;
    new_array[DARRAY_STRIDE]= stride;
    return (void*)(new_array+DARRAY_FIELD_LENGTH);
}


void _d_destroy_array(void*array){
    u64* header = (u64*)array - DARRAY_FIELD_LENGTH;
    u64 header_szie =DARRAY_FIELD_LENGTH * sizeof(u64);
    u64 total_size = header_szie + (header[DARRAY_STRIDE] * header[DARRAY_CAPACITY]);
    Dfree_memory(header,MEMORY_TAG_ARRAY,total_size);
}

u64 _d_array_field_get(void*array, u64 field){
    u64*header = (u64*)array-DARRAY_FIELD_LENGTH;
    return header[field];
}

void _d_array_field_set(void* array, u64 field,u64 value){
     u64*header = (u64*)array-DARRAY_FIELD_LENGTH;
     header[field]=value;
}
 u64 darray_length(void*array){
   u64*header = (u64*)array-DARRAY_FIELD_LENGTH;
   return header[DARRAY_LENGTH];
};
 u64 darray_stride(void* array){
    u64*header = (u64*)array-DARRAY_FIELD_LENGTH;
   return header[DARRAY_STRIDE];
};
 u64 darray_capacity(void* array){
    u64*header = (u64*)array-DARRAY_FIELD_LENGTH;
   return header[DARRAY_CAPACITY];
};

 void* _d_array_reszie(void* array){
    u64 length= darray_length(array);
    u64 stride = darray_stride(array);
    u64 capacity =darray_capacity(array); 
    void* new_array = _d_array_create( DARRY_RESIZE_FACTOR * capacity, stride);
    Dmemory_copy(new_array,array,length*stride);
    _d_array_field_set(new_array,DARRAY_LENGTH, length);
    _d_destroy_array(array);
    return new_array;
}
void* _d_array_push( void*array,   void* value_ptr){
    u64 length = darray_length(array);
    u64 stride = darray_stride(array);
    if(length >= darray_capacity(array)){
        array = _d_array_reszie(array); // storing a new address with copied data
    }
    u64 offset = length *stride;
    char* address= (char*)array + offset;
//    int value;
//    int*data=&value;
//    int num=4; 
    Dmemory_copy(address, value_ptr, stride);
    _d_array_field_set(array, DARRAY_LENGTH,length+1);
    return array;
}
void _d_array_pop(void*array, void*des){
       u64 length = darray_length(array);
       u64 stride = darray_stride(array);
    if( length==0){
        DWARNING("Array empty");
        return;
       }
     char * address=(char*)array+((length-1)*stride);
     Dmemory_copy(des, address, stride);
      _d_array_field_set(array, DARRAY_LENGTH, length-1);
}

void  _d_array_pop_at(void*array, u64 index, void*des){
       u64 length = darray_length(array);
       u64 stride = darray_stride(array);
       if(index >= length){
        DERROR("index is outside the bounds of this array! Length: %d, index: %d", length, index); 
        return;
       }
       u64 offset = index *stride;
       char*address = (char*)array+offset;
       Dmemory_copy(des,address, stride);
       if(index != length-1){
        // we arrange the data in the array
        char*addr_source =(char*)array+((index+1)*stride);
        Dmemory_copy( address,addr_source, stride*(length-(index+1)));
       }
       _d_array_field_set(array, DARRAY_LENGTH, length-1);
}

 void* _d_array_insert_at(void*array, u64 index,  void*value_ptr){
           u64 length = darray_length(array);
           u64 stride = darray_stride(array); 
           if(index >= length){
            DERROR("index is outside the bounds of this array! Length: %d, index: %d", length,index);
            return array; 
           }
           if(length >= darray_capacity(array)){
            array = _d_array_reszie(array);
           }
         if(index != length){
            // shifting the array outward 
            char * address = (char*)array+((index+1)*stride);
            char * source =(char*)array+(index*stride);
          Dmemory_copy(address, source, (length-index)*stride);
         }
         //setting the value
         char * address = (char*)array+(index*stride);
         Dmemory_copy(address,value_ptr, stride);
         _d_array_field_set(array, DARRAY_LENGTH , length+1);
         return array;
}
u64 _d_array_get_size(void*array){
    return darray_length(array)*darray_stride(array);
}






