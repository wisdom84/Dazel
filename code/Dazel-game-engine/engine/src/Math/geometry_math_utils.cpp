#include "geometry_math_utils.h"
#include "core/Dmemory.h"
#include "core/logger.h"
void geometry_generate_tangent(u32 vertext_count, Vertex_3d*verticies, u32 index_count, u32*indicies){
    for(u32 i=0; i < index_count; i +=3){
        u32 i0 = indicies[i + 0];
        u32 i1 = indicies[i + 1];
        u32 i2 = indicies[i + 2];
        
        vec3 edge1 = Subtraction_vec3(verticies[i1].position, verticies[i0].position);
        vec3 edge2 = Subtraction_vec3(verticies[i2].position, verticies[i0].position);

        float deltaU1 = verticies[i1].texture_cord.x - verticies[i0].texture_cord.x;
        float deltaV1 = verticies[i1].texture_cord.y - verticies[i0].texture_cord.y;

        float deltaU2 = verticies[i2].texture_cord.x - verticies[i0].texture_cord.x;
        float deltaV2 = verticies[i2].texture_cord.y - verticies[i0].texture_cord.y;

        float  dividend = (deltaU1 * deltaV2 - deltaU2 * deltaV1);
        float fc = 1.0f/dividend;
        vec3 tangent = (vec3){
            (fc *(deltaV2 * edge1.x - deltaV1 * edge2.x)),
            (fc *(deltaV2 * edge1.y - deltaV1 * edge2.y)),
            (fc *(deltaV2 * edge1.z - deltaV1 * edge2.z))
        };
        tangent = vec3_nomalize(tangent);

        float sx = deltaU1, sy = deltaU2;
        float tx = deltaV1, ty = deltaV2;
        float handedness = ((tx * sy - ty *sx) < 0.0f) ? -1.0f : 1.0f;
        vec4 t4 = vec3_to_vec4(tangent, handedness);
        verticies[i0].tangent = t4;
        verticies[i1].tangent = t4;
        verticies[i2].tangent = t4;
    }
}
 void geometry_generate_normals(u32 vertext_count, Vertex_3d*verticies, u32 index_count, u32*indicies){
      for(u32 i=0; i < index_count; i +=3){
        u32 i0 = indicies[i + 0];
        u32 i1 = indicies[i + 1];
        u32 i2 = indicies[i + 2];

        vec3 edge1 = Subtraction_vec3(verticies[i1].position, verticies[i0].position);
        vec3 edge2 = Subtraction_vec3(verticies[i2].position, verticies[i0].position);

        vec3 normal = vec3_nomalize(Cross_prod_vec3(edge1, edge2));

        verticies[i0].normal = normal;
        verticies[i1].normal = normal;
        verticies[i2].normal = normal;

      }  
 }

bool vertex3d_equal(Vertex_3d vertex_0, Vertex_3d vertex_1){
   return (
            vec3_compare(vertex_0.position, vertex_1.position,EPSILON)&&
            vec3_compare(vertex_0.normal, vertex_1.normal,EPSILON)&&
            vec2_compare(vertex_0.texture_cord, vertex_1.texture_cord, EPSILON)&&
            vec4_compare(vertex_0.tangent, vertex_1.tangent,EPSILON)
         );
};

void reassign_index(u32 index_count, u32*indicies, u32 from, u32 to){
    for(u32 i=0; i<index_count; i++){
       if(indicies[i] == from){
        indicies[i] = to;
       }else if(indicies[i] > from){
         indicies[i]--;            
       }
    }
}

void geometry_deduplicate_verticies(u32 vertex_count, Vertex_3d*verticies, u32 index_count, u32*indicies,u32* out_vertext_count, Vertex_3d**out_verticies){
    Vertex_3d*unique_verts =(Vertex_3d*)Dallocate_memory(sizeof(Vertex_3d)*vertex_count, MEMORY_TAG_ARRAY);
    *out_vertext_count = 0;
    u32 found_count = 0;
    for(u32 v= 0; v < vertex_count; v++){
        bool found = false;
        for(u32 u = 0; u < *out_vertext_count; u++){
            if(vertex3d_equal(verticies[v],unique_verts[u])){
                 reassign_index(index_count,indicies,v - found_count,u); 
                 found = true;
                 found_count++;
                 break; 
            }
        }
        if(!found){
            unique_verts[*out_vertext_count] = verticies[v];
            (*out_vertext_count)++;
        }
    }
    *out_verticies =(Vertex_3d*)Dallocate_memory(sizeof(Vertex_3d) * (*out_vertext_count), MEMORY_TAG_ARRAY);
    Dmemory_copy(*out_verticies,unique_verts,sizeof(Vertex_3d)*(*out_vertext_count));
    Dfree_memory(unique_verts,MEMORY_TAG_ARRAY,sizeof(Vertex_3d)*vertex_count);
    u32 removed_count = vertex_count - *out_vertext_count;
    DDEBUG("geometry_deduplicate_verticies: removed %d verticies, orig/now %d/%d",removed_count,vertex_count,*out_vertext_count);

}