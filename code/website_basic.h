// Basic defines useful types and functions similar to C++ standard library

#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <cstring>
#include <cstdarg>

// NOTE(alexander): rename static to better reflect its actual meaning
#define internal static
#define global static
#define local_persist static
// NOTE(alexander): count the number of elements in a fixed size array
#define array_count(array) (sizeof(array) / sizeof((array)[0]))

// NOTE(alexander): specify file size macros
#define kilobytes(value) (1024LL * (value))
#define megabytes(value) (1024LL * kilobytes(value))
#define gigabytes(value) (1024LL * megabytes(value))
#define terabytes(value) (1024LL * gigabytes(value))

// NOTE(alexander): bit stuff
#define bit(x) (1 << (x))

// NOTE(alexander): define assestion macro on debug mode
#ifdef assert
#undef assert
#endif

#if BUILD_DEBUG
void
__assert(cstr expression, cstr file, int line) {
    // TODO(alexander): improve assertion printing.
    fprintf(stderr, "%s:%d: Assertion failed: %s\n", file, line, expression);
    *(int *)0 = 0; // NOTE(alexander): purposefully trap the program
}
#define assert(expression) (void)((expression) || (__assert(#expression, __FILE__, __LINE__), 0))
#else
#define assert(expression)
#endif

// TODO(alexander): special asserts
#define assert_enum(T, v) assert((v) > 0 && (v) < T##_Count && "enum value out of range")
#define assert_power_of_two(x) assert((((x) & ((x) - 1)) == 0) && "x is not power of two")

// NOTE(alexander): define more convinient types
typedef unsigned int uint;
typedef int8_t       s8;
typedef uint8_t      u8;
typedef int16_t      s16;
typedef uint16_t     u16;
typedef int32_t      s32;
typedef uint32_t     u32;
typedef int64_t      s64;
typedef uint64_t     u64;
typedef uintptr_t    umm;
typedef intptr_t     smm;
typedef float        f32;
typedef double       f64;
typedef int32_t      b32;
typedef char*        str;
typedef const char*  cstr;

// NOTE(alexander): string functions
#define str_count(s) *((u32*) s - 1)

// TODO(alexander): lazy!!!! don't use malloc for this, put in arena later...
inline str
str_alloc(u32 count) {
    char* result = (char*) malloc(count + 5) + 4;
    result[count] = '\0';
    *((u32*) result - 1) = count;
    return (str) result;
}

inline str
str_lit(str string, u32 count) {
    str result = str_alloc(count);
    memcpy(result, string, count);
    return result;
}

inline str
str_lit(cstr string) {
    u32 count = (u32) strlen(string);
    return str_lit((str) string, count);
}

inline str
str_concat(str a, str b) {
    u32 a_count = str_count(a);
    u32 b_count = str_count(b);
    str result = str_alloc(a_count + b_count);
    memcpy(result, a, a_count);
    memcpy(result + a_count, b, b_count);
    return result;
}

inline u32
count(str string) {
    return str_count(string);
}

// NOTE(alexander): io
str read_entire_file(str filepath);
bool write_entire_file(str filepath, str contents);


// TODO(alexander): implement this later, we use stb_ds for now!
// NOTE(alexander): dynamic arrays, usage:
//     i32* array = 0;
//     arr_push(array, 5);

//struct Array_Header {
//smm count;
//smm capacity;
//};

//#define arr_push(a, x) _arr_push(a, sizeof((a)[0]), )
//#define arr_count(a) ((Array_Header*) (a) - 1)->count
//#define arr_capacity(a) ((Array_Header*) (a) - 1)->capacity

//void
//_arr_alloc(void** array, smm elem_size, smm capacity) {
//if (*array) {
//Array_Header* header = (Array_Header*) *array - 1;
//smm new_capacity = header->capacity*2;
//
//} else {
//Array_Header* header = (Array_Header*) malloc(sizeof(Array_Header) + capacity*elem_size);
//header->count = 0;
//header->capacity = capacity;
//*array = header + 1;
//}
//}

//void
//_arr_push(void* array, smm elem_size, void* data) {

//}

// NOTE(alexander): change the naming convention of stb_ds
#define arr_push(a, x) arrput(a, x)
#define arr_pop(a, x) arrpop(a, x)
#define arr_insert(a, x, p) arrins(a, p, x)
#define arr_remove(a, p) arrdel(a, p)
#define arr_set_capacity(a, c) arrsetcap(a, c)
#define arr_get_capacity(a) arrcap(a)

#define map_put(m, k, v) hmput(m, k, v)
#define str_map_put(m, k, v) shput(m, k, v)
#define str_map_get(m, k) shget(m, k)

// NOTE(alexander): hash map

// NOTE(alexander): memory arena
#ifndef DEFAULT_ALIGNMENT
#define DEFAULT_ALIGNMENT (2*alignof(smm))
#endif
#define ARENA_DEFAULT_BLOCK_SIZE kilobytes(10)

// NOTE(alexander): align has to be a power of two.
inline umm
align_forward(umm address, umm align) {
    assert_power_of_two(align);
    umm modulo = address & (align - 1);
    if (modulo != 0) {
        address += align - modulo;
    }
    return address;
}

// NOTE(alexander): memory arena
struct Arena {
    u8* base;
    umm size;
    umm curr_used;
    umm prev_used;
    umm min_block_size;
};

inline void
arena_initialize(Arena* arena, void* base, umm size) {
    arena->base = (u8*) base;
    arena->size = size;
    arena->curr_used = 0;
    arena->prev_used = 0;
    arena->min_block_size = 0;
}

inline void
arena_initialize(Arena* arena, umm min_block_size) {
    arena->base = 0;
    arena->size = 0;
    arena->curr_used = 0;
    arena->prev_used = 0;
    arena->min_block_size = min_block_size;
}

void* arena_push_size(Arena* arena, umm size, umm align=DEFAULT_ALIGNMENT, umm flags=0);

inline void
arena_push_string(Arena* arena, char* string, umm count) {
    void* ptr = arena_push_size(arena, count, 1);
    memcpy(ptr, string, count);
}

#define arena_push_struct(arena, type, flags) (type*) arena_push_size(arena, (umm) sizeof(type), (umm) alignof(type), flags)

inline void
arena_rewind(Arena* arena) {
    arena->curr_used = arena->prev_used;
}

inline void
arena_clear(Arena* arena) {
    arena->curr_used = 0;
    arena->prev_used = 0;
}
