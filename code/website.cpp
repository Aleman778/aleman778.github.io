#include "website.h"

#include "website_basic.cpp"
#include "website_tokenizer.cpp"
#include "website_format_markdown.cpp"

internal bool
eat_string(u8** scanner, cstring pattern) {
    u32 count = 0;
    u8* scan = *scanner;
    while (*pattern) {
        count++;
        if (*scan++ != *pattern++) {
            return false;
        }
    }
    
    *scanner = scan;
    return true;
}

internal u32
eat_until(u8** scanner, u8 end) {
    u32 count = 0;
    u8* scan = *scanner;
    while (*scan) {
        if (*scan++ == end) {
            *scanner = scan;
            return count;
        }
        count++;
    }
    
    return 0;
}

internal void
generate_dom_from_markdown_text(Memory_Arena* arena, u8** scanner, bool double_newline=false) {
}

int
main(int argc, char* argv[]) {
    const string output_directory = string_lit("generated/");
    const string template_directory = string_lit("templates/");
    const string css_directory = string_lit("css/");
    const string js_directory = string_lit("js/");
    const string docs_directory = string_lit("docs/");
    
    struct{ cstring key; string value; }* template_params = 0;
    string_map_put(template_params, "script_filepath", string_lit("script.js"));
    string_map_put(template_params, "style_filepath", string_lit("style.css"));
    
    
    // NOTE(alexander): markdown to html converter
    {
        string filepath = string_concat(docs_directory, string_lit("hello_world.md"));
        string markdown = read_entire_file(filepath);
        
        Tokenizer tokenizer = {};
        tokenizer_set_source(&tokenizer, markdown);
        Memory_Arena dom_arena = {};
        Dom dom = read_markdown(&tokenizer, &dom_arena);
    }
#if 0
    {
        string filepath = string_concat(docs_directory, string_lit("hello_world.md"));
        string markdown = read_entire_file(filepath);
        
        string heading_begin = string_alloc("<h0>");
        string heading_end = string_alloc("</h0>");
        Memory_Arena dom_arena = {};
        
        s32 prev_depth = 0;
        
        string* scopes = 0;
        
        u8* curr = markdown.data;
        while (*curr) {
            
            
            u8* begin_line = curr;
            
            // NOTE(Alexander): handle nested scopes, no tab support
            {
                s32 depth = 0;
                while (*curr == ' ') {
                    curr++;
                    if (*curr == ' ') {
                        curr++;
                    }
                    depth++;
                }
                
                string begin_scope = {};
                string end_scope = {};
                
                bool create_heading = false;
                u8 heading_level = '0';
                
                bool create_list_item = false;
                
                string image_alt_text = {};
                string image_link = {};
                bool create_image = false;
                
                
                if (*curr == '#') {
                    
                    u8 h = '0';
                    while (*curr == '#') {
                        curr++;
                        h++;
                    }
                    
                    
                    if (*curr == ' ') {
                        curr++;
                        if (h <= '6') {
                            create_heading = true;
                            heading_level = h;
                        }
                    }
                    
                } else if (*curr == '!' && *(curr + 1) == '[') {
                    curr += 2;
                    
                    u8* str = curr;
                    s32 count = eat_until(&curr, ']');
                    image_alt_text = create_string(str, count);
                    
                    if (*curr == '(') {
                        curr++;
                        str = curr;
                        count = eat_until(&curr, ')');
                        image_link= create_string(str, count);
                        create_image = true;
                    }
                    
                } else if (*curr == '*' && *(curr + 1) == ' ') {
                    depth++;
                    curr += 2;
                    begin_scope = string_lit("<ul>");
                    end_scope = string_lit("</ul>");
                    create_list_item = true;
                    
                } else if (*curr >= '1' && *curr <= '9' && *(curr + 1) == '.' && *(curr + 2) == ' ') {
                    depth++;
                    
                    curr += 3;
                    begin_scope = string_lit("<ol>");
                    end_scope = string_lit("</ol>");
                    
                    create_list_item = true;
                    
                } else if (*curr == '\n' || *curr == '\r') {
                    curr++;
                    continue;
                }
                
                while (prev_depth < depth && begin_scope.count && end_scope.count) {
                    prev_depth++;
                    push_string(&dom_arena, begin_scope);
                    array_push(scopes, end_scope);
                }
                
                while (depth < prev_depth) {
                    prev_depth--;
                    end_scope = array_pop(scopes);
                    push_string(&dom_arena, end_scope);
                }
                
                if (depth > 0) {
                    depth--;
                }
                
                if (create_heading) {
                    heading_begin.data[2] = heading_level;
                    heading_end.data[3] = heading_level;
                    push_string(&dom_arena, heading_begin);
                    generate_dom_from_markdown_text(&dom_arena, &curr);
                    push_string(&dom_arena, heading_end);
                    continue;
                } else if (create_list_item) {
                    push_cstring(&dom_arena, "<li>");
                    generate_dom_from_markdown_text(&dom_arena, &curr);
                    push_cstring(&dom_arena, "</li>");
                    continue;
                } else if (create_image) {
                    push_cstring(&dom_arena, "<img src=\"");
                    push_string(&dom_arena, image_link);
                    
                    push_cstring(&dom_arena, "\" alt=\"");
                    push_string(&dom_arena, image_alt_text);
                    
                    push_cstring(&dom_arena, "\">");
                    continue;
                } else {
                    curr = begin_line;
                }
                
            }
            
            push_cstring(&dom_arena, "<p>");
            generate_dom_from_markdown_text(&dom_arena, &curr, true);
            push_cstring(&dom_arena, "</p>");
        }
        
        // NOTE(Alexander): might not work if arena is stored on multiple memory blocks
        string result = create_string((u8*) dom_arena.base, (u32) dom_arena.curr_used);
        pln("%\n", f_string(markdown));
        pln("%\n", f_string(result));
        string_map_put(template_params, "body", result);
    }
#endif
    
    // NOTE(alexander): template processing
    string generated_html_code;
    {
        // NOTE(alexander): read the basic template
        string filepath = string_concat(template_directory, string_lit("basic.html"));
        string template_code = read_entire_file(filepath);
        
        Memory_Arena buffer = {};
        
        u8* curr = template_code.data;
        u8* last_push = curr;
        while (*curr) {
            u8* base = curr;
            // TODO(alexander): handle escape of dollar sign \$ or maybe $$.
            if (*curr == '$') {
                if (*(curr + 1) == '$') {
                    curr += 2;
                    continue;
                }
                
                if (*(curr + 1) == '{') {
                    curr += 2;
                    u8* variable = curr;
                    u32 count = eat_until(&curr, '}');
                    if (count > 0) {
                        push_string(&buffer, last_push, (u32) (base - last_push));
                        string result = string_map_get(template_params, string_data(string_alloc(variable, count)));
                        if (result.count) {
                            push_string(&buffer, result);
                        }
                        last_push = curr;
                    }
                }
            }
            
            *curr++;
        }
        
        if (last_push != curr - 1) {
            push_string(&buffer, last_push, (u32) (curr - last_push - 1));
        }
        
        // NOTE(Alexander): might not work if arena is stored on multiple memory blocks
        generated_html_code = string_alloc((u8*) buffer.base, (u32) buffer.curr_used);
        free(buffer.base);
    }
    
    bool success = true;
    
    // NOTE(alexander): write out the generated html
    string index_html_file = string_concat(output_directory, string_lit("index.html"));
    if (!write_entire_file(index_html_file, generated_html_code)) {
        printf("Failed to write out the generated html to `index.html`!\n");
        success = false;
    }
    
    // NOTE(alexander): write out the generated css
    string style_css_file = string_concat(output_directory, string_lit("style.css"));
    string generated_css_code = read_entire_file(string_concat(css_directory, string_lit("main.css")));
    if (!write_entire_file(style_css_file, generated_css_code)) {
        printf("Failed to write out the generated css to `style.css`!\n");
        success = false;
    }
    
    // NOTE(alexander): read javascript onload code
    string onload_js_code = read_entire_file(string_concat(template_directory, string_lit("onload.js")));
    
    // NOTE(alexander): write out the generated js
    string script_js_file = string_concat(output_directory, string_lit("script.js"));
    string generated_js_code = onload_js_code;
    generated_js_code = string_concat(onload_js_code, string_lit("function main() { console.log(\"Hello World!\"); }"));
    if (!write_entire_file(script_js_file, generated_js_code)) {
        printf("Failed to write out the generated css to `script.js`!\n");
        success = false;
    }
    
    if (!success) {
        printf("\nGenerating website failed!\n");
        return 1;
    }
    
    printf("\nGenerating website successfully!\n");
    return 0;
}
