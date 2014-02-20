#pragma once

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)

#define IS_WINDOWS

#ifdef AXONCOMMUNICATE_EXPORTS
#define AXON_COMMUNICATE_API __declspec(dllexport)
#else
#define AXON_COMMUNICATE_API __declspec(dllimport)
#endif

#else

#define AXON_COMMUNICATE_API 

#endif