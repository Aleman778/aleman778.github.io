#include "website.h"
#include "website_basic.cpp"

internal bool
eat_string(char** scanner, const char* pattern) {
    u32 count = 0;
    char* scan = *scanner;
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
eat_until(char** scanner, char end) {
    u32 count = 0;
    char* scan = *scanner;
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
generate_markdown_text(Arena* buffer, char** scanner, bool double_newline=false) {
    char* curr = *scanner;
    char* last_push = curr;
    while (*curr) {
        if (*curr == '*') {
            bool bold_font = *(curr + 1) == '*';
            
            arena_push_str(buffer, last_push, (u32) (curr - last_push));
            curr += bold_font ? 2 : 1;
            last_push = curr;
            
            if (bold_font) {
                arena_push_cstr(buffer, "<b>");
            } else {
                arena_push_cstr(buffer, "<i>");
            }
            
            u32 count = eat_until(&curr, '*');
            arena_push_str(buffer, last_push, count);
            curr += bold_font ? 1 : 0;
            last_push = curr;
            
            
            if (bold_font) {
                arena_push_cstr(buffer, "</b>");
            } else {
                arena_push_cstr(buffer, "</i>");
            }
        }
        
        if (*curr == '\n') {
            *curr++;
            if (*curr == '\r') {
                *curr++;
            }
            break;
        }
        
        curr++;
    }
    
    if (last_push != curr) {
        arena_push_str(buffer, last_push, (u32) (curr - last_push));
    }
    
    *scanner = curr;
}

int
main(int argc, char* argv[]) {
    const str output_directory = str_lit("generated/");
    const str template_directory = str_lit("templates/");
    const str css_directory = str_lit("css/");
    const str js_directory = str_lit("js/");
    const str docs_directory = str_lit("docs/");
    
    struct { cstr key; cstr value; }* template_params = 0;
    str_map_put(template_params, "script_filepath", "script.js");
    str_map_put(template_params, "style_filepath", "style.css");
    
    
    // NOTE(alexander): markdown to html converter
    {
        str filepath = str_concat(docs_directory, str_lit("hello_world.md"));
        str markdown = read_entire_file(filepath);
        
        str heading_begin = str_lit("<h0>");
        str heading_end = str_lit("</h0>");
        Arena buffer = {};
        
        char* curr = markdown;
        while (*curr) {
            if (*curr == '#') {
                char h = '0';
                while (*curr == '#') {
                    curr++;
                    h++;
                }
                
                if (*curr == ' ') {
                    curr++;
                    if (h <= '6') {
                        heading_begin[2] = h;
                        heading_end[3] = h;
                        arena_push_str(&buffer, heading_begin);
                        generate_markdown_text(&buffer, &curr);
                        arena_push_str(&buffer, heading_end);
                    }
                    continue;
                }
                
            } else if (*curr == '*' && *(curr + 1) == ' ') {
                arena_push_cstr(&buffer, "<ul>");
                
                do {
                    *curr += 2;
                    arena_push_cstr(&buffer, "<li>");
                    generate_markdown_text(&buffer, &curr);
                    arena_push_cstr(&buffer, "</li>");
                } while (*curr == '*' && *(curr + 1) == ' ');
                
                arena_push_cstr(&buffer, "</ul>");
                continue;
                
            } else if (*curr >= '1' && *curr <= '9') {
                curr++;
                continue;
            } else if (*curr == '\n' || *curr == '\r') {
                curr++;
                continue;
            }
            
            arena_push_cstr(&buffer, "<p>");
            generate_markdown_text(&buffer, &curr, true);
            arena_push_cstr(&buffer, "</p>");
        }
        
        str result = str_lit((char*) buffer.base, (u32) buffer.curr_used);
        printf(markdown);
        printf(result);
        str_map_put(template_params, "body", result);
    }
    
    // NOTE(alexander): template processing
    str generated_html_code;
    {
        // NOTE(alexander): read the basic template
        str filepath = str_concat(template_directory, str_lit("basic.html"));
        str template_code = read_entire_file(filepath);
        
        Arena buffer = {};
        
        char* curr = template_code;
        char* last_push = curr;
        while (*curr) {
            char* base = curr;
            // TODO(alexander): handle escape of dollar sign \$ or maybe $$.
            if (*curr == '$') {
                if (*(curr + 1) == '$') {
                    curr += 2;
                    continue;
                }
                
                if (*(curr + 1) == '{') {
                    curr += 2;
                    char* variable = curr;
                    u32 count = eat_until(&curr, '}');
                    if (count > 0) {
                        arena_push_str(&buffer, last_push, (u32) (base - last_push));
                        cstr result = str_map_get(template_params, str_lit(variable, count));
                        if (result) {
                            arena_push_cstr(&buffer, result);
                        }
                        last_push = curr;
                    }
                }
            }
            
            *curr++;
        }
        
        if (last_push != curr - 1) {
            arena_push_str(&buffer, last_push, (u32) (curr - last_push - 1));
        }
        generated_html_code = str_lit((char*) buffer.base, (u32) buffer.curr_used);
    }
    
    // NOTE(alexander): write out the generated html
    str index_html_file = str_concat(output_directory, str_lit("index.html"));
    if (!write_entire_file(index_html_file, generated_html_code)) {
        printf("Failed to write out the generated html to `index.html`!\n");
    }
    
    // NOTE(alexander): write out the generated css
    str style_css_file = str_concat(output_directory, str_lit("style.css"));
    str generated_css_code = read_entire_file(str_concat(css_directory, str_lit("main.css")));
    if (!write_entire_file(style_css_file, generated_css_code)) {
        printf("Failed to write out the generated css to `style.css`!\n");
    }
    
    // NOTE(alexander): read javascript onload code
    str onload_js_code = read_entire_file(str_concat(template_directory, str_lit("onload.js")));
    
    // NOTE(alexander): write out the generated js
    str script_js_file = str_concat(output_directory, str_lit("script.js"));
    str generated_js_code = onload_js_code;
    generated_js_code = str_concat(onload_js_code, str_lit("function main() { console.log(\"Hello World!\"); }"));
    if (!write_entire_file(script_js_file, generated_js_code)) {
        printf("Failed to write out the generated css to `script.js`!\n");
    }
    
    printf("Generated website successfully!\n");
    
    return 0;
}
