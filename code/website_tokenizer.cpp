
void
tokenizer_set_source(Tokenizer* t, string source) {
    t->base = source.data;
    t->curr = t->base;
    t->curr_line = t->base;
    t->end = t->base + source.count;
    t->source = source;
}

Token
eat_integer(Tokenizer* t) {
    Token result;
    result.type = Token_Integer;
    const u64 base = 10;
    
    u64 value = 0;
    while (t->curr < t->end) {
        u8 c = *t->curr;
        u64 d = (u64) c - (u64) '0';
        assert(d >= 0 && d <= 9 && "tokenization error");
        u64 x = value * base;
        if (value != 0 && x / base != value) {
            assert(0 && "integer literal is too large");
        }
        
        u64 y = x + d;
        if (y < x) { // NOTE(alexander): this should work since d is small compared to x
            assert(0 && "integer literal is too large");
            break;
        }
        
        value = y;
        t->curr++;
    }
    result.value = value;
    
    return result;
}


Token
next_token(Tokenizer* t) {
    Token result = {};
    
    if (t->peek.type != Token_None) {
        result = t->peek;
        t->peek = {};
        return result;
    }
    
    u8* begin = t->curr;
    
    while (t->curr < t->end) {
        u8 c = *t->curr;
        
        if (is_whitespace(c)) {
            if (result.type != Token_None) {
                break;
            }
            result.type = Token_Whitespace;
            
            while (is_whitespace(*t->curr) && t->curr < t->end) {
                if (is_end_of_line(*t->curr)) {
                    result.end_of_line = true;
                    break;
                }
                t->curr++;
            }
        }
        
        if (is_digit(c)) {
            result = eat_integer(t);
        }
        
        switch (c) {
            case '.': result.type = Token_Dot; break;
            case '*': result.type = Token_Star; break;
            case '@': result.type = Token_At; break;
            case '#': result.type = Token_Hash; break;
            case '`': result.type = Token_Backtick; break;
            case '-': result.type = Token_Dash; break;
            case '_': result.type = Token_Underscore; break;
            case '[': result.type = Token_Open_Bracket; break;
            case ']': result.type = Token_Close_Bracket; break;
            case '(': result.type = Token_Open_Paren; break;
            case ')': result.type = Token_Close_Paren; break;
            case '<': result.type = Token_Open_Angle; break;
            case '>': result.type = Token_Close_Angle; break;
            case '{': result.type = Token_Open_Brace; break;
            case '}': result.type = Token_Close_Brace; break;
            default: result.type = Token_Text; break;
        }
        
        t->curr++;
    }
    
    result.text = string_view(begin, t->curr);
    
    return result;
}

Token peek_token(Tokenizer* t) {
    if (t->peek.type == Token_None) {
        t->peek = next_token(t);
    }
    return t->peek;
}

bool
token_match(Tokenizer* t, Token_Type token_type) {
    Token token = peek_token(t);
    if (token.type == token_type) {
        next_token(t);
        return true;
    }
    return false;
}
