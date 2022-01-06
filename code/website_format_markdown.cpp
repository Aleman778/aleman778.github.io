
internal Dom_Node*
push_dom_node(Memory_Arena* arena, Dom_Node* prev = 0) {
    Dom_Node* result = push_struct(arena, Dom_Node);
    if (prev != 0) {
        prev->next = result;
    }
    return result;
}

Dom_Node*
read_markdown_text(Tokenizer* t, Memory_Arena* arena) {
    Dom_Node* result = push_dom_node(arena);
    Dom_Node* curr_node = result;
    
    while (true) {
        Token token = next_token(t);
        if (token.type == Token_None) {
            break;
        }
        
        if (token.type == Token_Star) {
            curr_node = push_dom_node(arena, curr_node);
            
            Token peek = peek_token(t);
            if (token_match(t, Token_Star)) {
                set_bitflag(curr_node->text_style, TextStyle_Bold);
            } else {
                set_bitflag(curr_node->text_style, TextStyle_Italics);
            }
        } else {
            
            
        }
        
    }
    
    return result;
}

Dom
read_markdown(Tokenizer* t, Memory_Arena* arena) {
    Dom result = {};
    
    
    result.root = push_dom_node(arena);
    Dom_Node* node = result.root;
    
    while (true) {
        Token token = peek_token(t);
        if (token.type == Token_None) {
            break;
        }
        
        switch (token.type) {
            case Token_Text: {
                Dom_Node* next = read_markdown_text(t, arena);
                node->next = next;
                node = next;
            } break;
            
            case Token_Hash: {
                s32 heading_level = 0;
                while (token_match(t, Token_Hash)) heading_level++;
                Dom_Node* next = read_markdown_text(t, arena);
                node->next = next;
                node = next;
                node->type = Dom_Heading;
                node->heading.level = heading_level;
            } break;
        }
    }
    
    return result;
}
