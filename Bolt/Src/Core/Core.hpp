#pragma once

#ifdef BT_PLATFORM_WINDOWS
 #ifdef BT_BUILD_DLL
   #define BOLT_API __declspec(dllexport)
  #else
   #define BOLT_API __declspec(dllimport)
 #endif
#else
#error Bolt only supports Windows!
#endif
