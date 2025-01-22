#include "image_loader.h"
#include "loader_utils.h"
#include "core/Dmemory.h"
#include "platform/file_system.h"
#include "core/Dstrings.h"
#include "core/logger.h"
#include "resources/resources.inl"

#define STB_IMAGE_IMPLEMENTATION
#include "vendor/stb_image.h"

bool image_loader_load(struct resource_loader*self, const char*name, resource*out_resource){
    if(!self || !name || !out_resource){
        return false;
    }
    const char*format_str = "%s/%s/%s%s";
    const int  required_channel_count = 4;
    stbi_set_flip_vertically_on_load(true);
    char full_file_path[512];
    #define IMAGE_EXTENSION_COUNT 3
    bool found = false;
    char*extension[IMAGE_EXTENSION_COUNT] = {".tga",".png",".jpg"};
    for(u32 i =0; i < IMAGE_EXTENSION_COUNT; i++){
        string_format(full_file_path,format_str, resource_system_base_path(),self->type_path, name,extension[i]);
        if(filesystem_exists(full_file_path)){
            found = true;
            break;
        }
    }
    if(!found){
        DERROR("image resource loader failed to find file '%s' or with any supported extension ",full_file_path);
        return false;
    }
    // string_format(full_file_path,format_str, resource_system_base_path(),self->type_path, name,".png");


    int width, height, channel_count;
    u8*data=stbi_load(full_file_path,&width,&height,&channel_count,required_channel_count);
    const char* fail_message = stbi_failure_reason();
    // if(fail_message){
    //     DERROR("image resource loader failed to load file %s:%s", full_file_path, fail_message);

    //     stbi__err(0,0);

    //     if(data){
    //         stbi_image_free(data);
    //     }
    //     return false;
    // }
    if(!data){
        DERROR("image resource loader failed to load file %s", full_file_path);
        return false;
    }
    image_resource_data*resource_data = (image_resource_data*)Dallocate_memory(sizeof(image_resource_data), MEMORY_TAG_TEXTURE);
    resource_data->channel_count = required_channel_count;
    resource_data->height = height;
    resource_data->width = width;
    resource_data->pixels = data;
    out_resource->data = resource_data;
    out_resource->full_path = string_duplicate(full_file_path);
    out_resource->name = name;
    out_resource->size_of_data = sizeof(image_resource_data);

    return true;
};

void image_loader_unload(struct resource_loader*self, resource*resources){
    resource_unload(self, resources, MEMORY_TAG_TEXTURE);
}
resource_loader image_resource_loader_create(){
    resource_loader loader;
    loader.type = RESOURCE_TYPE_IMAGE;
    loader.custom_type = 0;
    loader.load = image_loader_load;
    loader.unload = image_loader_unload;
    loader.type_path = "textures";
    return loader;
}