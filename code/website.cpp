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

int
main(int argc, char* argv[]) {
    const str output_directory = str_lit("generated/");
    const str template_directory = str_lit("templates/");
    const str css_directory = str_lit("css/");
    const str js_directory = str_lit("js/");
    const str docs_directory = str_lit("docs/");
    
    // NOTE(alexander): read the basic template
    str template_html_code = read_entire_file(str_concat(template_directory, str_lit("basic.html")));
    
    struct { cstr key; cstr value; }* template_params = 0;
    str_map_put(template_params, "script_filepath", "script.js");
    str_map_put(template_params, "style_filepath", "style.css");
    str_map_put(template_params, "body", "Hello World!");
    
    // TODO(alexander): template processing
    str generated_html_code;
    {
        Arena buffer = {};
        
        char* curr = template_html_code;
        char* last_push = curr;
        while (*curr != '\0') {
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
                        char* result = (char*) str_map_get(template_params, str_lit(variable, count));
                        if (result) {
                            printf(result);
                            arena_push_string(&buffer, result, strlen(result));
                        }
                        last_push = curr;
                    }
                }
            }
            
            *curr++;
        }
        
        if (last_push != curr) {
            arena_push_string(&buffer, last_push, (u32) (curr - last_push));
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
