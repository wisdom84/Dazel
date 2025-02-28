#include "Dstrings.h"
#include "Dmemory.h"
#include "core/logger.h"
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
u64 string_length(const char*str){
    return strlen(str);
}
 char* string_duplicate(const char*str){
    u64 length = string_length(str);
    char*copy = (char*)Dallocate_memory(length+1, MEMORY_TAG_STRING);
    Dmemory_copy(copy, str, length+1);
    return copy;
 }
 bool string_equal(const char* str0, const char* str1){
   if(strcmp(str0, str1) == 0){
    return true;
   }
   return false;
 }

bool string_nequal(const char* str0, const char*str1, u64 length){
  return strncmp(str0,str1,length);
};

bool string_nequali(const char* str0, const char*str1, u64 length){
#if defined(__GNUC__)
    return strncasecmp(str0,str1,length) == 0;
#elif (defined _MSC_VER)
     return _strnicmp(str0,str1,length) == 0;
#endif         
};
 
void string_format(char*formated_des, const char*format,...){
    __builtin_va_list args_ptr;
    va_start(args_ptr, format);
    vsnprintf(formated_des, 512, format,args_ptr);
    va_end(args_ptr);
} 


 char* string_copy(char*dst, const char*source){
     return strcpy(dst, source);
 };

 char* string_n_copy(char*dst, const char*source, int length){
    return strncpy(dst,source,(size_t)length);
 };

 char* string_trim(char*str){
      while(isspace((unsigned char)*str)){
        str++;
      }
      if(*str){
        char*p  = str;
        while(*p){
            p++;
        }
        while(isspace((unsigned char)*(--p)));
        p[1] = '\0';
      }
      return str;
 };

 void string_mid(char*dst, const char*source, int start, int length){
     if(length == 0){
      dst[0] = 0;
      return;
     }
     if(length > 0){
      for(u64 i = start, j =0; j < length && source[i]; i++,j++){
        dst[j] = source[i];
      }
      dst[length+1] = 0;
     }else{
      u64 j = 0;
      for(u64 i = start; source[i]; i++,j++){
        dst[j] = source[i];
      }
      dst[j+1] = 0;
     }

 };

 int string_index_of(char* str, char c){
    if(!str){
      return -1;
    }
    u32 length =string_length(str);
    if(length > 0){
        for(u32 i = 0; i < length; i++){
            if(str[i] == c){
              return i;
            }
        }
    }
    return -1;
 };

 bool string_to_vec4(char*str, vec4*out_vec4){
   if(!str){
     return false;
   }
   Dzero_memory(out_vec4, sizeof(vec4));
   int result = sscanf(str, "%f %f %f %f", &out_vec4->x, &out_vec4->y, &out_vec4->z, &out_vec4->w);
   return result != -1;
 };


 bool string_to_vec3(char*str, vec3*out_vec3){
       if(!str){
     return false;
   }

   Dzero_memory(out_vec3, sizeof(vec3));
   int result = sscanf(str, "%f %f %f", &out_vec3->x, &out_vec3->y, &out_vec3->z);
   return result != -1;
 };

void string_empty(char*str){
  if(str){
    str[0]='\0';
    return;
  }
  DWARNING("passed a nullptr or an invalid string address to string empty returning nullptr");
}

 bool string_to_vec2(char*str, vec2*out_vec2){
  if(!str){
     return false;
   }

   Dzero_memory(out_vec2, sizeof(vec2));
   int result = sscanf(str, "%f %f ", &out_vec2->x, &out_vec2->y);
   return result != -1;
 };

 bool string_to_float(char*str, float*f){
     if(!str){
        return false;
     }
     *f = 0;
     int result = sscanf(str, "%f",f);
     return result != -1;
 };

 bool string_to_double(char*str, double*f){
          if(!str){
            return false;
        }
        *f = 0;
        int result = sscanf(str, "%lf",f);
        return result != -1;
 };

 bool string_to_i32 (char*str, int*i){
    if(!str){
        return false;
     }
     *i = 0;
     int result = sscanf(str, "%i",i);
     return result != -1;
 };

 bool string_to_i64 (char*str, i64*i){
     if(!str){
        return false;
     }
     *i = 0;
     int result = sscanf(str, "%lli",i);
     return result != -1;
 };

 bool string_to_i16 (char*str, i16*i){
     if(!str){
        return false;
     }
     *i = 0;
     int result = sscanf(str, "%hi",i);
     return result != -1;
 };

 bool string_to_u32(char*str, u32*i){
     if(!str){
        return false;
     }
     *i = 0;
     int result = sscanf(str, "%u",i);
     return result != -1;
 };

 bool string_to_u64(char*str, u64*i){
     if(!str){
        return false;
     }
     *i = 0;
     int result = sscanf(str, "%llu",i);
     return result != -1;
 };

 bool string_to_u16(char*str, u16*i){
     if(!str){
        return false;
     }
     *i = 0;
     int result = sscanf(str, "%hu",i);
     return result != -1;
 };

 bool string_to_u8(char*str, u8*i){
     if(!str){
        return false;
     }
     *i = 0;
     int result = sscanf(str, "%hhu",i);
     return result != -1;
 };

 bool string_to_bool(char* str, bool*b){
     if(!str){
      return false;
     }
     *b = string_equal(str,"1") || string_equal(str, "true");
     return true;
 };

 bool char_isspace(char c){
   int value =  isspace(c);
   return (value != 0);
 };


void string_append_string(char*dest, const char*source, const char*append){
  sprintf(dest,"%s%s",source,append);
};

void string_append_int(char*dest, const char*source, i64 i){
    sprintf(dest, "%s%lli",source,i);
};

void string_append_float(char*dest, const char*source, float f){
    sprintf(dest, "%s%f",source,f);
};

void string_append_bool(char*dest, const char*source, bool b){
    sprintf(dest, "%s%s",source, b?"true" : "false");
};

void string_append_char(char*dest, const char*source, char c){
     sprintf(dest, "%s%c",source,c);
};

void string_directory_from_path(char*dest, const char*path){
    u64 length = strlen(path);
    for(int i=length; i >= 0; --i){
       char c = path[i];
       if(c == '/' || c == '\\'){
        strncpy(dest, path, i+1);
        return;
       }
    }
};

void string_filename_from_path(char*dest, const char*path){
    u64 length = strlen(path);
    for(int i=length; i >= 0; --i){
       char c = path[i];
       if(c == '/' || c == '\\'){
        strcpy(dest, path + i + 1);
        return;
       }
    }
};

void string_filename_no_extension_from_path(char*dest, const char*path){
   u64 length = strlen(path);
   u64 start = 0;
   u64 end = 0;
   for(int i = length; i >= 0; --i){
      char c = path[i];
      if(end == 0 && c == '.'){
        end = i;
      }
      if(start == 0 && (c == '/' || c == '\\')){
        start = i + 1;
        break;
      }
   }
   string_mid(dest, path, start, end-start);
};

void string_remove_characters(char*str, char*characters, u32 number, char*dest){
  // if(!str){
  //   DWARNING("reading invalid pointer or address to strings");
  //   return; 
  // }
  // char*p = &dest[0];
  // char*ps = &str[0];
  // for(u32 i=0; i < number; i++){
  //   char c = characters[i];
  //   while(*ps){
  //       if(*ps != c){
  //         *p = *ps;
  //          p++;
  //       }
  //         ps++;
  //     }
  //     ps = &str[0];
  // }
}