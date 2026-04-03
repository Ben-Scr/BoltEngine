#pragma once

#if defined(BT_PLATFORM_WINDOWS)
#if defined(BT_BUILD_DLL)
#define BOLT_API __declspec(dllexport)
#elif defined(BT_IMPORT_DLL)
#define BOLT_API __declspec(dllimport)
#else
#define BOLT_API
#endif
#else
#define BOLT_API
#endif