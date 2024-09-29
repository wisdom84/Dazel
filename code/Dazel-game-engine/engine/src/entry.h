#pragma once 
#include "game_types.h"
#include "core/logger.h"
#include "core/Dmemory.h"
#include "containers/Darray.h"
#include <iostream>

extern bool create_game_inst(game*out_game); // extern is a keyword used to tell the compiler
// that we have defined this function in different file and it should link it during the runtime

// create the game instance
// application entry point
int main(){
    intialize_memory();
    game out_game;
    if(!create_game_inst(&out_game)){
    DFATAL("Application game object creation was not successful");
    return -1;
    }
    // check if our function pointers are all returning successfully or in general if they are assigned
    if(!out_game.initialize || !out_game.render || !out_game.update ){
        DFATAL("The game object function pointers have not been assigned");
        return -1;
    }
   if(!application_create(&out_game)){
    DFATAL("Application creation was not successfull");
    return -1;
   };
   if(!application_run()){
     DFATAL("Application did not shutdown successfully");
     return -1;
   };
   shutdown_memory();
   return 0;
}


