
#ifndef BME280DRIVER_DEFS_H
#define BME280DRIVER_DEFS_H
#pragma once


#ifdef BME280_EXTERN_EXPORT
#  ifdef __cplusplus
#    define BME280_EXPORT _declspec(dllexport) 
#  else
#    define BME280_EXPORT  
#  endif
#endif

#ifndef BME280_EXPORT
#  ifdef __cplusplus
#    define BME280_EXPORT  
#  else
#    define BME280_EXPORT
#  endif
#endif

#undef BME280_EXTERN_EXPORT
#endif
