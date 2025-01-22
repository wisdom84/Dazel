#pragma once 
#include "defines.h"
#include "Math/dlm.h"
using namespace dlm;
void geometry_generate_tangent(u32 vertext_count, Vertex_3d*verticies, u32 index_count, u32*indicies);
void geometry_generate_normals(u32 vertext_count, Vertex_3d*verticies, u32 index_count, u32*indicies);
void geometry_deduplicate_verticies(u32 vertex_count, Vertex_3d*verticies, u32 index_count, u32*indicies,u32* out_vertext_count, Vertex_3d**out_verticies);
bool vertex3d_equal(Vertex_3d vertex_0, Vertex_3d vertex_1);
void reassign_index(u32 index_count, u32*indicies, u32 from, u32 to);
void geometry_deduplicate_verticies(u32 vertex_count, Vertex_3d*verticies, u32 index_count, u32*indicies,u32* out_vertext_count, Vertex_3d**out_verticies);