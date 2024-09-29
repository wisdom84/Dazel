#include "defines.h"
#include "loader_utils.h"

void resource_unload(struct resource_loader*self, resource*resources, memory_tag tag){
    if(!self || !resources){
        DWARNING("resource loader unload is called with an invalid loader or resource");
        return;
    }

    u32 path_length = string_length(resources->full_path);
    if(path_length){
        Dfree_memory(resources->full_path, MEMORY_TAG_STRING, sizeof(char)*path_length+1);
    }
    if(resources->data){
        Dfree_memory(resources->data, tag, resources->size_of_data);
        resources->data = nullptr;
        resources->size_of_data = 0;
        resources->loader_id = INVALID_ID;
    }
}