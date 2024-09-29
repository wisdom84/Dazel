#include "text_loader.h"
#include "core/Dmemory.h"
#include "core/Dstrings.h"
#include "core/logger.h"
#include "platform/file_system.h"
#include "loader_utils.h"


bool text_loader_load(struct resource_loader*self, const char*name, resource*out_resource){
    if(!self || !name || !out_resource){
       return false;
    }
    const char*format_str = "%s/%s%s";
    const int  required_channel_count = 4;
    char full_file_path[512];
    string_format(full_file_path,format_str, resource_system_base_path(),name,".txt");
    file_handle f;
    if(!filesystem_open(full_file_path,FILE_MODE_READ,false,&f)){
        DERROR("unable to open text file: %s", full_file_path);
        return false;
    }
    u64 size=0;
    if(!filesystem_size(&f,&size)){
        DERROR("could not read file size from file: %s", full_file_path);
        return false;
    };
    char*file_buffer = (char*)Dallocate_memory(sizeof(char)*size,MEMORY_TAG_ARRAY);
    if(!filesystem_read_all_text(&f,file_buffer,&size)){
        DERROR("unable to read text file from file: %s", full_file_path);
        return false;
    };
    out_resource->data = file_buffer;
    out_resource->full_path = string_duplicate(full_file_path);
    out_resource->name = name;
    out_resource->size_of_data = size;
    filesystem_close(&f);
    return true;
}

void text_loader_unload(struct resource_loader*self, resource*resources){
    resource_unload(self, resources, MEMORY_TAG_ARRAY);
}


resource_loader text_resource_loader_create(){
    resource_loader loader;
    loader.load = text_loader_load;
    loader.unload = text_loader_unload;
    loader.type_path = "";
    loader.type = RESOURCE_TYPE_TEXT;
    loader.custom_type = 0;
    return loader;
}