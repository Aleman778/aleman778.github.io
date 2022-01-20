#ifndef GENERATOR_H
#define GENERATOR_H

#include "stdio.h"
#include "stdlib.h"
#include "stdint.h"

#define array_count(array) (sizeof(array) / sizeof((array)[0]))
#define zero_struct(s) (memset(&s, 0, sizeof(s)))

#if BUILD_DEBUG
void
__assert(const char* expression, const char* file, int line) {
    // TODO(alexander): improve assertion printing,
    // maybe let platform layer decide how to present this?
    fprintf(stderr, "%s:%d: Assertion failed: %s\n", file, line, expression);
    *(int *)0 = 0; // NOTE(alexander): purposfully trap the program
}

#define assert(expression) (void)((expression) || (__assert(#expression, __FILE__, __LINE__), 0))
#else
#define assert(expression)
#endif

// NOTE(Alexander): define more convinient types
#define true 1
#define false 0
typedef int          bool;
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

typedef struct {
    char* data;
    size_t count;
} string;

inline string
string_lit(cstring lit) {
    string result;
    result.data = (char*) lit;
    result.count = strlen(lit);
    return result;
}

// NOTE(Alexander): allocates a new cstring that is null terminated
inline cstring
string_to_cstring(string str) {
    char* result = (char*) malloc(str.count + 1);
    memcpy(result, str.data, str.count);
    result[str.count] = 0;
    return (cstring) result;
}

u64
string_hash(string str) {
    u64 hash = 5381;
    for (u64 i = 0; i < str.count; i++) {
        hash = (hash * 33) ^ (size_t) str.data[i];
    }
    return hash;
}

int
string_compare(string a, string b) {
    if (a.count == b.count) {
        for (umm i = 0; i < a.count; i++) {
            if (a.data[i] != b.data[i]) {
                return a.data[i] - b.data[i];
            }
        }
    } else if (a.count > b.count) {
        return a.data[a.count - 1];
    } else {
        return '\0' - b.data[b.count - 1];
    }
    
    return 0;
}

inline int
string_equals(string a, string b) {
    return string_compare(a, b) == 0;
}

typedef struct {
    char* data;
    umm size;
    umm curr_used;
} String_Builder;

inline void
string_builder_free(String_Builder* sb) {
    free(sb->data);
    sb->data = 0;
    sb->curr_used = 0;
    sb->size = 0;
}

inline void
string_builder_alloc(String_Builder* sb, umm new_size) {
    void* new_data = realloc(sb->data, new_size);
    if (!new_data) {
        free(sb->data);
        sb->data = (char*) malloc(new_size);
    }
    sb->data = (char*) new_data;
    sb->size = new_size;
}

inline void
string_builder_ensure_capacity(String_Builder* sb, umm capacity) {
    umm min_size = sb->curr_used + capacity;
    if (min_size > sb->size) {
        umm new_size = max(sb->size * 2, min_size);
        string_builder_alloc(sb, new_size);
    }
}

void
string_builder_push_string(String_Builder* sb, string str) {
    string_builder_ensure_capacity(sb, str.count);
    
    memcpy(sb->data + sb->curr_used, str.data, str.count);
    sb->curr_used += str.count;
}

void
string_builder_push_cstring(String_Builder* sb, cstring str) {
    string_builder_push_string(sb, string_lit(str));
}

string
string_builder_to_string(String_Builder* sb) {
    string result;
    result.data = (char*) malloc(sb->curr_used + 1);
    result.count = sb->curr_used;
    memcpy(result.data, sb->data, sb->curr_used);
    result.data[result.count] = 0;
    return result;
}

string
string_builder_to_string_nocopy(String_Builder* sb) {
    string result;
    result.data = sb->data;
    result.count = sb->curr_used;
    return result;
}

string
read_entire_file(cstring filepath) {
    string result;
    zero_struct(result);
    
    FILE* file;
    fopen_s(&file, filepath, "rb");
    if (!file) {
        printf("File `%s` was not found!", filepath);
        return result;
    }
    
    fseek(file, 0, SEEK_END);
    umm file_size = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    
    result.data = malloc(file_size);
    result.count = file_size;
    fread(result.data, result.count, 1, file);
    fclose(file);
    return result;
}

