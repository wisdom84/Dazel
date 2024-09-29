#pragma once

#include "defines.h"


typedef struct dynamic_allocator {
    void* memory;
} dynamic_allocator;


EXP bool dynamic_allocator_create(u64 total_size, u64* memory_requirement, void* memory, dynamic_allocator* out_allocator);


EXP bool dynamic_allocator_destroy(dynamic_allocator* allocator);

EXP void* dynamic_allocator_allocate(dynamic_allocator* allocator, u64 size);


// EXP void* dynamic_allocator_allocate_aligned(dynamic_allocator* allocator, u64 size, u16 alignment);


EXP bool dynamic_allocator_free(dynamic_allocator* allocator, void* block, u64 size);


// EXP bool dynamic_allocator_free_aligned(dynamic_allocator* allocator, void* block);


// EXP bool dynamic_allocator_get_size_alignment(void* block, u64* out_size, u16* out_alignment);


EXP u64 dynamic_allocator_free_space(dynamic_allocator* allocator);


// EXP u64 dynamic_allocator_total_space(dynamic_allocator* allocator);

// EXP u64 dynamic_allocator_header_size(void);