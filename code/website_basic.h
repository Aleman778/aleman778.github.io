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
__assert(cstring expression, cstring file, int line) {
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


// NOTE(Alexander): define more convinient types
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
typedef const char*  cstring;
// TODO(alexander): add SIMD support later...

inline int
cstring_count(cstring str) {
    int count = 0;
    while (*str++) {
        count++;
    }
    
    return count;
}

struct buffer {
    umm count;
    u8* data;
};
typedef buffer string;

inline string
string_alloc(umm count) {
    string result;
    result.data = (u8*) malloc(count);
    result.count = count;
    return result;
}

inline string
string_alloc(u8* data, umm count) {
    string result;
    result.data = (u8*) malloc(count + 1);
    memcpy(result.data, data, count);
    result.count = count;
    result.data[count] = 0;
    return result;
}

inline string
string_alloc(cstring str) {
    umm count = cstring_count(str);
    return string_alloc((u8*) str, count);
}

inline void
string_concat(string a, string b, string* dest) {
    assert(dest->count >= (a.count + b.count) && "string concatenate out of bounds");
    
    u8* dest_buffer = dest->data;
    for (int index = 0; index < a.count; index++) {
        *dest_buffer++ = *a.data++;
    }
    
    for (int index = 0; index < b.count; index++) {
        *dest_buffer++ = *b.data++;
    }
    
    *dest_buffer++ = 0;
    dest->count = a.count + b.count;
}

inline string
string_concat(string a, string b) {
    string dest = string_alloc(a.count + b.count);
    string_concat(a, b, &dest);
    return dest;
}

inline string
create_string(u8* data, umm count) {
    string result;
    result.data = data;
    result.count = count;
    return result;
}

inline string
string_lit(cstring cstr, umm count=0) {
    string result;
    result.count = count != 0 ? count : cstring_count(cstr);
    result.data = (u8*) cstr;
    return result;
}

inline string
string_view(u8* begin, u8* end) {
    string result;
    result.data = begin;
    result.count = (umm) (end - begin);
    return result;
}

inline char*
string_data(string str) {
    return (char*) str.data;
}

bool
string_equals(string a, string b) {
    if (a.count != b.count) {
        return false;
    }
    
    for (int index = 0; index < a.count; index++) {
        if (a.data[index] != b.data[index]) {
            return false;
        }
    }
    
    return true;
}

inline bool
string_equals(string a, cstring b) {
    return string_equals(a, string_lit(b));
}

// NOTE(Alexander): improved string formatting and printf
typedef int Format_Type;
enum { // TODO(Alexander): add more types
    FormatType_char,
    FormatType_int,
    FormatType_uint,
    FormatType_smm,
    FormatType_umm,
    FormatType_string,
    FormatType_cstring,
};

// TODO(Alexander): add more types
#define f_char(x) FormatType_char, (char) (x)
#define f_int(x) FormatType_int, (int) (x)
#define f_smm(x) FormatType_smm, (smm) (x)
#define f_uint(x) FormatType_uint, (uint) (x)
#define f_umm(x) FormatType_umm, (umm) (x)
#define f_string(x) FormatType_string, (int) (x).count, (char*) (x).data
#define f_cstring(x) FormatType_cstring, (cstr) (x)

void pln(cstring format...);
string string_format(cstring format...);

// NOTE(alexander): io
string read_entire_file(string filepath);
bool write_entire_file(string filepath, string contents);


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
#define array_push(a, x) arrput(a, x)
#define array_pop(a) arrpop(a)
#define array_insert(a, x, p) arrins(a, p, x)
#define array_remove(a, p) arrdel(a, p)
#define array_set_capacity(a, c) arrsetcap(a, c)
#define array_get_capacity(a) arrcap(a)

// NOTE(Alexander): hash maps
// Usage:
// struct { int key, int value }* map = 0; // don't need to allocate memory
// map_put(map, 10, 20);                   // will allocate memory here
// int x = map_get(map, 10);               // x = 20
// int count = map_count(map);             // count = 1
#define map_put(m, k, v) hmput(m, k, v)
#define map_get(m, k) hmget(m, k)
#define map_count(m) hmlen(m)

// NOTE(Alexander): hash map iterator
// Usage: continuing from previous example...
//
// int result = 0;
// map_iterator(map, it, it_index) {
//     result += it;
// }
// pln("%d", f_int(result)); // 10
#define map_iterator(map, it, it_index) \
int it_index = 0; \
for (auto it = map[0]; \
it_index < map_count(map); \
it_index++, it = map[it_index])

// NOTE(Alexander): string hash maps
#define string_map_count(m) shlen(m)
#define string_map_put(m, k, v) shput(m, k, v)
#define string_map_get(m, k) shget(m, k)
#define string_map_new_arena(m) sh_new_arena(m)
