#pragma once
#include "defines.h"

enum memory_tag{
 MEMORY_TAG_UNKNOWN,
 MEMORY_TAG_ARRAY,
 MEMORY_TAG_LINEAR_ALLOCATOR,
 MEMORY_TAG_DICT,
 MEMORY_TAG_RING_QUEUE,
 MEMORY_TAG_BST,
 MEMORY_TAG_STRING,
 MEMORY_TAG_APPLICATION,
 MEMORY_TAG_JOB,
 MEMORY_TAG_TEXTURE,
 MEMORY_TAG_MATERIAL_INSTANCE,
 MEMORY_TAG_RENDERER,
 MEMORY_TAG_GAME,
 MEMORY_TAG_TRANSFORM,
 MEMORY_TAG_ENTITY,
 MEMORY_TAG_ENTITY_NODE,
 MEMORY_TAG_SCENE,
 MEMORY_TAG_SHADER,
 MEMORY_TAG_MAX_TAGS

};
EXP void intialize_memory();
EXP void shutdown_memory();

EXP void* Dallocate_memory(u64 size, memory_tag tag);
EXP void*Dallocate_aligned_memory(u64 size, u64 aligned_szie, memory_tag tag);
EXP void Dfree_aligned_memory(void*block,memory_tag tag, u64 size);
EXP void Dfree_memory(void*block, memory_tag tag, u64 size);
EXP void*Dzero_memory(void*block, u64 size);
EXP void*Dmemory_set(u64 size, void*des, int value);
EXP void*Dmemory_copy(void*des, const void*source, u64 size);

EXP char* get_memory_usage();

EXP u64 get_memory_alloc_count();
