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
            
            arena_push_string(buffer, last_push, (u32) (curr - last_push));
            curr += bold_font ? 2 : 1;
            last_push = curr;
            
            if (bold_font) {
                arena_push_cstring(buffer, "<b>");
            } else {
                arena_push_cstring(buffer, "<i>");
            }
            
            u32 count = eat_until(&curr, '*');
            arena_push_string(buffer, last_push, count);
            curr += bold_font ? 1 : 0;
            last_push = curr;
            
            
            if (bold_font) {
                arena_push_cstring(buffer, "</b>");
            } else {
                arena_push_cstring(buffer, "</i>");
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
        arena_push_string(buffer, last_push, (u32) (curr - last_push));
    }
    
    *scanner = curr;
}

int
main(int argc, char* argv[]) {
    const string output_directory = string_lit("generated/");
    const string template_directory = string_lit("templates/");
    const string css_directory = string_lit("css/");
    const string js_directory = string_lit("js/");
    const string docs_directory = string_lit("docs/");
    
    struct{ cstring key; cstring value; }* template_params = 0;
    string_map_put(template_params, "script_filepath", "script.js");
    string_map_put(template_params, "style_filepath", "style.css");
    
    
    // NOTE(alexander): markdown to html converter
    {
        string filepath = string_concat(docs_directory, string_lit("hello_world.md"));
        string markdown = read_entire_file(filepath);
        
        string heading_begin = string_lit("<h0>");
        string heading_end = string_lit("</h0>");
        Arena buffer = {};
        
        s32 prev_depth = 0;
        
        string* scopes = 0;
        
        char* curr = markdown;
        while (*curr) {
            
            char* begin_line = curr;
            
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
                
                string begin_scope = 0;
                string end_scope = 0;
                
                bool create_heading = false;
                char heading_level = '0';
                
                bool create_list_item = false;
                
                string image_alt_text= 0;
                string image_link = 0;
                bool create_image = false;
                
                
                if (*curr == '#') {
                    
                    char h = '0';
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
                    
                    char* str = curr;
                    s32 count = eat_until(&curr, ']');
                    image_alt_text = string_lit(str, count);
                    
                    if (*curr == '(') {
                        curr++;
                        str = curr;
                        count = eat_until(&curr, ')');
                        image_link= string_lit(str, count);
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
                
                while (prev_depth < depth && begin_scope && end_scope) {
                    prev_depth++;
                    arena_push_string(&buffer, begin_scope);
                    array_push(scopes, end_scope);
                }
                
                while (depth < prev_depth) {
                    prev_depth--;
                    end_scope = array_pop(scopes);
                    arena_push_string(&buffer, end_scope);
                }
                
                if (depth > 0) {
                    depth--;
                }
                
                if (create_heading) {
                    heading_begin[2] = heading_level;
                    heading_end[3] = heading_level;
                    arena_push_string(&buffer, heading_begin);
                    generate_markdown_text(&buffer, &curr);
                    arena_push_string(&buffer, heading_end);
                    continue;
                } else if (create_list_item) {
                    arena_push_cstring(&buffer, "<li>");
                    generate_markdown_text(&buffer, &curr);
                    arena_push_cstring(&buffer, "</li>");
                    continue;
                } else if (create_image) {
                    arena_push_cstring(&buffer, "<img src=\"");
                    arena_push_string(&buffer, image_link);
                    
                    arena_push_cstring(&buffer, "\" alt=\"");
                    arena_push_string(&buffer, image_alt_text);
                    
                    arena_push_cstring(&buffer, "\">");
                    continue;
                } else {
                    curr = begin_line;
                }
                
            }
            
            arena_push_cstring(&buffer, "<p>");
            generate_markdown_text(&buffer, &curr, true);
            arena_push_cstring(&buffer, "</p>");
        }
        
        string result = string_lit((char*) buffer.base, (u32) buffer.curr_used);
        printf(markdown);
        printf(result);
        string_map_put(template_params, "body", result);
    }
    
    // NOTE(alexander): template processing
    string generated_html_code;
    {
        // NOTE(alexander): read the basic template
        string filepath = string_concat(template_directory, string_lit("basic.html"));
        string template_code = read_entire_file(filepath);
        
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
                        arena_push_string(&buffer, last_push, (u32) (base - last_push));
                        cstring result = string_map_get(template_params, string_lit(variable, count));
                        if (result) {
                            arena_push_cstring(&buffer, result);
                        }
                        last_push = curr;
                    }
                }
            }
            
            *curr++;
        }
        
        if (last_push != curr - 1) {
            arena_push_string(&buffer, last_push, (u32) (curr - last_push - 1));
        }
        generated_html_code = string_lit((char*) buffer.base, (u32) buffer.curr_used);
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
