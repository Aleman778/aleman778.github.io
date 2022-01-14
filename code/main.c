#include "generator.h"

typedef struct {
    string defaultBgColor;
    string defaultFgColor;
    
    string primaryBgColor;
    string primaryFgColor;
    
    string darkBgColor;
    string darkFgColor;
} Theme_Parameters;


typedef struct {
    string stylesheet_path;
    string script_path;
    string content;
} Template_Parameters;

size_t
string_hash(string str) {
    size_t hash = 5381;
    
    for (int i = 0; i < str.count; i++) {
        hash = (hash * 33) ^ (size_t) str.data[i];
    }
    
    return hash;
}



int
main(int argc, char* argv[]) {
    char* filename = "pages/hello_world.md";
    Dom dom = read_markdown_file(filename);
    string html = generate_html_from_dom(&dom);
    //printf("Generated:\n%.*s\n", (int) html.count, html.data);
    
    // NOTE(Alexander): copy over the assets to generated dir
    {
        string raw_css = read_entire_file("assets/style.css");
        
        Theme_Parameters theme;
        theme.defaultBgColor = string_lit("rgb(30, 31, 32)");
        theme.defaultFgColor = string_lit("rgb(230, 230, 230)");
        
        theme.primaryBgColor = string_lit("rgb(74, 150, 217)");
        theme.primaryFgColor = theme.defaultFgColor;
        
        theme.darkBgColor = string_lit("rgb(20, 21, 22)");
        theme.darkFgColor = theme.defaultFgColor;
        
        string result = template_process_string(raw_css, 6, (string*) &theme);
        write_entire_file("generated/assets/style.css", result);
        free(raw_css.data);
        free(result.data);
    }
    
    copy_file("assets/script.js", "generated/assets/script.js");
    
    
    Template_Parameters params;
    params.stylesheet_path = string_lit("assets/style.css");
    params.script_path = string_lit("assets/script.js");
    params.content = html;
    
    string template_html = read_entire_file("assets/base_template.html");
    string result = template_process_string(template_html, 3, (string*) &params);
    printf("Generated:\n%.*s\n", (int) result.count, result.data);
    write_entire_file("generated/index.html", result);
}