bool
write_entire_file(cstring filepath, string contents) {
    FILE* file = fopen(filepath, "wb+");
    if (!file) {
        printf("Failed to open `%s` for writing!\n", filepath);
        return false;
    }
    fwrite(contents.data, contents.count, 1, file);
    fclose(file);
    
    return true;
}

// TODO(Alexander): OS probably has a better option 
bool
copy_file(cstring src_filepath, cstring dst_filepath) {
    string contents = read_entire_file(src_filepath);
    if (!contents.data) {
        return false;
    }
    bool result = write_entire_file(dst_filepath, contents);
    free(contents.data);
    return result;
}

typedef struct {
    string text;
    char symbol;
    int number;
    bool whitespace;
    bool new_line;
} Token;

typedef struct {
    char* base;
    char* curr;
    char* end;
    Token peeked;
} Tokenizer;

inline bool 
is_digit(char c) {
    return c >= '0' && c <= '9';
}

inline bool
is_end_of_line(char c) {
    return c == '\n' || c == '\r';
}

inline bool
is_whitespace(char c) {
    return c == ' ' || c == '\t' || c == '\v' || c == '\f' || is_end_of_line(c);
}

inline bool
is_whitespace_no_new_line(char c) {
    return c == ' ' || c == '\t' || c == '\v' || c == '\f';
}

inline bool
is_special_character(char c) {
    return c == '*'
        || c == '#'
        || c == '$'
        || c == '@'
        || c == ':'
        || c == '/'
        || c == '"'
        || c == '`'
        || c == '.'
        || c == ','
        || c == '_'
        || c == '+'
        || c == '-'
        || c == '[' 
        || c == ']'
        || c == '('
        || c == ')'
        || c == '{'
        || c == '}'
        || c == '<'
        || c == '>';
}

Token
next_token(Tokenizer* t) {
    Token result;
    
    if (t->peeked.symbol) {
        result = t->peeked;
        t->peeked.symbol = 0;
        return result;
    }
    
    zero_struct(result);
    result.number = -1;
    result.symbol = *t->curr;
    result.text.data = t->curr;
    
    if (t->curr == t->end) {
        result.symbol = 0;
        result.new_line = true;
        result.whitespace = true;
        return result;
    }
    
    char c = *t->curr++;
    if (is_special_character(c)) {
        while (t->curr < t->end && *t->curr == c) {
            t->curr++;
        }
    } else if (is_end_of_line(c)) {
        if (c == '\r' && *t->curr == '\n') t->curr++; // crlf
        result.new_line = true;
        result.whitespace = true;
    } else if (is_whitespace_no_new_line(c)) {
        result.whitespace = true;
        while (t->curr < t->end && is_whitespace_no_new_line(*t->curr)) {
            t->curr++;
        }
    } else if (is_digit(c)) {
        result.number = c - '0';
        while (t->curr < t->end && is_digit(*t->curr)) {
            c = *t->curr++;
            result.number = result.number * 10 + c - '0';
        }
    } else {
        while (t->curr < t->end && !is_whitespace(*t->curr) && !is_special_character(*t->curr)) {
            t->curr++;
        }
    }
    
    result.text.count = (int) (t->curr - result.text.data);
    
    return result;
}

Token
peek_token(Tokenizer* t) {
    if (!t->peeked.symbol) {
        t->peeked = next_token(t);
    }
    return t->peeked;
}

string
parse_enclosed_string(Tokenizer* t, char open, char close) {
    string result;
    zero_struct(result);
    
    if (open != 0) {
        Token token = peek_token(t);
        if (token.symbol != open) {
            return result;
        }
        next_token(t);
    }
    
    Token token = next_token(t);
    result.data = token.text.data;
    result.count = 0;
    
    while (token.symbol != close) {
        result.count += token.text.count;
        token = next_token(t);
    }
    
    return result;
}

typedef enum {
    CodeBlockLanguage_None,
    CodeBlockLanguage_C
} Code_Block_Language;

typedef enum {
    Dom_None,
    Dom_Root,
    Dom_Line_Break,
    Dom_Inline_Text,
    Dom_Paragraph,
    Dom_Heading,
    Dom_Unordered_List,
    Dom_Ordered_List,
    Dom_List_Item,
    Dom_Image,
    Dom_Link,
    Dom_Date,
    Dom_Code_Block,
} Dom_Node_Type;

