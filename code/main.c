#include "generator.h"


typedef union {
    struct {
        string stylesheet_path;
        string script_path;
        string content;
    };
    string data[3];
    
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
    
    Template_Parameters params;
    params.stylesheet_path = string_lit("assets/style.css");
    params.script_path = string_lit("assets/script.js");
    params.content = html;
    
    string template_html = read_entire_file("assets/base_template.html");
    string result = template_process_string(template_html, array_count(params.data), params.data);
    printf("Generated:\n%.*s\n", (int) result.count, result.data);
    write_entire_file("generated/generated.html", result);
}