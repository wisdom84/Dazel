#pragma once
// #include "renderer_types.inl"
#include "defines.h"
struct platform_state;

bool create_renderer_backend(u16 type, struct platform_state*plat_state,struct renderer_backend*out_renderer_backend);

void renderer_backend_destroy(struct renderer_backend* renderer_backend);