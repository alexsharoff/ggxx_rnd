#pragma once

#ifdef _WIN32
#  ifdef libgg_EXPORTS
#    define LIBGG_API __declspec(dllexport)
#  else
#    define LIBGG_API __declspec(dllimport)
#  endif
#else
#  define LIBGG_API
#endif

extern "C" LIBGG_API void libgg_init();