typedef enum {
    TextStyle_None = 0,
    TextStyle_Italics = 1<<0,
    TextStyle_Bold = 1<<1,
    TextStyle_Code = 1<<2,
} Text_Style;

// Forward declare
typedef struct Dom_Node Dom_Node;

typedef struct {
    Dom_Node* first;
    Dom_Node* last;
} Dom_Sequence;

struct Dom_Node {
    Dom_Node_Type type;
    
    string text;
    Text_Style text_style;
    
    Dom_Node* next;
    int depth;
    
    union {
        struct {
            Dom_Sequence seq;
        } paragraph;
        
        struct {
            int level;
        } heading;
        
        struct {
            Dom_Sequence seq;
        } unordered_list;
        
        struct {
            Dom_Sequence seq;
        } ordered_list;
        
        struct {
            Dom_Sequence seq;
        } list_item;
        
        struct {
            string source;
        } image;
        
        struct {
            string source;
        } link;
        
        struct {
            int year;
            int month;
            int day;
        } date;
        
        struct {
            Code_Block_Language language;
        } code_block;
    };
};

typedef struct {
    Dom_Sequence seq;
} Dom;

typedef struct Memory_Block_Header Memory_Block_Header;
struct Memory_Block_Header {
    Memory_Block_Header* prev;
    Memory_Block_Header* next;
    
    // NOTE(Alexander): sizes included the header
    umm size;
    umm size_used;
};

typedef struct {
    char* base;
    umm size;
    umm curr_used;
    umm prev_used;
    umm min_block_size;
} Memory_Arena;

#define ARENA_DEFAULT_BLOCK_SIZE 10240; // 10 kB

// NOTE(Alexander): align has to be a power of two.
inline umm
align_forward(umm address, umm align) {
    umm modulo = address & (align - 1);
    if (modulo != 0) {
        address += align - modulo;
    }
    return address;
}

void*
arena_push_size(Memory_Arena* arena, umm size, umm align) {
    umm current = (umm) (arena->base + arena->curr_used);
    umm offset = align_forward(current, align) - (umm) arena->base;
    
    if (offset + size > arena->size) {
        if (arena->min_block_size == 0) {
            arena->min_block_size = ARENA_DEFAULT_BLOCK_SIZE;
        }
        
        void* block = calloc(1, arena->min_block_size);
        Memory_Block_Header* header = (Memory_Block_Header*) block;
        header->size = arena->min_block_size;
        
        if (arena->base) {
            Memory_Block_Header* prev_header = (Memory_Block_Header*) block;
            prev_header->size_used = arena->curr_used;
            prev_header->next = header;
            header->prev = prev_header;
        }
        
        arena->base = block;
        arena->curr_used = sizeof(Memory_Block_Header);
        arena->prev_used = arena->curr_used;
        arena->size = arena->min_block_size;
        
        current = (umm) arena->base + arena->curr_used;
        offset = align_forward(current, align) - (umm) arena->base;
    }
    
    void* result = arena->base + offset;
    arena->prev_used = arena->curr_used;
    arena->curr_used = offset + size;
    
    Memory_Block_Header* header = (Memory_Block_Header*) arena->base;
    header->size_used = offset + size;
    
    return result;
}

#define arena_push_struct(arena, type) (type*) arena_push_size(arena, sizeof(type), 16)

inline void
arena_push_string(Memory_Arena* arena, string str) {
    void* ptr = arena_push_size(arena, str.count, 1);
    memcpy(ptr, str.data, str.count);
}

inline void
arena_push_cstring(Memory_Arena* arena, cstring data) {
    umm count = strlen(data);
    void* ptr = arena_push_size(arena, count, 1);
    memcpy(ptr, data, count);
}

void
arena_push_new_line(Memory_Arena* arena, int trailing_spaces) {
    char* buf = (char*) arena_push_size(arena, trailing_spaces + 1, 1);
    *buf++ = '\n';
    for (int i = 0; i < trailing_spaces; i++) *buf++ = ' ';
}

