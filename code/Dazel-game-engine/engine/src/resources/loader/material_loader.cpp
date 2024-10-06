#include "material_loader.h"
#include "core/Dmemory.h"
#include "core/Dstrings.h"
#include "core/logger.h"
#include "resources/resources.inl"
#include "Math/dlm.h"
#include "platform/file_system.h"
#include "loader_utils.h"

bool material_loader_load(struct resource_loader*self, const char*name, resource*out_resource){
   if(!self || !name || !out_resource){
        return false;
    }
    const char*format_str = "%s/%s/%s%s";
    const int  required_channel_count = 4;
    char full_file_path[512];
    string_format(full_file_path,format_str, resource_system_base_path(),self->type_path, name,".dmt");



    file_handle f;
    if(!filesystem_open(full_file_path, FILE_MODE_READ, false,&f)){
        DERROR("material_laoder load was unable to open file to load material data into configuration file");
        return false;
    }
    out_resource->full_path = string_duplicate(full_file_path);

    material_config* resource_data =(material_config*)Dallocate_memory(sizeof(material_config), MEMORY_TAG_MATERIAL_INSTANCE);
    // set some default 
    resource_data->type = MATERIAL_TYPE_WORLD;
    resource_data->auto_release = true;
    resource_data->diffuse_color = vec4_create(1.0f,1.0f,1.0f,1.0f);
    resource_data->diffuse_map_name[0] = '\0';
    string_n_copy(resource_data->name, name, MATERIAL_NAME_MAX_LENGTH);

    char line_buff[512] = "";
    char*p = &line_buff[0];
    u64  line_length = 0;
    u32 line_number = 1;
    while(filesystem_read_line(&f,511, &p, &line_length)){
        // we trimm the string to remove leading and trailing white spaces 
        char*trimmed = string_trim(line_buff);
        //geting the length of the trimmed string 
        line_length = string_length(trimmed);
        
        // skip the blank lines and comments 
         if(line_length < 1 || trimmed[0] == '#'){
            line_number++;
            Dzero_memory(line_buff, sizeof(char)*512);
            continue;
         }

         // split into variables 
         int equal_sign_index = string_index_of(trimmed, '=');
         if(equal_sign_index == -1){
            DWARNING("potential formatting issue found in file %s : '=' token not found  skipping line %hu.", full_file_path, line_number);
            line_number++;
            continue;
         }

         // Assume a max of 64 characters for the variable name 
         char raw_var_name[64];
         Dzero_memory(raw_var_name, sizeof(char)*64);
         string_mid(raw_var_name, trimmed, 0, equal_sign_index);
         char*trimmed_var_name = string_trim(raw_var_name);

         // Assume the max of 511-65 (446) for the length of the value
         char raw_value[446];
         Dzero_memory(raw_value, sizeof(char)*446);
         string_mid(raw_value, trimmed, equal_sign_index+1,-1);// read the rest of the line 
         char*trimmed_value = string_trim(raw_value);


         // processing the strings into the struct 
         if(string_equal(trimmed_var_name, "version")){
            // TODO:
         }else if(string_equal(trimmed_var_name, "name")){
            string_n_copy(resource_data->name,trimmed_value,MATERIAL_NAME_MAX_LENGTH);
         }
         else if(string_equal(trimmed_var_name, "diffuse_map_name")){
            string_n_copy(resource_data->diffuse_map_name,trimmed_value,TEXTURE_NAME_MAX_LENGTH);
         }else if(string_equal(trimmed_var_name, "specular_map_name")){
            string_n_copy(resource_data->specular_map_name,trimmed_value,TEXTURE_NAME_MAX_LENGTH);
         }else if(string_equal(trimmed_var_name, "normal_map_name")){
            string_n_copy(resource_data->normal_map_name,trimmed_value,TEXTURE_NAME_MAX_LENGTH);
         }
         else if(string_equal(trimmed_var_name, "diffuse_color")){
                if(!string_to_vec4(trimmed_value, &resource_data->diffuse_color)){
                    DERROR("error parsing string to vec4  for diffuse color in file %s at line %hu using the defualt diffuse color ", full_file_path, line_number);
                   resource_data->diffuse_color = vec4_create(1.0f,1.0f,1.0f,1.0f);
                }
         }else if(string_equal(trimmed_var_name, "shineness")){
                if(!string_to_float(trimmed_value,&resource_data->shineness)){
                    DERROR("error parsing string to float  for shineness in file %s at line %hu shineness ", full_file_path, line_number);
                   resource_data->shineness = 32.0f;
                }
         }
         else if(string_equal(trimmed_var_name, "type")){
            if(string_equal(trimmed_value, "UI")){
                resource_data->type = MATERIAL_TYPE_UI;
            }
            else if(string_equal(trimmed_value, "light")){
               resource_data->type = MATERIAL_TYPE_LIGHT;
            }
         }
         else if(string_equal(trimmed_var_name, "shader_name")){
            string_n_copy(resource_data->shader_name,trimmed_value,SHADER_NAME_MAX_LENGTH);
         }  
         Dzero_memory(line_buff, sizeof(char)*512);
         line_number++;
    }
    filesystem_close(&f);
    out_resource->name = name;
    out_resource->size_of_data = sizeof(material_config);
    out_resource->data = resource_data;
    return true;
}

void material_loader_unload(struct resource_loader*self, resource*resources){
    resource_unload(self, resources, MEMORY_TAG_MATERIAL_INSTANCE);
}

resource_loader material_loader_create(){
    resource_loader loader;
    loader.type = RESOURCE_TYPE_MATERIAL;
    loader.custom_type = 0;
    loader.load = material_loader_load;
    loader.unload = material_loader_unload;
    loader.type_path = "materials";
    return loader;
}
