
enum Code_Block_Language {
    CodeBlockLanguage_None,
    CodeBlockLanguage_C
};

enum Dom_Node_Type {
    Dom_None,
    Dom_Inline_Text,
    Dom_Paragraph,
    Dom_Heading,
    Dom_Link,
    Dom_Date,
    Dom_Code_Block,
};

typedef s32 Text_Style;
enum {
    TextStyle_None = 0,
    TextStyle_Italics = bit(0),
    TextStyle_Bold = bit(1),
    TextStyle_Code = bit(2),
};

struct Dom_Node {
    Dom_Node_Type type;
    
    string text;
    Text_Style text_style;
    
    Dom_Node* next;
    s32 depth;
    
    union {
        struct {
            Dom_Node* first_item;
        } paragraph;
        
        struct {
            s32 level;
        } heading;
        
        struct {
            Dom_Node* first_item;
        } unordered_list;
        
        struct {
            Dom_Node* second_item;
        } ordered_list;
        
        struct {
            string source;
        } link;
        
        struct {
            s32 year;
            s32 month;
            s32 day;
        } date;
        
        struct {
            Code_Block_Language language;
        } code_block;
    };
};


struct Dom {
    Dom_Node* root;
};
