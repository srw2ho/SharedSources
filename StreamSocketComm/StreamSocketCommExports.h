#pragma once

#ifndef STREAMSOCKETCOMM_H
#define STREAMSOCKETCOMM_H
#pragma once


#ifdef STREAMSOCKETCOMM_EXTERN_EXPORT
#  ifdef __cplusplus
#    define STREAMSOCKETCOMM_EXPORT _declspec(dllexport) 
#  else
#    define STREAMSOCKETCOMM_EXPORT  
#  endif
#endif

#ifndef STREAMSOCKETCOMM_EXPORT
#  ifdef __cplusplus
#    define STREAMSOCKETCOMM_EXPORT  
#  else
#    define STREAMSOCKETCOMM_EXPORT
#  endif
#endif

#undef STREAMSOCKETCOMM_EXTERN_EXPORT
#endif
