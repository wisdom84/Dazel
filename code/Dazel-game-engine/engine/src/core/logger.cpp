#include "logger.h"
#include "asserts.h"
#include <iostream>
#include <stdarg.h>
#include <string.h>
#include "platform/platform.h"


bool intialize_logging(){
  // TODO: Somethin later
  return true;
};
void shutdown_logging(){
  // TODO: Somethin later
};

void log_output(log_level level,const char*message,...){
   const char*levels[6] = {"[FATAL]:", "[ERROR]:","[WARNING]:","[INFO]:","[DEBUG]:","[TRACE]:"};
   bool  is_error = level < LOG_LEVEL_WARNING;

   char output_message[32000];
   memset(output_message, 0,sizeof(output_message));// this zeros out the merrory allocted on the stack
   // we using the char that can take up to 32000 spaces 

   __builtin_va_list args_ptr; // this forms a pointer to the list of the variable argumnets 

   va_start(args_ptr,message);

   vsnprintf(output_message,32000,message,args_ptr); // using printf to print things but in the format
   //of the formated string (c style string) then pasting the whole output to a buffer(output_message 
   //in our case)
   va_end(args_ptr);
   char output_message2[32000];
   sprintf(output_message2,"%s%s\n",levels[level],output_message);
   // using platform specific logging methods
   if(is_error){
    platform_console_write_error(output_message2,level);
   }
   else{
    platform_console_write(output_message2,level);
   }
  //  printf("%s",output_message2);
 };
 void report_assrtion_failure(const char*expression, const char*message,const char*file, int line){
  log_output(LOG_LEVEL_FATAL, "ASsertion Failure: %s, message:%s, in file:%s, in line:%d \n", expression, 
  message, file, line);
}
