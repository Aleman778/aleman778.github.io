
// NOTE(alexander): io
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

// NOTE(alexander): memory arena
void*
arena_push_size(Arena* arena, umm size, umm align, umm flags) {
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
        // TODO(alexander): we need to also store the previous memory block so we can eventually free it.
    }
    
    void* result = arena->base + offset;
    arena->prev_used = arena->curr_used;
    arena->curr_used = offset + size;
    
    // TODO(alexander): add memory clear to zero flag
    
    return result;
}