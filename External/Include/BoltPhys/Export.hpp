#pragma once

#if (defined(_WIN32) || defined(_WIN64))
#ifdef BT_PHYS_BUILD_DLL
#define BOLT_PHYS_API __declspec(dllexport)
#elif BT_PHYS_IMPORT_DLL
#define BOLT_PHYS_API __declspec(dllimport)
#else
#define BOLT_PHYS_API
#endif
#else
#define BOLT_PHYS_API
#endif