
enum Code_Block_Language {
    
};

enum Dom_Node_Type {
    DomNode_Inline_Text,
    DomNode_Paragraph,
    DomNode_Link,
    DomNode_Date,
    DomNode_Code_Block,
};

enum Text_Style {
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
    Dom_Node* first;
};
