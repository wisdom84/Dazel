#include "platform.h"
#include "core/input.h"
#include "core/event_manager.h"
#include "containers/Darray.h"
#include "Renderer/vulkan/vulkan_platform.h"
// #include <vulkan/vulkan.h>
// #include <vulkan/vulkan_win32.h>
#if PLATFORM_WINDOWS

                              // WINDOWS PLATFORM
//============================================================================================
#include <stdio.h>
#include "core/logger.h"
LRESULT CALLBACK win32_process_message(HWND hwnd, u32 msg, WPARAM w_param, LPARAM l_param);

//clock
static double clock_frequency;
static LARGE_INTEGER start_time;
internal_state test_state;
bool platform_start_up(platform_state*plat_state, const char*application_name, int x, int y, int height,int width){
   plat_state->internal_state = malloc(sizeof(internal_state));
   internal_state*state=(internal_state*)plat_state->internal_state;
   state->hinstance= GetModuleHandleA(0);
   // set up and register a window class 
   WNDCLASSA wc;
   memset(&wc, 0, sizeof(wc));// zero out the memory in the window class 
   wc.hIcon = LoadIcon(state->hinstance, IDI_APPLICATION);
   wc.hInstance = state->hinstance;
   wc.hCursor = LoadCursor(state->hinstance,IDC_ARROW);
   wc.hbrBackground =(HBRUSH)(COLOR_WINDOW+1);// transparent
   wc.style = CS_DBLCLKS;// geting double clicks
   wc.cbClsExtra =0;
   wc.cbWndExtra =0;
   wc.lpszClassName = "dazel-window-class";
   wc.lpfnWndProc= win32_process_message;
   // register the window 
   if(!RegisterClassA(&wc)){
      MessageBoxA(0,"Window failed to by registered by the os", "Error",MB_ICONEXCLAMATION |MB_OK);
      return false;
   }
   // window creation
   int client_x=x;
   int client_y=y;
   int client_height= height;
   int client_width = width;

   int window_x= client_x;
   int window_y= client_y;
   int window_height=client_height;
   int window_width=client_width;

   int window_style=WS_CAPTION | WS_SYSMENU | WS_OVERLAPPED |WS_MINIMIZEBOX ;
    window_style  |= WS_MAXIMIZEBOX;
   //  window_style  |= WS_MINIMIZE ;
    window_style  |= WS_THICKFRAME;
   int window_style_ex= WS_EX_APPWINDOW;
   // we need to specify the border 
   RECT border_rect={0,0,0,0};
   AdjustWindowRectEx(&border_rect,window_style,0,window_style_ex);

   window_x += border_rect.left;
   window_y += border_rect.top;

   window_width += border_rect.right-border_rect.left;
   window_height += border_rect.bottom-border_rect.top;
   // DINFO("window_width: %i, window_height: %i", window_width, window_height);
   HWND handle= CreateWindowExA(window_style_ex, "dazel-window-class", application_name, window_style,
   window_x,window_y, window_width,window_height,0, 0, state->hinstance, 0);
   // check if window was successfully created 
   if(handle == 0){
      MessageBoxA(0,"Window creation failed", "Error",MB_ICONEXCLAMATION |MB_OK);
      DFATAL("window creattion failed!");
      return false;
   }
   else{
    state->hwnd = handle;
    test_state.hinstance = state->hinstance;
    test_state.hwnd = state->hwnd;
   }
   // show the window 
   bool should_activate= true;
   int show_window_command_flag = should_activate? SW_SHOWNORMAL : SW_SHOWNOACTIVATE; 
   ShowWindow(state->hwnd, show_window_command_flag);

   // set application start time
   LARGE_INTEGER frequency;
   QueryPerformanceFrequency(&frequency);
   clock_frequency = 1.0f/ (double)frequency.QuadPart;
   QueryPerformanceCounter(&start_time);

   return true;
 }
 void platform_shut_down(platform_state*plat_state){
    internal_state*state = (internal_state*)plat_state->internal_state;
    if(state->hwnd){
        DestroyWindow(state->hwnd);
        state->hwnd = 0;
    }
 }
 bool platform_pump_messages(platform_state*plat_state){
    internal_state*state=(internal_state*)plat_state->internal_state;
    MSG message;

    while(PeekMessageA(&message,state->hwnd,0,0,PM_REMOVE)){
        TranslateMessage(&message);
        DispatchMessageA(&message); // this triggers the windows procedure function 

    }
    return true;

 }
 void*platform_allocate(u64 size, bool aligned){
    return malloc(size);
 }
 void*platform_allocate_aligned_memory(u64 size, u64 aligned_size){
   return _aligned_malloc(size,aligned_size);
 };
 void platform_free_aligned_memory(void*block){
     _aligned_free(block);
 };
 void platform_free(void*block, char aligned){
     free(block);
 }
 void*platform_zero_memory(void*block,u64 size){
    return memset(block, 0, size);
 }
void*platform_set_memory(void*des, int value, u64 size){
    return memset(des, value, size);
}
void*platform_copy_memory(void*des, const void*source, u64 size){
    return memcpy(des, source,size);
}
void platform_console_write(const char*message, u8 color){
    HANDLE console_handle = GetStdHandle(STD_OUTPUT_HANDLE);// specific to windows only // it allows
    // us to get the handle to our console window
    static u8 levels[6]={64,4,6,2,1,8}; // u8 is an unsigned char which are int values from 0-255;
    SetConsoleTextAttribute(console_handle,levels[color]); // specific to windows also
    // allows us to add color attributes to our text on the console
    OutputDebugStringA(message); // specific to windows. output message to the  debug console 
    u64 length = strlen(message);
    LPDWORD number_written = 0;
    WriteConsoleA(GetStdHandle(STD_OUTPUT_HANDLE), message,(DWORD)length,number_written,0 );
}

