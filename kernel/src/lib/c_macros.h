#ifndef __C_MACROS_H__
#define __C_MACROS_H__

#include <stdint.h>

// NOTE: yes, i lkie clang
#ifndef __clang__
#error "Provide your own non-clang stuff"
#endif

#define IS_ARRAY(arr) ( !__builtin_types_compatible_p( typeof(arr), typeof(&(arr)[0]) ) )

#define ARRAY_LEN(arr) \
	(__builtin_choose_expr( \
        IS_ARRAY(arr), \
        sizeof(arr) / sizeof((arr)[0]), \
        __builtin_trap() /* or (__builtin_unreachable(), 0) */ \
    ))

#define HIGHEST_BIT_MASK_U64 (uint64_t)~( ~0ull >> 1 )

#define STRINGIZE(x) STRINGIZE2(x)
#define STRINGIZE2(x) #x
#define LINE_STR STRINGIZE(__LINE__)
#define RUNTIME_ERR_LINE_FILE_STR ">>>QAKERNEL_ERROR<<<\nLine: " LINE_STR", File: " __FILE__

#define QK_CHECK( expr )															\
do{																																			\
	if( expr ){																																                        \
		hcf();																	    \
	}																				\
}while( 0 )		

#define STRUCT_HAS_MEMBER( type_t, member ) _Generic( ((type_t*)0)->member, default: 1, void: 0 )

#endif