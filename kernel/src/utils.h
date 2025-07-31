#ifndef __UTILS_H__
#define __UTILS_H__

#include <stdint.h>
// TODO: move to different files ?

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

#endif