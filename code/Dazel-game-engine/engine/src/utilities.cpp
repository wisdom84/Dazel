#include "utilities.h"
void change_to_working_directory(const char*directory){
     char current_dir[1024];
    _getcwd(current_dir,sizeof(current_dir));
    const char*cwd= (const char*)current_dir; 
    DDEBUG("curerent directory: %s",current_dir);
    if(string_equal(cwd,BASE_DIRECTORY)){
       if(_chdir(directory) != 0){
        DERROR("could not change to working directory");
       };
    }
   else if (!string_equal(cwd, directory)){
       if (_chdir(BASE_DIRECTORY) !=0){
        DERROR("could not change to base directory");
       }
       if(_chdir(directory) !=0){
        DERROR("could not change to the directory");
       }
    }
}

const char*set_directory_path(const char*location){
  std::string source_directory =  SOURCE_DIRECTORY;
  std::string directory_location = location;
  std::string dir = source_directory + directory_location;
  const char*final_dir = dir.c_str();
  return final_dir;
};