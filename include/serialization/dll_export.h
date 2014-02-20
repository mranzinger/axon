#pragma once

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)

#ifdef AXONSERIALIZE_EXPORTS
#define AXON_SERIALIZE_API __declspec(dllexport)
#else
#define AXON_SERIALIZE_API __declspec(dllimport)
#endif

#else

#define AXON_SERIALIZE_API 

#endif