inline Dom_Node*
arena_push_dom_node(Memory_Arena* arena, Dom_Node* parent_node) {
    Dom_Node* node = arena_push_struct(arena, Dom_Node);
    if (parent_node) {
        node->text_style = parent_node->text_style;
        parent_node->next = node;
    }
    return node;
}

inline bool
is_unordered_list_symbol(char c) {
    return c == '*' || c == '+' || c == '-';
}

// NOTE(Alexander): parses a chain of nodes, returns the last node
Dom_Sequence
parse_markdown_text_line(Tokenizer* t, Memory_Arena* arena, Token token)  {
    Dom_Sequence result;
    
    result.first = arena_push_dom_node(arena, 0);
    result.last = result.first;
    
    Dom_Node* node = result.first;
    node->type = Dom_Inline_Text;
    node->text.data = token.text.data;
    node->text.count = 0;
    
    for (;;) {
        if (!token.symbol) {
            break;
        }
        
        if (token.new_line) {
            node->text.count += token.text.count;
            break;
        }
        
        if (token.symbol == '*' || token.symbol == '`') {
            node = arena_push_dom_node(arena, node);
            node->type = Dom_Inline_Text;
            
            if (token.symbol == '*') {
                if (token.text.count == 1 || token.text.count > 2) {
                    node->text_style ^= TextStyle_Italics;
                }
                if (token.text.count >= 2) {
                    node->text_style ^= TextStyle_Bold;
                }
            } else {
                node->text_style ^= TextStyle_Code;
            }
            
            token = next_token(t);
            node->text.data = token.text.data;
            node->text.count = 0;
        } else if (token.symbol == '[') {
            string text = parse_enclosed_string(t, 0, ']');
            string src = parse_enclosed_string(t, '(', ')');
            
            if (text.count > 0 && src.count > 0) {
                node = arena_push_dom_node(arena, node);
                node->type = Dom_Link;
                node->text = text;
                node->link.source = src;
                
                token = next_token(t);
                node = arena_push_dom_node(arena, node);
                node->type = Dom_Inline_Text;
                node->text.data = token.text.data;
                node->text.count = 0;
                continue;
            }
        } else if (token.text.count >= 4) {
            if (memcmp("http", token.text.data, 4) == 0) {
                if (token.text.count == 4 || (token.text.count == 5 && token.text.data[4] == 's')) {
                    Tokenizer temp_t = *t;
                    next_token(&temp_t);
                    bool success = next_token(&temp_t).symbol != ':';
                    success &= next_token(&temp_t).symbol != '/';
                    success &= next_token(&temp_t).symbol != '/';
                    success &= !next_token(&temp_t).whitespace;
                    
                    if (success) {
                        string link;
                        link.data = token.text.data;
                        link.count = 0;
                        
                        // NOTE(Alexander): not complete parser for URLs, they are way to complicated
                        while (!token.whitespace) {
                            if ((token.symbol == ',' || token.symbol == '.') && peek_token(t).whitespace) {
                                break;
                            }
                            
                            link.count += token.text.count;
                            token = next_token(t);
                        }
                        
                        node = arena_push_dom_node(arena, node);
                        node->type = Dom_Link;
                        node->text = link;
                        node->link.source = link;
                        
                        node = arena_push_dom_node(arena, node);
                        node->type = Dom_Inline_Text;
                        node->text.data = token.text.data;
                        node->text.count = 0;
                        continue;
                    }
                } 
            }
        }
        
        node->text.count += token.text.count;
        token = next_token(t);
    }
    
    result.last = node;
    return result;
}

