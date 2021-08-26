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

// NOTE(alexander): Input/ Output
str
read_entire_file(str filepath) {
    FILE* file;
    fopen_s(&file, filepath, "rb");
    if (!file) {
        printf("File `%s` was not found!", filepath);
        return str_lit("");
    }
    
    fseek(file, 0, SEEK_END);
    umm file_size = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    str result = (str) malloc(file_size + 5) + 4;
    *((u32*) result - 1) = (u32) file_size;
    fread(result, str_count(result), 1, file);
    fclose(file);
    return result;
}

bool
write_entire_file(str filepath, str contents) {
    FILE* file = fopen(filepath, "wb");
    if (!file) {
        printf("Failed to open `%s` for writing!", filepath);
        return false;
    }
    fwrite(contents, str_count(contents), 1, file);
    fclose(file);
    
    return true;
}