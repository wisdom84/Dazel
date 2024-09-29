#include <core/Dmemory.h>
#include <entry.h>
#include "game.h"
#include <containers/Darray.h>

bool create_game_inst(game*out_game){
   //  app config  info
   out_game->app_config.application_name="Dazel game engine";
   out_game->app_config.x=100;
   out_game->app_config.y=100;
   out_game->app_config.height=600;
   out_game->app_config.width=900;
// game function pointers
   out_game->initialize=game_initialize;
   out_game->render=game_render;
   out_game->on_resize=game_onresize;
   out_game->update = game_update;
// game state
   out_game->state=Dallocate_memory(sizeof(game_state),MEMORY_TAG_GAME);
   return true;
}
 