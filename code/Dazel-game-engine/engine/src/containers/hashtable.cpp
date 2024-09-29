#include "hashtable.h"
#include "core/Dmemory.h"
#include "core/logger.h"

// algorithm that creates a hash value(a hash value is a random index in a array of elements )
u64 hash_name(const char* name, u32 element_count){
    static const u64 multiplier = 97;
    u64 hash = 0;
    const unsigned char*us;
    for(us = (const unsigned char*)name; *us; us++){
      hash = hash * multiplier + *us;    
    }
    hash %= element_count;
    return hash;
}

void hash_table_create(u64 element_size, u64 element_count, void*memory, bool is_pointer_type, hashtable*out_hashtable){
    if(!memory || !out_hashtable){
        DERROR("hash table creation failed the hash table requires a valid pointer to the memory and out hash table");
        return;
    }
    if(!element_count || !element_size){
        DERROR("element count and the element size has to be a valid count and valid size");
        return;
    }
    out_hashtable->memory = memory;
    out_hashtable->element_count = element_count;
    out_hashtable->element_size = element_size;
    out_hashtable->is_pointer_type = is_pointer_type;
    Dzero_memory(out_hashtable->memory, element_size*element_count);
}

 void hash_table_destroy(hashtable*table){
    if(table){
        Dzero_memory(table, sizeof(hashtable));
    }
 };

 bool hash_table_set(hashtable*table, const char*name, void*value){
    if(!table || !name || !value){
        DERROR("hash table requires a valid name, table and value to exist");
    };
    if(table->is_pointer_type){
        DERROR("hashtable_set shoud not be used with tables that have pointer types instead use hashtable_set_ptr");
        return false;
    }
    u64 hash = hash_name(name, table->element_count);
    Dmemory_copy((char*)table->memory+(table->element_size * hash), value, table->element_size);
    return true;
 }
 
 bool hash_table_set_ptr(hashtable * table, const char*name, void**value){
    if(!table || !name || !value){
        DERROR("hash table requiress a valid name, table and value to exist");
        return false;
    }
    if(!table->is_pointer_type){
        DERROR("hashtable_set_ptr should not be used with tables that dont have a pointer type use hashtable_set instead");
        return false;
    }
    u64 hash = hash_name(name, table->element_count); //  random index  in the boundry of the element_count of the table
    ((void**)table->memory)[hash] = value ? value : 0; // the ternary operation checks if the value is a valid pointer if not is sets 0 at that index 
    return true;
 }

 bool hash_table_get(hashtable*table, const char*name, void*out_value){
     if(!table || !name || !out_value){
        DERROR("hash table requiress a valid name, table and out_value to exist");
        return false;
    } 
   if(table->is_pointer_type){
        DERROR("hashtable_get should not be used with tables that  have a pointer type use hashtable_get_ptr instead");
        return false;
    }
   u64 hash = hash_name(name, table->element_count);
   Dmemory_copy(out_value, ((char*)table->memory + (table->element_size * hash)), table->element_size);
   return true;
 };
  bool hash_table_get_ptr(hashtable * table, const char*name, void**out_value){
    if(!table || !name || !out_value){
        DERROR("hash table requiress a valid name, table and value to exist");
        return false;
    }
    if(!table->is_pointer_type){
        DERROR("hashtable_get_ptr should not be used with tables that dont have a pointer type use hashtable_get instead");
        return false;
    }
    u64 hash = hash_name(name, table->element_count); //  random index  in the boundry of the element_count of the table
    *out_value =((void**)table->memory)[hash];
    // the ternary operation checks if the value is a valid pointer if not is sets 0 at that index 
    return *out_value != nullptr;  // if the address is valid we retuern true if not we return false 
 }

 bool hash_table_fill(hashtable*table, void*value){
    if(!table || !value){
          DERROR("hash table requiress a valid  table and value to exist");
          return false;
    }
    if(table->is_pointer_type){
         DERROR("hashtable_fill should not be used with tables that have pointer types");    
    }
    for(u32 i=0; i < table->element_count; i++){
        Dmemory_copy((char*)table->memory+(table->element_size *i), value, table->element_size);
    }
    return true;
 };