Dom_Sequence
parse_markdown_list(Tokenizer* t, Memory_Arena* arena, Token line_start, int curr_indent) {
    Dom_Sequence result;
    result.first = arena_push_dom_node(arena, 0);
    result.last = result.first;
    
    if (line_start.number > 0 && peek_token(t).symbol == '.') {
        next_token(t);
    }
    
    Dom_Node* node = result.first;
    node->type = Dom_List_Item;
    node->list_item.seq = parse_markdown_text_line(t, arena, next_token(t));
    
    node = arena_push_dom_node(arena, node);
    
    for (;;) {
        Token token = peek_token(t);
        if (token.new_line) {
            break;
        }
        
        if (line_start.number > 0) {
            line_start.number++;
        }
        
        int indent = 0;
        if (token.whitespace) {
            indent = (int) token.text.count; // TODO(Alexander): tabs counts as 1 unit
            next_token(t);
            token = peek_token(t);
        }
        
        // NOTE(Alexander): either symbol +, -, * (unordered is correct) or 1. 2. etc. (ordered is correct)
        bool correct_line_start = token.symbol == line_start.symbol 
            || (line_start.number > 0 && token.number == line_start.number);
        
        if (indent < curr_indent || !correct_line_start) {
            break;
        }
        
        if (indent > curr_indent) {
            next_token(t);
            node = arena_push_dom_node(arena, node);
            
            if (token.number > 0) {
                node->type = Dom_Ordered_List;
                node->ordered_list.seq = parse_markdown_list(t, arena, token, indent);
            } else if (is_unordered_list_symbol(token.symbol)) {
                node->type = Dom_Unordered_List;
                node->unordered_list.seq = parse_markdown_list(t, arena, token, indent);
            }
            
            token = peek_token(t);
            correct_line_start = token.symbol == line_start.symbol 
                || (line_start.number > 0 && token.number == line_start.number);
            token = peek_token(t);
            if (!correct_line_start) {
                break;
            }
        }
        
        token = next_token(t);
        if (line_start.number > 0 && peek_token(t).symbol == '.') {
            next_token(t);
        }
        
        node = arena_push_dom_node(arena, node);
        node->type = Dom_List_Item;
        node->list_item.seq = parse_markdown_text_line(t, arena, next_token(t));
    }
    
    result.last = node;
    return result;
}

// Forward declare
Dom read_markdown_file_ex(cstring filename, Memory_Arena* arena);

Dom_Sequence
parse_markdown_line(Tokenizer* t, Memory_Arena* arena, Dom_Node* prev_node) {
    Dom_Sequence result;
    zero_struct(result);
    
    Token token = next_token(t);
    for (;;) {
        if (!token.symbol) {
            return result;
        }
        
        if (token.whitespace) {
            token = next_token(t);
            continue;
        }
        
        break;
    }
    
    int indent = 0;
    if (token.whitespace) {
        indent = (int) token.text.count; // TODO(Alexander): tabs count as one indentation unit
        token = next_token(t);
    }
    
    // NOTE(Alexander): remove emacs -*- encoding: utf-8 -*- crap
    if (token.symbol == '-' && peek_token(t).symbol == '*') {
        Tokenizer temp_t = *t;
        next_token(&temp_t);
        Token temp_token = next_token(&temp_t);
        if (temp_token.symbol == '-') {
            temp_token = next_token(&temp_t);
            while (!temp_token.new_line) {
                temp_token = next_token(&temp_t);
            }
            *t = temp_t;
            token = next_token(t);
        }
    }
    
    if (indent == 0 && token.symbol == '@' && token.text.count == 1 && !peek_token(t).whitespace) {
        token = next_token(t);
        
        // TODO(alexander): probably not how we should detect macro definitions
        const string include_literal = string_lit("include");
        
        if (string_equals(token.text, include_literal) && peek_token(t).whitespace) {
            next_token(t);
            
            if (next_token(t).symbol == '"' && peek_token(t).symbol != '"') {
                token = next_token(t);
                string filename = token.text;
                filename.count = 0;
                while (token.symbol != '"') {
                    filename.count += token.text.count;
                    token = next_token(t);
                }
                next_token(t);
                
                // TODO(Alexander): create a preprocessing later on
                Dom included_dom = read_markdown_file_ex(string_to_cstring(filename), arena);
                
                result = included_dom.seq;
                
            } else {
                assert(0 && "invalid macro #include declaration");
            }
        } else {
            printf("Parsed unexpected macro: %.*s\n", (int) token.text.count, token.text.data);
            assert(0 && "invalid macro");
        }
        
    } else if (indent == 0 && token.symbol == '#' && peek_token(t).whitespace) {
        next_token(t);
        Token begin = next_token(t);
        Token end = begin;
        while (!end.new_line) {
            end = next_token(t);
        }
        
        Dom_Node* node = arena_push_struct(arena, Dom_Node);
        node->type = Dom_Heading;
        node->text.data = begin.text.data;
        node->text.count = (umm) (end.text.data - begin.text.data);
        node->heading.level = (int) token.text.count;
        result.first = node;
        
    } else if (is_unordered_list_symbol(token.symbol) && peek_token(t).whitespace) {
        Dom_Node* node = arena_push_struct(arena, Dom_Node);
        node->type = Dom_Unordered_List;
        node->unordered_list.seq = parse_markdown_list(t, arena, token, indent);
        result.first = node;
        
    } else if (token.number >= 0 && peek_token(t).symbol == '.') {
        Dom_Node* node = arena_push_struct(arena, Dom_Node);
        node->type = Dom_Ordered_List;
        node->ordered_list.seq = parse_markdown_list(t, arena, token, indent);
        result.first = node;
        
    } else if (token.symbol == '`' && token.text.count == 3) {
        Dom_Node* node = arena_push_struct(arena, Dom_Node);
        node->type = Dom_Code_Block;
        result.first = node;
        
    } else if (token.symbol == '!' && peek_token(t).symbol == '[') {
        string alt = parse_enclosed_string(t, '[', ']');
        string src = parse_enclosed_string(t, '(', ')');
        
        if (src.count > 0 && alt.count > 0) {
            Dom_Node* node = arena_push_struct(arena, Dom_Node);
            node->type = Dom_Image;
            node->image.source = src;
            node->text = alt;
            result.first = node;
        }
    } else {
        if (!token.new_line && prev_node->type == Dom_Paragraph) {
            // Join the two sequence of nodes into single paragraph node
            Dom_Sequence next_seq = parse_markdown_text_line(t, arena, token);
            result.first = prev_node;
            result.first->paragraph.seq.last->next = next_seq.first;
            result.first->paragraph.seq.last = next_seq.last;
        } else {
            Dom_Node* node = arena_push_struct(arena, Dom_Node);
            node->type = Dom_Paragraph;
            node->paragraph.seq = parse_markdown_text_line(t, arena, token);
            result.first = node;
        }
    }
    
    if (result.last == 0) {
        result.last = result.first;
    }
    
    return result;
}

