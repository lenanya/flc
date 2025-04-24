#ifndef INCLUDE_PARSER_H
#define INCLUDE_PARSER_H
#include "lexer.h"

void error(char* expected, char* got) {
    fprintf(stderr, "[Error] Expected %s, got %s\n", expected, got);
    exit(1);
}

void parse_function(Function* f, Lexer* lex, Token* t) {
    bool done = false;
    while (!done) {
        Expression expr = {0};
        expr.expression_type = ET_FUNCTION_CALL;
        get_token(lex, t);
        expect_token_type(*t, TT_SYMBOL);
        expr.expression_value.function_name = t->token_value.TV_symbol;
        get_token(lex, t);
        expect_token_type(*t, TT_PUNCTUATION);
        if (t->token_value.TV_punct != '(') error("`(`", "sumn else"); // TODO FIX

        bool args = true;
        Token closingparenmaybe = next_token(lex);
        while (closingparenmaybe.token_value.TV_punct != ')' && args) {
            get_token(lex, t);
            switch (t->token_type) {
                case TT_EOF:
                    error("anything but EOF", "EOF"); // TODO FIX
                case TT_PUNCTUATION:
                    StringBuilder sb = {0};
                    da_append(&sb, t->token_value.TV_punct);
                    sb_append_null(&sb);
                    if (t->token_value.TV_punct == ')') {
                        args = false; 
                        goto endofargs; // oh nooo, scary goto
                    }
                    if (t->token_value.TV_punct != ',') error("`,`", sb.items);
                    break;
                case TT_INT_LIT:
                    Expression intlit = {
                        .expression_type = ET_INT_LIT,
                        .expression_value.intlit = t->token_value.TV_int_lit,
                    };
                    da_append(&(expr.func_args), intlit);
                    break;
                case TT_SYMBOL:
                    Token next = next_token(lex);
                    if (next.token_type != TT_PUNCTUATION) error("`,`, `(`, or `)`", TT_VIS[next.token_type]);
                    if (next.token_value.TV_punct == '(') {
                        Expression funcall = {
                            .expression_type = ET_FUNCTION_CALL,
                            .expression_value.function_name = t->token_value.TV_symbol,
                        };
                        TODO("Implement function calls in function calls");
                    } else {
                        Expression var = {
                            .expression_type = ET_VARIABLE,
                            .expression_value.variable_name = t->token_value.TV_symbol,
                        };
                        da_append(&(expr.func_args), var);
                    }
                    break;
            }
        }
        get_token(lex, t);
        if (t->token_type != TT_PUNCTUATION) {
            StringBuilder sb = {0};
            da_append(&sb, t->token_value.TV_punct);
            sb_append_null(&sb);
            error("`,` or `)`", sb.items);
        }
endofargs:
        get_token(lex, t);
        if (t->token_type != TT_PUNCTUATION) {
            if (t->token_type == TT_INT_LIT) error("`;`", "Integer");
            else if (t->token_type == TT_SYMBOL) error("`;`", t->token_value.TV_symbol);
            else error(";", "EOF");
        } 
        if (t->token_value.TV_punct != ';') {
            StringBuilder sb = {0};
            da_append(&sb, t->token_value.TV_punct);
            sb_append_null(&sb);
            error("`;`", sb.items);
        }

        da_append(f, expr);

        Token lastmaybe = next_token(lex);
        if (lastmaybe.token_type == TT_PUNCTUATION && lastmaybe.token_value.TV_punct == '}') done = true;
    }
}

void create_program(Program* p, Lexer* lex) {
    bool done = false;
    while (!done) {
        Token t = {0};
        get_token(lex, &t);
        expect_token_type(t, TT_SYMBOL);
        if (strcmp(t.token_value.TV_symbol, "fun") != 0) {
            error("keyword `fun`", t.token_value.TV_symbol);
        }

        get_token(lex, &t);
        expect_token_type(t, TT_SYMBOL);
        if (is_internal(t.token_value.TV_symbol)) {
            error("function name", t.token_value.TV_symbol);
        }

        Function f = {0};
        f.name = t.token_value.TV_symbol;

        get_token(lex, &t);
        expect_token_type(t, TT_PUNCTUATION);
        if (t.token_value.TV_punct != '(') {
            StringBuilder sb = {0};
            da_append(&sb, t.token_value.TV_punct);
            sb_append_null(&sb);
            error("`(`", sb.items);
        }

        // TODO: add args?
        get_token(lex, &t);
        expect_token_type(t, TT_PUNCTUATION);
        if (t.token_value.TV_punct != ')') {
            StringBuilder sb = {0};
            da_append(&sb, t.token_value.TV_punct);
            sb_append_null(&sb);
            error("`)`", sb.items);
        }

        get_token(lex, &t);
        expect_token_type(t, TT_PUNCTUATION);
        if (t.token_value.TV_punct != '{') {
            StringBuilder sb = {0};
            da_append(&sb, t.token_value.TV_punct);
            sb_append_null(&sb);
            error("`{`", sb.items);
        }

        parse_function(&f, lex, &t);

        get_token(lex, &t);
        expect_token_type(t, TT_PUNCTUATION);
        if (t.token_value.TV_punct != '}') {
            StringBuilder sb = {0};
            da_append(&sb, t.token_value.TV_punct);
            sb_append_null(&sb);
            error("`}`", sb.items);
        }

        da_append(p, f);

        if (next_token(lex).token_type == TT_EOF) done = true;
    }
}



#endif // INCLUDE_PARSER_H