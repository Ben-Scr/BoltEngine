#pragma once

#ifdef BT_PLATFORM_WINDOWS
 #ifdef BT_BUILD_DLL
   #define BOLT_API __declspec(dllexport)
  #elif BT_IMPORT_DLL
   #define BOLT_API __declspec(dllimport)
#else
   #define BOLT_API
 #endif
#else
#error Bolt only supports Windows!
#endif
