#pragma once

#if defined(_WIN32) || defined(_WIN64)
#ifdef BT_BUILD_DLL
#define BOLT_PHYS_API __declspec(dllexport)
#else
#define BOLT_PHYS_API __declspec(dllimport)
#endif
#else
#define BOLT_PHYS_API
#endif