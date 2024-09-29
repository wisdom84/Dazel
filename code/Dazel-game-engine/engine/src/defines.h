#pragma once 
typedef unsigned long long u64;
typedef signed long long   i64;
typedef unsigned char u8; // is an integer from 0-255
typedef unsigned int u32;
typedef signed short i16;
typedef unsigned short u16;
// properly defining the static assertion of the types we want to use 
#if defined (__clang__)||defined (__gcc__) 
#define STATIC_ASSERT _static_assert
#else
#define STATIC_ASSERT static_assert
#endif
#define INVALID_ID 4294967295U
// ensuring all data types are of the correct sizes on our machine 
static_assert(sizeof(unsigned char)==1, "Expected to be 1 byte in size ");
static_assert(sizeof(unsigned short)==2, "Expected to be 2 byte in size ");
static_assert(sizeof(unsigned int)==4, "Expected to be 4 byte in size ");
static_assert(sizeof(unsigned long long)==8, "Expected to be 8 byte in size ");

static_assert(sizeof(signed char)==1, "Expected to be 1 byte in size ");
static_assert(sizeof(signed short)==2, "Expected to be 2 byte in size ");
static_assert(sizeof(signed int)==4, "Expected to be 4 byte in size ");
static_assert(sizeof(signed long long)==8, "Expected to be 8 byte in size ");

static_assert(sizeof(float)==4, "Expected to be 4 byte in size ");
static_assert(sizeof(double)==8, "Expected to be 8 byte in size ");


// platform detection code
#if defined (_WIN32)
#ifndef _WIN64
#error "A 64-bit machine is need to run this application"
#else
#define PLATFORM_WINDOWS 1 
#endif
#endif

#if defined (__linux__) || defined (__gnu_linux__)
#define PLATFORM_LINUX 1
#endif

#if defined (__ANDROID__) 
#define PLATFORM_ANDROID 1
#endif

#if defined (__APPLE__) 
#define PLATFORM_MAC 1
#include <Targetconditionals.h>
#endif

// for exporting and importing dll files 
#ifdef KEXPORT
#ifdef _MSC_VER
#define EXP __declspec(dllexport)
#else
#define EXP __attribute__((visibility("default")))
#endif
#else
#ifdef _MSC_VER
#define EXP __declspec(dllimport)
#else
#define EXP
#endif
#endif 
#define BASE_DIRECTORY   "C:\\dazel\\code\\Dazel-game-engine"
#define SOURCE_DIRECTORY  "C:\\dazel\\code\\Dazel-game-engine\\engine\\src\\"
#define DCLAMP(value, min_num, max_num) ({ value <= min_num ? min_num : value >= max_num ? max_num : value})                                                              

