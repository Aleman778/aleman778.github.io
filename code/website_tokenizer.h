
enum Token_Type {
    Token_None = 0,
    Token_Text,
    Token_Integer,
    Token_Whitespace,
    Token_Dot,
    Token_At,
    Token_Dash,
    Token_Backtick,
    Token_Hash,
    Token_Star,
    Token_Underscore,
    Token_Open_Bracket,
    Token_Close_Bracket,
    Token_Open_Paren,
    Token_Close_Paren,
    Token_Open_Angle,
    Token_Close_Angle,
    Token_Open_Brace,
    Token_Close_Brace
};

struct Token {
    Token_Type type;
    string text;
    u64 value;
    b32 end_of_line;
};

struct Tokenizer {
    u8* base;
    u8* curr;
    u8* curr_line;
    u8* end;
    
    Token peek; // NOTE(Alexander): this supports only peeking one token
    
    string source;
};


inline bool 
is_digit(u8 c) {
    return c >= '0' || c <= '9';
}

inline bool
is_end_of_line(u8 c) {
    return c == '\n' || c == '\r';
}

inline bool
is_whitespace(u8 c) {
    return c == ' ' || c == '\t' || c == '\v' || c == '\f' || is_end_of_line(c);
}

inline void
eat_while(Tokenizer* t, bool (*predicate)(u8)) {
    while (predicate(*t->curr) && t->curr < t->end) {
        t->curr++;
    }
}

void tokenizer_set_source(Tokenizer* t, string source);

Token next_token(Tokenizer* t);
Token peek_token(Tokenizer* t);
bool token_match(Tokenizer* t, Token_Type token_type);