Dom
read_markdown_file_ex(cstring filename, Memory_Arena* arena) {
    Dom result;
    zero_struct(result);
    
    string source = read_entire_file(filename);
    if (!source.data) {
        return result;
    }
    
    Tokenizer tokenizer;
    zero_struct(tokenizer);
    
    Tokenizer* t = &tokenizer;
    t->base = source.data;
    t->curr = t->base;
    t->end = t->curr + source.count;
    
    Dom_Node* root = arena_push_struct(arena, Dom_Node);
    root->type = Dom_Root;
    result.seq.first = root;
    Dom_Node* curr_node = root;
    
    while (true) {
        Dom_Sequence nodes = parse_markdown_line(t, arena, curr_node);
        if (!nodes.first || nodes.first->type == Dom_None) {
            break;
        }
        
        if (nodes.first != curr_node) {
            curr_node->next = nodes.first;
            curr_node = nodes.last;
        }
    }
    result.seq.last = curr_node;
    
    return result;
}

inline Dom
read_markdown_file(cstring filename) {
    Memory_Arena arena;
    zero_struct(arena);
    return read_markdown_file_ex(filename, &arena);
}

void
push_generated_html_from_dom_node(Memory_Arena* arena, Dom_Node* node, int depth) {
    while (node) {
        switch (node->type) {
            case Dom_Heading: {
                if (node->heading.level > 6) node->heading.level = 6;
                char level = '0' + (char) node->heading.level;
                char* open = "<h0>";
                char* close = "</h0>";
                *(open + 2) = level;
                *(close + 3) = level;
                
                arena_push_new_line(arena, depth);
                arena_push_cstring(arena, open);
                arena_push_string(arena, node->text);
                arena_push_cstring(arena, close);
            } break;
            
            case Dom_Paragraph: {
                arena_push_new_line(arena, depth);
                arena_push_cstring(arena, "<p>");
                push_generated_html_from_dom_node(arena, node->paragraph.seq.first, depth + 2);
                arena_push_cstring(arena, "</p>");
            } break;
            
            case Dom_Unordered_List: {
                arena_push_new_line(arena, depth);
                arena_push_cstring(arena, "<ul>");
                push_generated_html_from_dom_node(arena, node->unordered_list.seq.first, depth + 2);
                arena_push_cstring(arena, "</ul>");
            } break;
            
            case Dom_Ordered_List: {
                arena_push_new_line(arena, depth);
                arena_push_cstring(arena, "<ol>");
                push_generated_html_from_dom_node(arena, node->unordered_list.seq.first, depth + 2);
                arena_push_cstring(arena, "</ol>");
            } break;
            
            case Dom_List_Item: {
                arena_push_new_line(arena, depth);
                arena_push_cstring(arena, "<li>");
                push_generated_html_from_dom_node(arena, node->list_item.seq.first, depth + 2);
                arena_push_cstring(arena, "</li>");
            } break;
            
            case Dom_Image: {
                arena_push_cstring(arena, "<img alt=\"");
                arena_push_string(arena, node->text);
                arena_push_cstring(arena, "\" src=\"");
                arena_push_string(arena, node->image.source);
                arena_push_cstring(arena, "\" width=\"100%\"/>");
            } break;
            
            case Dom_Link: {
                arena_push_cstring(arena, "<a href=\"");
                arena_push_string(arena, node->link.source);
                arena_push_cstring(arena, "\">");
                arena_push_string(arena, node->text);
                arena_push_cstring(arena, "</a>");
                arena_push_new_line(arena, depth);
            } break;
            
            case Dom_Line_Break: {
                arena_push_cstring(arena, "<br>");
            } break;
            
            case Dom_Inline_Text: {
                if (node->text_style & TextStyle_Bold) {
                    arena_push_cstring(arena, "<strong>");
                }
                if (node->text_style & TextStyle_Italics) {
                    arena_push_cstring(arena, "<em>");
                }
                if (node->text_style & TextStyle_Code) {
                    arena_push_cstring(arena, "<code>");
                }
                arena_push_string(arena, node->text);
                if (node->text_style & TextStyle_Italics) {
                    arena_push_cstring(arena, "</em>");
                }
                if (node->text_style & TextStyle_Bold) {
                    arena_push_cstring(arena, "</strong>");
                }
                if (node->text_style & TextStyle_Code) {
                    arena_push_cstring(arena, "</code>");
                }
            } break;
        }
        
        node = node->next;
    }
}

