#ifndef GPIODRIVER_DEFS_H
#define GPIODRIVER_DEFS_H
#pragma once


#if GPIODRIVER_EXTERN_EXPORT
#  ifdef __cplusplus
#    define GPIODRIVER_EXPORT _declspec(dllexport) 
#  else
#    define GPIODRIVER_EXPORT  
#  endif
#endif

#ifndef GPIODRIVER_EXPORT
#  ifdef __cplusplus
#    define GPIODRIVER_EXPORT  
#  else
#    define GPIODRIVER_EXPORT
#  endif
#endif

#undef GPIODRIVER_EXTERN_EXPORT
#endif

