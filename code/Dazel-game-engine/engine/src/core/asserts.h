#pragma once
#include "defines.h"

#define DASSERTION_ENABLED


#ifdef  DASSERTION_ENABLED
//============================================================
#if  _MSC_VER
#include <intrin.h>
#define debugbreak() __debugbreak()
#else
#define debugbreak() __builtin_trap()
#endif

EXP void report_assrtion_failure(const char*expression, const char*message,const char*file, int line);


#define DASSERT(expr) {if(expr){}else{report_assrtion_failure(#expr,"",__FILE__,__LINE__);debugbreak();}}\


#define DASSERT_MSG(expr,message) {{if(expr){}else{report_assrtion_failure(#expr,message,__FILE__,__LINE__);debugbreak();}}}


#ifdef _DEBUG
#define DASSERT_DEBUG(expr) {{if(expr){}else{report_assrtion_failure(#expr,"",__FILE__,__LINE__);debugbreak();}}}
#else
#define DASSERT_DEBUG(expr);
#endif
//=============================================================================
#else
#define DASSERT(expr)
#define DASSERT_MSG(expr,message)
#define DASSERT_DEBUG(expr)
#endif