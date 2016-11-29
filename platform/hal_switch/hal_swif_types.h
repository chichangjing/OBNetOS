
#ifndef _HAL_SWIF_TYPES_H_
#define _HAL_SWIF_TYPES_H_

#ifndef NULL
#define NULL 0
#endif

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef FALSE
#define FALSE 0
#endif

#define COMPILER_UINT64		struct { unsigned int u64_w[2]; }
#define COMPILER_INT64		struct { int u64_w[2]; }
#define u64_H(v)			((v).u64_w[1])
#define u64_L(v)			((v).u64_w[0])

typedef signed char			int8;
typedef signed short		int16;
typedef signed int			int32;
typedef COMPILER_INT64		int64;

typedef unsigned char		uint8;
typedef unsigned short		uint16;
typedef unsigned int		uint32;
typedef COMPILER_UINT64		uint64;

typedef enum {
    HAL_FALSE = 0,
    HAL_TRUE  = 1
} HAL_BOOL;

#endif

