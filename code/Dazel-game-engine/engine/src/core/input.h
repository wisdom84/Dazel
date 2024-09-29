#pragma once
#include "defines.h"

// recording what the keys and button would be defined as in our engine
enum mouse_button{
  BUTTON_LEFT,
  BUTTON_RIGHT,
  BUTTON_MIDDLE,
  BUTTON_MAX_BUTTON
};
#define DEFINE_KEY(code,name) KEY_##name = code
enum keys{
    DEFINE_KEY(0x08,    BACKSPACE), // the enum backspace 
    DEFINE_KEY(0x09,    TAB ),
    DEFINE_KEY(0x0C,    CLEAR),	 
    DEFINE_KEY(0x0D,    ENTER) , 
	DEFINE_KEY(0x10,	SHIFT) ,  
 	DEFINE_KEY(0x11,	CTRL) ,
    DEFINE_KEY(0x12,	ALT ) ,
	DEFINE_KEY(0x13,	PAUSE) , 
    DEFINE_KEY(0x14,	CAPS_LOCK ) ,
	DEFINE_KEY(0x15,	IME_Hangul_mode), 
	DEFINE_KEY(0x16,	IME_On) ,
	DEFINE_KEY(0x17,	IME_Junja_mode),
	DEFINE_KEY(0x18,	IME_final_mode),
	DEFINE_KEY(0x19,	IME_Hanja_mode), 
	DEFINE_KEY(0x19,	IME_Kanji_mode), 
	DEFINE_KEY(0x1A,	IME_Off) ,
	DEFINE_KEY(0x1B,	ESC ), 
	DEFINE_KEY(0x1C,	IME_convert) ,
	DEFINE_KEY(0x1D,	IME_nonconvert) ,
	DEFINE_KEY(0x1E,	IME_accept),  
	DEFINE_KEY(0x1F,	IME_mode_change_request),  
	DEFINE_KEY(0x20,	SPACEBAR) ,
	DEFINE_KEY(0x21,	PAGE_UP), 
	DEFINE_KEY(0x22,	PAGE_DOWN ) ,
	DEFINE_KEY(0x23,	END ) ,
	DEFINE_KEY(0x24,	HOME ), 
	DEFINE_KEY(0x25,	LEFT_ARROW ),  
	DEFINE_KEY(0x26,	UP_ARROW) , 
	DEFINE_KEY(0x27,	RIGHT_ARROW ), 
	DEFINE_KEY(0x28,	DOWN_ARROW),  
	DEFINE_KEY(0x29,	SELECT ) ,
	DEFINE_KEY(0x2A,	PRINT),  
	// DEFINE_KEY(0x2B,	EXECUTE ),  
	DEFINE_KEY(0x2C,	PRINT_SCREEN) ,  
	DEFINE_KEY(0x2D,	INS ),  
	DEFINE_KEY(0x2E,	DEL) ,  
	DEFINE_KEY(0x2F,	HELP ) ,
    DEFINE_KEY(0x30,	0 ),
    DEFINE_KEY(0x31,	1) ,
    DEFINE_KEY(0x32,	2 ),
    DEFINE_KEY(0x33,	3 ), 
    DEFINE_KEY(0x34,	4),  
    DEFINE_KEY(0x35,	5),  
    DEFINE_KEY(0x36,	6),  
    DEFINE_KEY(0x37,	7),  
    DEFINE_KEY(0x38,	8 ) ,
    DEFINE_KEY(0x39,	9),  
    DEFINE_KEY(0x41,	A ),
    DEFINE_KEY(0x42,	B ),
    DEFINE_KEY(0x43,	C ),
    DEFINE_KEY(0x44,	D ),
    DEFINE_KEY(0x45,	E ),
    DEFINE_KEY(0x46,	F ),
    DEFINE_KEY(0x47,	G ),
    DEFINE_KEY(0x48,	H ),
    DEFINE_KEY(0x49,	I ),
    DEFINE_KEY(0x4A,	J ),
    DEFINE_KEY(0x4B,	K ),
    DEFINE_KEY(0x4C,	L ),
    DEFINE_KEY(0x4D,	M ),
    DEFINE_KEY(0x4E,	N ),
    DEFINE_KEY(0x4F,	O ),
    DEFINE_KEY(0x50,	P ),
    DEFINE_KEY(0x51,	Q ),
    DEFINE_KEY(0x52,	R ),
    DEFINE_KEY(0x53,	S ),
    DEFINE_KEY(0x54,	T ),
    DEFINE_KEY(0x55,	U ),
    DEFINE_KEY(0x56,	V ),
    DEFINE_KEY(0x57,	W ),
    DEFINE_KEY(0x58,	X ),
    DEFINE_KEY(0x59,	Y ),
    DEFINE_KEY(0x5A,	Z ),
	DEFINE_KEY(0x5B,	Left_Windows ),
	DEFINE_KEY(0x5C,	Right_Windows ),
	DEFINE_KEY(0x5D,	Applications ),
    DEFINE_KEY(0x5F,	Computer_Sleep ),
    DEFINE_KEY(0x60,	Numeric_keypad_0 ),
    DEFINE_KEY(0x61,	Numeric_keypad_1),
    DEFINE_KEY(0x62,	Numeric_keypad_2 ),
    DEFINE_KEY(0x63,	Numeric_keypad_3 ),
    DEFINE_KEY(0x64,	Numeric_keypad_4 ),
    DEFINE_KEY(0x65,	Numeric_keypad_5 ),
    DEFINE_KEY(0x66,	Numeric_keypad_6 ),
    DEFINE_KEY(0x67,	Numeric_keypad_7 ),
    DEFINE_KEY(0x68,	Numeric_keypad_8 ),
    DEFINE_KEY(0x69,	Numeric_keypad_9), 
    DEFINE_KEY(0x6A,	Multiply ),
    DEFINE_KEY(0x6B,	Add_key),
    DEFINE_KEY(0x6C,	Separator), 
    DEFINE_KEY(0x6D,	Subtract), 
    DEFINE_KEY(0x6E,	Decimal), 
    DEFINE_KEY(0x6F,	Divide ),
    DEFINE_KEY(0x70,	F1 ),
    DEFINE_KEY(0x71,	F2 ),
    DEFINE_KEY(0x72,	F3), 
    DEFINE_KEY(0x73,	F4 ),
    DEFINE_KEY(0x74,	F5 ),
    DEFINE_KEY(0x75,	F6 ),
    DEFINE_KEY(0x76,	F7 ),
    DEFINE_KEY(0x77,	F8 ),
    DEFINE_KEY(0x78,	F9 ),
    DEFINE_KEY(0x79,	F10 ),
    DEFINE_KEY(0x7A,	F11 ),
    DEFINE_KEY(0x7B,	F12), 
    DEFINE_KEY(0x7C,	F13 ),
    DEFINE_KEY(0x7D,	F14 ),
    DEFINE_KEY(0x7E,	F15 ),
    DEFINE_KEY(0x81,	F18 ),
    DEFINE_KEY(0x82,	F19), 
    DEFINE_KEY(0x83,	F20), 
    DEFINE_KEY(0x84,	F21 ),
    DEFINE_KEY(0x85,	F22 ),
    DEFINE_KEY(0x86,	F23),
    DEFINE_KEY(0x87,	F24 ),
    DEFINE_KEY(0x90,	NUM_LOCK), 
    DEFINE_KEY(0x91,	SCROLL_LOCK), 
    DEFINE_KEY(0xA0,	Left_SHIFT), 
    DEFINE_KEY(0xA1,	Right_SHIFT), 
    DEFINE_KEY(0xA2,	Left_CONTROL), 
    DEFINE_KEY(0xA3,	Right_CONTROL), 
    DEFINE_KEY(0xA4,	Left_ALT ),
    DEFINE_KEY(0xA5,	Right_ALT),  
  
    KEY_MAX_KEYS
    
};

void input_intialize();
void input_shutdown();
void input_update(float delta_time);

//handling input from keys
// we need account for the state of the key: if pressed or released
//keyboard input
EXP bool input_key_is_down(keys key);
EXP bool input_key_is_up(keys key);
EXP bool input_key_was_down(keys key);
EXP bool input_key_was_up(keys key);

void input_process_key(keys key, bool pressed);

// mouse input
EXP bool input_is_button_down(mouse_button button);
EXP bool input_is_button_up(mouse_button button);
EXP bool input_was_button_down(mouse_button button);
EXP bool input_was_button_up(mouse_button button);
void input_get_mouse_position(int*x,int*y);
void input_get_previous_mouse_position(int*x,int*y);

void input_process_button(mouse_button button, bool pressed);
void input_process_mouse_move(int x, int y);
void input_process_mouse_wheel(int z_delta);


