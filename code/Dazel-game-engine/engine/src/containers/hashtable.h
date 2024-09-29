#pragma once
#include "defines.h"

typedef struct hashtable {
    u64 element_size;
    u64 element_count;
    bool is_pointer_type;
    void* memory;
}hashtable;

EXP void hash_table_create(u64 element_size, u64 element_count, void*memory, bool is_pointer_type, hashtable*out_hashtable);
EXP void hash_table_destroy(hashtable*table);
EXP bool hash_table_set(hashtable*table, const char*name, void*value);
EXP bool hash_table_set_ptr(hashtable * table, const char*name, void**value);
EXP bool hash_table_get(hashtable*table, const char*name, void*out_value);
EXP bool hash_table_get_ptr(hashtable*table, const char*name, void**out_value);
EXP bool hash_table_fill(hashtable*table, void*value);




