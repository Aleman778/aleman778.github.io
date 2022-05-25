#include "generator.h"
#include <windows.h>

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
    string template_path;
} Template_Parameters;

Template_Parameters global_template_params;

void 
generate_page_from_markdown(string filename, string dest_filename) {
    Template_Parameters params = global_template_params;
    
    string dest_dir = path_to_dir(dest_filename);
    //int folder_depth = dir_get_folder_depth(dest_dir);
    
    //string leave_dir = "../";
    //for (int i = 0; i < folder_depth; i++) {
    //leave_dir
    //}
    
    CreateDirectoryA(string_to_cstring(dest_dir), 0);
    Dom dom = read_markdown_file(filename);
    
    params.content = generate_html_from_dom(&dom);
    
    string template_html = read_entire_file(params.template_path);
    string result = template_process_string(template_html, 3, (string*) &params);
    //printf("Generated:\n%.*s\n", f_string(result));
    write_entire_file(dest_filename, result);
}


int
main(int argc, char* argv[]) {
    
    global_template_params.template_path = string_lit("assets/base_template.html");
    global_template_params.stylesheet_path = string_lit("/assets/style.css");
    global_template_params.script_path = string_lit("/assets/script.js");
    
    generate_page_from_markdown(string_lit("pages/home.md"), 
                                string_lit("generated/index.html"));
    generate_page_from_markdown(string_lit("pages/project_sqrrl.md"), string_lit("generated/projects/index.html"));
    //printf("Generated:\n%.*s\n", (int) html.count, html.data);
    
    
    {
        // NOTE(Alexander): proces and copy over the assets to generated/assets dir
        CreateDirectoryA("generated/assets", 0);
        string raw_css = read_entire_file(string_lit("assets/style.css"));
        
        Theme_Parameters theme;
        theme.defaultBgColor = string_lit("rgb(30, 31, 32)");
        theme.defaultFgColor = string_lit("rgb(230, 230, 230)");
        
        theme.primaryBgColor = string_lit("rgb(74, 150, 217)");
        theme.primaryFgColor = theme.defaultFgColor;
        
        theme.darkBgColor = string_lit("rgb(20, 21, 22)");
        theme.darkFgColor = theme.defaultFgColor;
        
        string result = template_process_string(raw_css, 6, (string*) &theme);
        write_entire_file(string_lit("generated/assets/style.css"), result);
        free(raw_css.data);
        free(result.data);
        
        copy_file(string_lit("assets/script.js"), string_lit("generated/assets/script.js"));
        copy_file(string_lit("assets/avatar.jpg"), string_lit("generated/assets/avatar.jpg"));
    }
}