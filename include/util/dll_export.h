#pragma once

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)

#define IS_WINDOWS

#ifdef AXONUTIL_EXPORTS
#define AXON_UTIL_API __declspec(dllexport)
#else
#define AXON_UTIL_API __declspec(dllimport)
#endif

#else

#define AXON_UTIL_API 

#endif