#ifndef INCLUDE_LEXER_H
#define INCLUDE_LEXER_H
#include "flc.h"
#include <ctype.h>

typedef enum TokenType {
    TT_INT_LIT,
    TT_SYMBOL,
    TT_PUNCTUATION,
    TT_EOF,
} TokenType;

char* TT_VIS[] = {
    "TT_INT_LIT",
    "TT_SYMBOL",
    "TT_PUNCTUATION",
    "TT_EOF",
};

typedef union TokenValue {
    int64_t TV_int_lit;
    char *TV_symbol;
    char TV_punct;
} TokenValue;

typedef struct Token {
    TokenType token_type;
    TokenValue token_value;
} Token;

typedef struct TokenList {
    Token *items;
    size_t count;
    size_t capacity;
} TokenList;

typedef struct Lexer {
    int index;
    TokenList tokens;
} Lexer;

void get_token(Lexer* lex, Token* t) {
    if (lex->index > lex->tokens.count) {
        fprintf(stderr, "[ERROR]: Lexer ran out of tokens\n");
        exit(1);
    }
    t->token_type = lex->tokens.items[lex->index].token_type;
    t->token_value = lex->tokens.items[lex->index].token_value;

    lex->index++;
}

Token next_token(Lexer* lex) {
    if (lex->index > lex->tokens.count) {
        fprintf(stderr, "[ERROR]: Lexer ran out of tokens\n");
        exit(1);
    }
    Token t;
    t.token_type = lex->tokens.items[lex->index].token_type;
    t.token_value = lex->tokens.items[lex->index].token_value;
    return t;
}

void expect_error(Token t, TokenType tt) {
    char recd_type[256];
    switch (t.token_type) {
        case TT_EOF:
            sprintf(recd_type, "%s", "EOF");
            break;
        case TT_INT_LIT:
            sprintf(recd_type, "Int literal: %ld", t.token_value.TV_int_lit);
            break;
        case TT_PUNCTUATION:
            sprintf(recd_type, "Punctuation: '%c'", t.token_value.TV_punct);
            break;
        case TT_SYMBOL:
            sprintf(recd_type, "Symbol: \"%s\"", t.token_value.TV_symbol);
            break;
    }
    switch (tt) {
        case TT_EOF:
            fprintf(stderr, "[ERROR] Expected token of type EOF got `%s`\n", recd_type);
            break;
        case TT_INT_LIT:
            fprintf(stderr, "[ERROR] Expected Int literal got `%s`\n", recd_type);
            break;
        case TT_PUNCTUATION:
            fprintf(stderr, "[ERROR] Expected Punctuation got `%s`\n", recd_type);
            break;
        case TT_SYMBOL:
            fprintf(stderr, "[ERROR] Expected Symbol got `%s`\n", recd_type);
            break;
    }
    exit(1);
}

void expect_token_type(Token t, TokenType tt) {
    if (t.token_type != tt) 
        expect_error(t, tt);

}

char PUNCTUATION[] = {
    ';',
    ',',
    '(',
    ')',
    '{',
    '}',
};

bool is_punct(char c) {
    for (size_t i = 0; i < array_len(PUNCTUATION); ++i)
        if (c == PUNCTUATION[i])
            return true;
    return false;
}

static char* get_tokenstring(char* string, size_t* index, StringBuilder* result) {
    result->count = 0;
    while ((*index) < strlen(string) && isspace(string[*index])) {
        (*index)++;
    }

    if ((*index) < strlen(string) && is_punct(string[*index])) {
        da_append(result, string[*index]);
        sb_append_null(result);
        (*index)++;
        return result->items;
    }

    while ((*index) < strlen(string) && !is_punct(string[*index]) && !isspace(string[*index])) {
        da_append(result, string[*index]);
        (*index)++;
    }
    sb_append_null(result);
    return result->items;
}

typedef struct TokenStrings {
    char** items;
    size_t count;
    size_t capacity;
} TokenStrings;

char *INTERNAL[] = {
    "set",
    "ret",
    "add",
    "sub",
    "fun",
};

bool is_internal(char *str) {
    for (size_t i = 0; i < array_len(INTERNAL); ++i) {
        if (strcmp(str, INTERNAL[i]) == 0)
            return true;
    }
    return false;
}

Token punct_token(char c) {
    return (Token) {
        .token_type = TT_PUNCTUATION,
        .token_value.TV_punct = c,
    };
}

Token int_token(int64_t i) {
    return (Token) {
        .token_type = TT_INT_LIT,
        .token_value.TV_int_lit = i,
    };
}

Token symbol_token(char* s) {
    return (Token) {
        .token_type = TT_SYMBOL,
        .token_value.TV_symbol = s,
    };
}

Token eof_token() {
    return (Token) {
        .token_type = TT_EOF,
    };
}

bool is_int_lit(char* str) {
    for (size_t i = 0; i < strlen(str); ++i) if (!isdigit(str[i])) return false;
    return true;
}

void tokenize(Lexer *lex, char* source) {
    size_t index = 0;
    StringBuilder sb = {0};
    TokenStrings tokenstrings = {0};
    size_t source_length = strlen(source);
    while (index < source_length) {
        char* tokstr = get_tokenstring(source, &index, &sb);
        char* heap_string = strdup(tokstr);
        da_append(&tokenstrings, heap_string);
    }
    da_free(sb);

    for (size_t i = 0; i < tokenstrings.count; ++i) {
        if (is_punct(tokenstrings.items[i][0])) {
            da_append(&lex->tokens, punct_token(tokenstrings.items[i][0]));
        } else if (is_int_lit(tokenstrings.items[i])) {
            int64_t val = strtoll(tokenstrings.items[i], NULL, 10);
            da_append(&lex->tokens, int_token(val));
        } else {
            da_append(&lex->tokens, symbol_token(tokenstrings.items[i]));
        }
    }
    da_append(&lex->tokens, eof_token());
}

void dump_tokens(Lexer* lex) {
    printf("[TOKENS] -----------\n");
    for (size_t i = 0; i < lex->tokens.count; ++i) {
        switch (lex->tokens.items[i].token_type) {
            case TT_SYMBOL:
                printf("    [SYMBOL] : %ld : \"%s\"\n", i, lex->tokens.items[i].token_value.TV_symbol);
                break;
            case TT_INT_LIT:
                printf("    [INT]    : %ld : %"PRId64"\n", i, lex->tokens.items[i].token_value.TV_int_lit);
                break;
            case TT_PUNCTUATION:
                printf("    [PUNCT]  : %ld : '%c'\n", i, lex->tokens.items[i].token_value.TV_punct);
                break;
            case TT_EOF:
                printf("    [EOF]\n");
                break;
        }
    }
    printf("----------- [/TOKENS]\n");
}

void create_lexer(Lexer* lex, char* source) {
    lex->index = 0;
    tokenize(lex, source);
}

#endif // INCLUDE_LEXER_H