void platform_console_write_error(const char*message, u8 color){
    HANDLE console_handle = GetStdHandle(STD_ERROR_HANDLE);// specific to windows only // it allows
   // us to get the handle to our console window
    static u8 levels[6]={64,4,6,2,1,8}; // u8 is an unsigned char which are int values from 0-255;
    SetConsoleTextAttribute(console_handle,levels[color]); // specific to windows also
    // allows us to add color attributes to our text on the console
    OutputDebugStringA(message); // specific to windows. output message to the  debug console 
    u64 length = strlen(message);
    LPDWORD number_written = 0;
    WriteConsoleA(GetStdHandle(STD_ERROR_HANDLE), message,(DWORD)length,number_written,0 );  
}
double get_absolute_time(){
   LARGE_INTEGER now_time;
   QueryPerformanceCounter(&now_time);
   return (double)now_time.QuadPart * clock_frequency;
}
void platform_sleep(u64 ms){
   Sleep(ms);// windows sleep function (different from the c own)
}

void platform_get_required_extensions(const char*** names_darray){
    darray_push(*names_darray,&"VK_KHR_win32_surface", const char*);
}
bool platform_create_vulkan_surface(struct platform_state*plat_state, struct vulkan_context*context){
   internal_state *state = (internal_state*) plat_state->internal_state;
   VkWin32SurfaceCreateInfoKHR create_info = {VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR};
   create_info.pNext = nullptr;
   create_info.flags = 0;
   create_info.hinstance = state->hinstance;
   create_info.hwnd = state->hwnd;
   VkResult result =vkCreateWin32SurfaceKHR(context->instance, &create_info, context->allocator,&state->surface); 
   if(result != VK_SUCCESS){
      DFATAL("vulkan surface creation failed");
      return false;
   }
   context->surface = state->surface;
   return true;
}


LRESULT CALLBACK win32_process_message(HWND hwnd, u32 msg, WPARAM w_param, LPARAM l_param){
   switch (msg){
      case WM_ERASEBKGND:
          // notify the os that erasing will be done by the engine
         return 1;
      case WM_CLOSE:
             {
             // TODO:  fire an event to close dazel application
              event_context data {};
              event_fire(EVENT_CODE_APPLICATION_QUIT,0,data);
            }
           return 0; 
      case WM_DESTROY:
          PostQuitMessage(0);
          return 0;
      case WM_SIZE:{
           RECT rect;
           if(test_state.hwnd == nullptr){
            DFATAL("the handle was a null pointer");
           }
           GetClientRect(test_state.hwnd,&rect);
           int window_rect_width = (rect.right - rect.left);
           int window_rect_height = (rect.bottom - rect.top);
           event_context context;
           context.u32_byte[0] = (u32)window_rect_width;
           context.u32_byte[1] = (u32)window_rect_height;
           event_fire(EVENT_CODE_RESIZED, 0,context);
           DINFO("event fired");
         //   test_state.window_on_resize((u32)window_rect_width, (u32)window_rect_height);
         //   DINFO("window_rect_width : %i, window_rect_height: %i", window_rect_width, window_rect_height);
          //get the upadated size
          // TODO: fire an event for the reszie of the window
      }break;
      case WM_KEYDOWN:
      case WM_SYSKEYDOWN:
      case WM_KEYUP:
      case WM_SYSKEYUP:{
         bool pressed = (msg == WM_KEYDOWN || msg == WM_SYSKEYDOWN);
         keys key =(keys)w_param;
         input_process_key(key, pressed);
         // TODO: input processing
      }break;
      case WM_MOUSEMOVE:{
         int get_x = GET_X_LPARAM(l_param);
         int get_y = GET_Y_LPARAM(l_param);
         input_process_mouse_move(get_x,get_y);  
         // TODO: input processing
      }break;
      case WM_MOUSEWHEEL:{
         int z_delta = GET_WHEEL_DELTA_WPARAM(w_param);
         if(z_delta !=0){
            // flatterning the value of the mouse wheel so that we only check if the mouse wheel
            // moved up or down
            z_delta =(z_delta <0) ? -1 : 1;
            input_process_mouse_wheel(z_delta);
            //TODO: input processing
         }
      }break;
      case WM_LBUTTONDOWN:
      case WM_LBUTTONUP:
      case WM_RBUTTONDOWN:
      case WM_RBUTTONUP:
      case WM_MBUTTONDOWN:
      case WM_MBUTTONUP:{
         bool pressed = msg == WM_LBUTTONDOWN || msg == WM_RBUTTONDOWN || msg == WM_MBUTTONDOWN;
         mouse_button button =  BUTTON_MAX_BUTTON;
         switch(msg){
            case WM_LBUTTONDOWN:
            case WM_LBUTTONUP:
                button = BUTTON_LEFT;
                break;
            case WM_RBUTTONDOWN:
            case WM_RBUTTONUP:
                 button = BUTTON_RIGHT;
                 break;
            case WM_MBUTTONDOWN:
            case WM_MBUTTONUP:
                 button = BUTTON_MIDDLE;
                 break;        
         }

        //TODO: input processing
        if(button != BUTTON_MAX_BUTTON){
         input_process_button(button, pressed);
        }
      }break;
   }
   return DefWindowProcA(hwnd, msg, w_param,l_param);

}
//============================================================================================


#endif