string
convert_memory_arena_to_string(Memory_Arena* arena) {
    string result;
    zero_struct(result);
    
    Memory_Block_Header* header = (Memory_Block_Header*) arena->base;
    Memory_Block_Header* first_header = header;
    while (header) {
        first_header = header;
        result.count += header->size_used - sizeof(Memory_Block_Header);
        header = header->prev;
    }
    
    if (result.count > 0) {
        result.data = (char*) malloc(result.count + 1);
        
        char* dest = result.data;
        
        header = first_header;
        while (header) {
            umm size = header->size_used - sizeof(Memory_Block_Header);
            memcpy(dest, header + 1, size);
            dest += size;
            header = header->next;
        }
        
        *dest = 0;
    }
    
    return result;
}

string
generate_html_from_dom(Dom* dom) {
    Memory_Arena html_buffer;
    zero_struct(html_buffer);
    
    Dom_Node* node = dom->seq.first;
    push_generated_html_from_dom_node(&html_buffer, node, 0);
    string result = convert_memory_arena_to_string(&html_buffer);
    return result;
}

string
template_process_string(string source, int argc, string* args) {
    assert(source.data);
    
    Tokenizer tokenizer;
    zero_struct(tokenizer);
    
    Tokenizer* t = &tokenizer;
    t->base = source.data;
    t->curr = t->base;
    t->end = t->curr + source.count;
    
    String_Builder string_builder;
    zero_struct(string_builder);
    String_Builder* sb = &string_builder;
    string_builder_alloc(sb, source.count);
    
    Token token = next_token(t);
    while (token.symbol) {
        if (token.symbol == '$' && token.text.count == 1) {
            int arg_index = peek_token(t).number;
            if (arg_index >= 0 && arg_index < argc) {
                string_builder_push_string(sb, args[arg_index]);
                next_token(t);
                token = next_token(t);
            }
        }
        
        string_builder_push_string(sb, token.text);
        token = next_token(t);
    }
    
    string result = string_builder_to_string(sb);
    return result;
}


#endif //GENERATOR_H
