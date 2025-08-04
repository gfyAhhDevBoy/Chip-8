#pragma once

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;

// Signed int types.
typedef signed char s8;
typedef signed short s16;
typedef signed int s32;
typedef signed long long s64;

// Regular int types.
typedef char i8;
typedef short i16;
typedef int i32;
typedef long long i64;

// Floating point types
typedef float f32;
typedef double f64;

// Boolean types
typedef u8  b8;
typedef u32 b32;

// Void function type
typedef void void_func(void);
typedef void (*void_func_ptr)(void);

#define true 1
#define false 0

#define null 0
#define u32_max 4294967295