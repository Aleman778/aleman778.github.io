#include "website.h"

int
main(int argc, char* argv[]) {
    // TODO(alexander): change this with CLI args
    const str output_directory = str_lit("generated/");
    const str template_directory = str_lit("templates/");
    const str css_directory = str_lit("css/");
    const str js_directory = str_lit("js/");
    
    // NOTE(alexander): read the basic template
    str template_html_code = read_entire_file(str_concat(template_directory, str_lit("basic.html")));
    
    // NOTE(alexander): write out the generated html
    str index_html_file = str_concat(output_directory, str_lit("index.html"));
    str generated_html_code = template_html_code;
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
