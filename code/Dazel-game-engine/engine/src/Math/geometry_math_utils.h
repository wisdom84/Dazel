#pragma once 
#include "defines.h"
#include "Math/dlm.h"
using namespace dlm;
void geometry_generate_tangent(u32 vertext_count, Vertex_3d*verticies, u32 index_count, u32*indicies);
 void geometry_generate_normals(u32 vertext_count, Vertex_3d*verticies, u32 index_count, u32*indicies);