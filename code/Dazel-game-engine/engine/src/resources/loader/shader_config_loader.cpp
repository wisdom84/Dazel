#include "shader_config_loader.h"
#include "core/Dmemory.h"
#include "core/Dstrings.h"
#include "core/logger.h"
#include "containers/hashtable.h"
#include "resources/resources.inl"
#include "Math/dlm.h"
#include "platform/file_system.h"
#include "loader_utils.h"

void shader_config_loader_unload(struct resource_loader*self, resource*resources);

void store_string_data(const char*data, char dst[][128]);

 void store_string_data(const char*data, char dst[][128]){
    int index = 0;
    char*p = dst[index];
    if(p){
          while(*p){
            index++;
            p =  dst[index];
            if(!p){
                DERROR(" Array is filled and currently out of bounds: access violation reading memory that has not been allocated");
                return;
           }
       }

       string_copy(p,data);
    }
    else{
        DERROR("invalid array storage for data");
    }
 }

void string_process( char*buffer_type,const char*characters, material_shader_config*config){
    if(string_equal(buffer_type,"uniforms") ||string_equal(buffer_type,"attributes" )|| string_equal(buffer_type,"samplers")){
            int column_index = string_index_of((char*)characters, ':');
            char var_name[64];
            char value[128];
            string_mid(var_name,characters,0,column_index);
            string_mid(value, characters,column_index+1,-1);
            if(string_equal(var_name,"type")){
                    if(string_equal(buffer_type, "attributes")){
                        store_string_data(value,config->attribute_type);  
                    }
                    else if(string_equal(buffer_type, "uniforms")){
                        store_string_data(value,config->uniform_type); 
                    }
            }
            else if(string_equal(var_name,"name")){
                    if(string_equal(buffer_type, "attributes")){
                        store_string_data(value,config->attribute_names);  
                    }
                    else if(string_equal(buffer_type, "uniforms")){
                        store_string_data(value,config->uniform_names); 
                    } 
                    else if(string_equal(buffer_type, "samplers")){
                        store_string_data(value,config->sampler_name); 
                    } 
                    
            }
          else if(string_equal(var_name,"scope")){
                if(string_equal(buffer_type, "uniforms")){
                    store_string_data(value,config->uniform_scope); 
                } 
                else if(string_equal(buffer_type, "samplers")){
                    store_string_data(value,config->sampler_scope); 
                } 
                
            }
        Dzero_memory(var_name,sizeof(char)*64);
        Dzero_memory(value,sizeof(char)*128);
    }
    else{
       store_string_data(characters,config->stage_names);
    }
  
 }
 PFN_string_proccess func = string_process;


 void sub_division(char*buffer_data_type,char*buffer, char symbols[1], char current_sysbol, u64 index, u64 bounds,PFN_string_proccess func,material_shader_config*config){
    char buffer_t [512];
    Dzero_memory(buffer_t, sizeof(char)*512);
    string_copy(buffer_t,buffer);
     char*p=&buffer_t[0];
     char temp_buffer[512];
     memset(temp_buffer,0,sizeof(char)*512); 
     char*t = &temp_buffer[0];
     u64 length = string_length(buffer);
     u64 position = 0;
     while(*p){
        if(*p != current_sysbol){
          *t = *p;
           t++;
        }

        if(*p == current_sysbol || position == length-1){
                index++;
                if(index <= bounds-1){
                    sub_division(buffer_data_type,temp_buffer,symbols,symbols[index],index, bounds, func,config); 
                }
                if(index > bounds-1){  
                    func(buffer_data_type,temp_buffer,config);
                }
                index--;
                memset(temp_buffer,0,sizeof(char)*512);
                t = &temp_buffer[0];  
        }
        p++;
        position++;

     }
     
 };


bool shader_config_loader_load(struct resource_loader*self, const char*name, resource*out_resource){
   if(!self || !name || !out_resource){
        return false;
    }
    const char*format_str = "%s/%s/%s%s";
    char full_file_path[512];
    string_format(full_file_path,format_str, resource_system_base_path(),self->type_path, name,".dst");

    file_handle f;
    if(!filesystem_open(full_file_path, FILE_MODE_READ, false,&f)){
        DERROR("shader_config_laoder load was unable to open file to load shader configuration  data into configuration file");
        return false;
    }
    out_resource->full_path = string_duplicate(full_file_path);

    material_shader_config*resource_data = (material_shader_config*)Dallocate_memory(sizeof(material_shader_config), MEMORY_TAG_SHADER);
    //set some defualt
    resource_data->attribute_count = 0;
    resource_data->uniform_count = 0;
    resource_data->stage_count = 2;
    
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
            string_n_copy(resource_data->shader_name,trimmed_value,SHADER_NAME_MAX_LENGTH);
         }
         else if(string_equal(trimmed_var_name, "stage_count")){
             if(!string_to_u32(trimmed_value,&resource_data->stage_count)){
                DERROR("error converting string to uint32");
             };
         } else if(string_equal(trimmed_var_name, "attribute_count")){
             if(!string_to_u32(trimmed_value,&resource_data->attribute_count)){
                DERROR("error converting string to uint32");
             };
         } else if(string_equal(trimmed_var_name, "uniform_count")){
             if(!string_to_u32(trimmed_value,&resource_data->uniform_count)){
                DERROR("error converting string to uint32");
             };
         }else if(string_equal(trimmed_var_name, "sampler_count")){
             if(!string_to_u32(trimmed_value,&resource_data->sampler_count)){
                DERROR("error converting string to uint32");
             };
         }else if(string_equal(trimmed_var_name, "renderpass")){
              string_n_copy(resource_data->renderpass,trimmed_value,128);
         }else if(string_equal(trimmed_var_name,"attributes") || string_equal(trimmed_var_name, "stage_names") || string_equal(trimmed_var_name,"uniforms") || string_equal(trimmed_var_name,"samplers")){
               char*symbol=",|";
               sub_division(trimmed_var_name,trimmed_value,symbol,symbol[0],0,string_length(symbol),func,resource_data);  
         }else if(string_equal(trimmed_var_name, "use_instance") || string_equal(trimmed_var_name, "use_local")){
               bool val;
               if(!string_to_bool(trimmed_value,&val)){
                DERROR("error in parsing string to bool check string format in config file");
               }
               else{
                      if(string_equal(trimmed_var_name, "use_instance")){
                                resource_data->use_instance = val;
                      }
                    else if(string_equal(trimmed_var_name, "use_local")){
                         resource_data->use_local = val;
                    }
                    
               }
         }
        
     }
    filesystem_close(&f);
    out_resource->name = name;
    out_resource->size_of_data = sizeof(material_config);
    out_resource->data = resource_data;
    return true;
}

void shader_config_loader_unload(struct resource_loader*self, resource*resources){
    resource_unload(self, resources, MEMORY_TAG_MATERIAL_INSTANCE);
}

resource_loader shader_config_loader_create(){
    resource_loader loader;
    loader.type = RESOURCE_TYPE_SHADER;
    loader.custom_type = 0;
    loader.load = shader_config_loader_load;
    loader.unload = shader_config_loader_unload;
    loader.type_path = "Shader_config";
    return loader;
}