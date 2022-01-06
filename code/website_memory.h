
// NOTE(Alexander): memory arena
#ifndef DEFAULT_ALIGNMENT
#define DEFAULT_ALIGNMENT (2*alignof(smm))
#endif
#define ARENA_DEFAULT_BLOCK_SIZE kilobytes(10)

// NOTE(Alexander): align has to be a power of two.
inline umm
align_forward(umm address, umm align) {
    assert_power_of_two(align);
    umm modulo = address & (align - 1);
    if (modulo != 0) {
        address += align - modulo;
    }
    return address;
}

// NOTE(Alexander): memory arena
struct Memory_Arena {
    u8* base;
    umm size;
    umm curr_used;
    umm prev_used;
    umm min_block_size;
};

inline void
set_specific_arena_block(Memory_Arena* arena, u8* base, umm size) {
    arena->base = base;
    arena->size = size;
    arena->curr_used = 0;
    arena->prev_used = 0;
    arena->min_block_size = size;
}

inline void
set_minimum_arena_block_size(Memory_Arena* arena, umm min_block_size=0) {
    arena->curr_used = 0;
    arena->prev_used = 0;
    arena->min_block_size = min_block_size;
}

void*
push_size(Memory_Arena* arena, umm size, umm align=DEFAULT_ALIGNMENT, umm flags=0) {
    umm current = (umm) (arena->base + arena->curr_used);
    umm offset = align_forward(current, align) - (umm) arena->base;
    
    if (offset + size > arena->size) {
        if (arena->min_block_size == 0) {
            arena->min_block_size = ARENA_DEFAULT_BLOCK_SIZE;
        }
        
        arena->base = (u8*) calloc(1, arena->min_block_size);
        arena->curr_used = 0;
        arena->prev_used = 0;
        arena->size = arena->min_block_size;
        
        current = (umm) arena->base + arena->curr_used;
        offset = align_forward(current, align) - (umm) arena->base;
        // TODO(Alexander): we need to also store the previous memory block so we can eventually free it.
    }
    
    void* result = arena->base + offset;
    arena->prev_used = arena->curr_used;
    arena->curr_used = offset + size;
    
    // TODO(Alexander): add memory clear to zero flag
    
    return result;
}

#define push_struct(arena, type, ...) (type*) push_size(arena, (umm) sizeof(type), (umm) alignof(type), __VA_ARGS__)

inline void
push_string(Memory_Arena* arena, u8* data, umm count) {
    void* ptr = push_size(arena, count, 1);
    memcpy(ptr, data, count);
}

inline void
push_string(Memory_Arena* arena, string str) {
    push_string(arena, str.data, str.count);
}

inline void
push_cstring(Memory_Arena* arena, cstring str) {
    umm count = cstring_count(str);
    void* ptr = push_size(arena, count, 1);
    memcpy(ptr, str, count);
}

inline void
arena_rewind(Memory_Arena* arena) {
    arena->curr_used = arena->prev_used;
}

inline void
clear(Memory_Arena* arena) {
    arena->curr_used = 0;
    arena->prev_used = 0;
}
