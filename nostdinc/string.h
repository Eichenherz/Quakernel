#ifndef __STRING_H__
#define __STRING_H__

#include <stddef.h>

extern void* memcpy( void *restrict dest, const void *restrict src, size_t count );
extern void* memmove( void* dest, const void* src, size_t count );
extern void *memset( void *dest, int ch, size_t count );

extern size_t strlen( const char* str );

extern char* strchr( const char* str, int ch );
extern char* strrchr( const char* str, int ch );
#endif