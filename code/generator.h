#ifndef GENERATOR_H
#define GENERATOR_H

#include "stdio.h"
#include "stdlib.h"

#define array_count(array) (sizeof(array) / sizeof((array)[0]))
#define zero_struct(s) (memset(&s, 0, sizeof(s)))

typedef int bool;
#define true 1
#define false 0

typedef struct {
    char* data;
    size_t count;
} string;

inline string
string_lit(const char* lit) {
    string result;
    result.data = (char*) lit;
    result.count = strlen(lit);
    return result;
}

string
read_entire_file(const char* filepath) {
    string result;
    zero_struct(result);
    
    FILE* file;
    fopen_s(&file, filepath, "rb");
    if (!file) {
        printf("File `%s` was not found!", filepath);
        return result;
    }
    
    fseek(file, 0, SEEK_END);
    size_t file_size = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    
    result.data = malloc(file_size);
    result.count = file_size;
    fread(result.data, result.count, 1, file);
    fclose(file);
    return result;
}

bool
write_entire_file(const char* filepath, string contents) {
    FILE* file = fopen(filepath, "wb");
    if (!file) {
        printf("Failed to open `%s` for writing!", filepath);
        return false;
    }
    fwrite(contents.data, contents.count, 1, file);
    fclose(file);
    
    return true;
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
        || c == ':'
        || c == '/'
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

typedef struct Dom_Node Dom_Node;
struct Dom_Node {
    Dom_Node_Type type;
    
    string text;
    Text_Style text_style;
    
    Dom_Node* next;
    int depth;
    
    union {
        struct {
            Dom_Node* first_node;
        } paragraph;
        
        struct {
            int level;
        } heading;
        
        struct {
            Dom_Node* first_item;
        } unordered_list;
        
        struct {
            Dom_Node* first_item;
        } ordered_list;
        
        struct {
            Dom_Node* first_node;
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
    Dom_Node* root;
} Dom;

typedef struct {
    char* data;
    size_t size;
    size_t curr_used;
} String_Builder;

inline void
string_builder_alloc(String_Builder* builder, size_t new_size) {
    void* new_data = realloc(builder->data, new_size);
    if (!new_data) {
        printf("TODO: handle when we can't realloc\n");
    }
    builder->data = new_data;
    builder->size = new_size;
}

void
string_builder_push(String_Builder* builder, string str) {
    size_t required_size = str.count + builder->curr_used;
    if (required_size > builder->size) {
        size_t new_size = max(builder->size * 2, required_size);
        string_builder_alloc(builder, new_size);
    }
    
    memcpy(builder->data + builder->curr_used, str.data, str.count);
    builder->curr_used += str.count;
}

string
string_builder_to_string(String_Builder* builder) {
    string result;
    result.data = malloc(builder->curr_used + 1);
    result.count = builder->curr_used;
    memcpy(result.data, builder->data, builder->curr_used);
    result.data[result.count] = 0;
    return result;
}

string
string_builder_to_string_nocopy(String_Builder* builder) {
    string result;
    result.data = builder->data;
    result.count = builder->curr_used;
    return result;
}

typedef struct Memory_Block_Header Memory_Block_Header;
struct Memory_Block_Header {
    Memory_Block_Header* prev;
    Memory_Block_Header* next;
    
    // NOTE(Alexander): sizes included the header
    size_t size;
    size_t size_used;
};

typedef struct {
    char* base;
    size_t size;
    size_t curr_used;
    size_t prev_used;
    size_t min_block_size;
} Memory_Arena;

#define ARENA_DEFAULT_BLOCK_SIZE 10240; // 10 kB

// NOTE(Alexander): align has to be a power of two.
inline size_t
align_forward(size_t address, size_t align) {
    size_t modulo = address & (align - 1);
    if (modulo != 0) {
        address += align - modulo;
    }
    return address;
}

void*
arena_push_size(Memory_Arena* arena, size_t size, size_t align) {
    size_t current = (size_t) (arena->base + arena->curr_used);
    size_t offset = align_forward(current, align) - (size_t) arena->base;
    
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
        
        current = (size_t) arena->base + arena->curr_used;
        offset = align_forward(current, align) - (size_t) arena->base;
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
arena_push_cstring(Memory_Arena* arena, const char* data) {
    size_t count = strlen(data);
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
Dom_Node*
parse_markdown_text_line(Tokenizer* t, Memory_Arena* arena, Token token)  {
    Dom_Node* result = arena_push_dom_node(arena, 0);
    Dom_Node* node = result;
    node->type = Dom_Inline_Text;
    node->text.data = token.text.data;
    node->text.count = 0;
    
    for (;;) {
        if (!token.symbol) {
            break;
        }
        
        if (token.new_line) {
            token = peek_token(t);
            
            if (token.new_line) {
                break;
            }
            
            {
                // TODO(Alexander): hackish exceptions avoid merging lines
                Tokenizer backup = *t;
                token = next_token(&backup);
                if (token.whitespace) {
                    token = next_token(&backup);
                }
                if (is_unordered_list_symbol(token.symbol) && peek_token(&backup).whitespace) {
                    break;
                }
                if (token.number > 0 && peek_token(&backup).symbol == '.') {
                    break;
                }
            }
            
            // NOTE(Alexander): push line break
            node = arena_push_dom_node(arena, node);
            node->type = Dom_Line_Break;
            
            // NOTE(Alexander): next node
            token = next_token(t);
            node = arena_push_dom_node(arena, node);
            node->type = Dom_Inline_Text;
            node->text.data = token.text.data;
            node->text.count = 0;
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
    
    return result;
}

Dom_Node*
parse_markdown_list(Tokenizer* t, Memory_Arena* arena, Token line_start, int curr_indent) {
    Dom_Node* result = arena_push_dom_node(arena, 0);
    Dom_Node* node = result;
    
    if (line_start.number > 0 && peek_token(t).symbol == '.') {
        next_token(t);
    }
    
    node->type = Dom_List_Item;
    node->list_item.first_node = parse_markdown_text_line(t, arena, next_token(t));
    
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
                node->ordered_list.first_item = parse_markdown_list(t, arena, token, indent);
            } else if (is_unordered_list_symbol(token.symbol)) {
                node->type = Dom_Unordered_List;
                node->unordered_list.first_item = parse_markdown_list(t, arena, token, indent);
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
        node->list_item.first_node = parse_markdown_text_line(t, arena, next_token(t));
    }
    
    return result;
}

Dom_Node*
parse_markdown_line(Tokenizer* t, Memory_Arena* arena) {
    Dom_Node* node = arena_push_struct(arena, Dom_Node);
    
    Token token = next_token(t);
    for (;;) {
        if (!token.symbol) {
            return node;
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
    
    if (indent == 0 && token.symbol == '#' && peek_token(t).whitespace) {
        Token begin = next_token(t);
        Token end = begin;
        while (!end.new_line) {
            end = next_token(t);
        }
        
        node->type = Dom_Heading;
        node->text.data = begin.text.data + begin.text.count;
        node->text.count = (size_t) (end.text.data - begin.text.data) - end.text.count;
        node->heading.level = (int) token.text.count;
        
    } else if (is_unordered_list_symbol(token.symbol) && peek_token(t).whitespace) {
        node->type = Dom_Unordered_List;
        node->unordered_list.first_item = parse_markdown_list(t, arena, token, indent);
        
    } else if (token.number >= 0 && peek_token(t).symbol == '.') {
        node->type = Dom_Ordered_List;
        node->ordered_list.first_item = parse_markdown_list(t, arena, token, indent);
        
    } else if (token.symbol == '`' && token.text.count == 3) {
        node->type = Dom_Code_Block;
        
    } else if (token.symbol == '!' && peek_token(t).symbol == '[') {
        string alt = parse_enclosed_string(t, '[', ']');
        string src = parse_enclosed_string(t, '(', ')');
        
        if (src.count > 0 && alt.count > 0) {
            node->type = Dom_Image;
            node->image.source = src;
            node->text = alt;
        }
    } else {
        node->type = Dom_Paragraph;
        node->paragraph.first_node = parse_markdown_text_line(t, arena, token);
    }
    
    return node;
}

Dom
read_markdown_file(const char* filename) {
    Dom result;
    zero_struct(result);
    
    string source = read_entire_file(filename);
    Tokenizer tokenizer;
    zero_struct(tokenizer);
    
    Tokenizer* t = &tokenizer;
    t->base = source.data;
    t->curr = t->base;
    t->end = t->curr + source.count;
    
    // Allocate memory block to store the DOM in
    
    Memory_Arena dom_arena;
    zero_struct(dom_arena);
    
    result.root = arena_push_struct(&dom_arena, Dom_Node);
    Dom_Node* curr_node = result.root;
    while (true) {
        Dom_Node* next_node = parse_markdown_line(t, &dom_arena);
        if (next_node->type == Dom_None) {
            break;
        }
        
        curr_node->next = next_node;
        curr_node = next_node;
    }
    
    
    return result;
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
                push_generated_html_from_dom_node(arena, node->paragraph.first_node, depth + 2);
                arena_push_cstring(arena, "</p>");
            } break;
            
            case Dom_Unordered_List: {
                arena_push_new_line(arena, depth);
                arena_push_cstring(arena, "<ul>");
                push_generated_html_from_dom_node(arena, node->unordered_list.first_item, depth + 2);
                arena_push_cstring(arena, "</ul>");
            } break;
            
            case Dom_Ordered_List: {
                arena_push_new_line(arena, depth);
                arena_push_cstring(arena, "<ol>");
                push_generated_html_from_dom_node(arena, node->unordered_list.first_item, depth + 2);
                arena_push_cstring(arena, "</ol>");
            } break;
            
            case Dom_List_Item: {
                arena_push_new_line(arena, depth);
                arena_push_cstring(arena, "<li>");
                push_generated_html_from_dom_node(arena, node->list_item.first_node, depth + 2);
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
            size_t size = header->size_used - sizeof(Memory_Block_Header);
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
    
    Dom_Node* node = dom->root;
    push_generated_html_from_dom_node(&html_buffer, node, 0);
    string result = convert_memory_arena_to_string(&html_buffer);
    return result;
}

string
template_process_string(string source, int argc, string* args) {
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
            
            token = peek_token(t);
            if (token.symbol == '{' && token.text.count == 1) {
                next_token(t);
                token = next_token(t);
                if (token.number >= 0 && token.number < argc) {
                    string_builder_push(sb, args[token.number]);
                    next_token(t);
                }
            }
        }
        
        string_builder_push(sb, token.text);
        token = next_token(t);
    }
    
    string result = string_builder_to_string(sb);
    return result;
}


#endif //GENERATOR_H
