#include "flc.h"

void compile_time_error(char* error) {
    fprintf(stderr, "[ERROR] %s\n", error);
}

void compile_time_error_and_exit(char* error) {
    fprintf(stderr, "[ERROR] %s\n", error);
    exit(1);
}

// builtin function to set local variables :3
int f_set(Expression expr, FunctionVariables* func_vars) {
    if (!expr.func_args.count == 2) {
        compile_time_error("Too many arguments in call to `set`");
        return 1;
    }
    if (expr.func_args.items[0].expression_type != ET_VARIABLE) {
        compile_time_error("First argument to `set` isn't a variable name");
        return 1;
    }
    // TODO: actually support assignment of things other than int
    Variable var = {
        .name = expr.func_args.items[0].expression_value.variable_name,
        .variable_type = VT_INT,
    };

    da_append(func_vars, var);

    printf("    #Creating variable `%s`\n", var.name);
    printf("    sub $8, %rsp\n");
    printf("    movq $%d, (%rsp)\n", expr.func_args.items[1].expression_value.intlit);
    return 0;
}

char* registers[] = {
    "%rdi",
    "%rsi",
};

int f_ext(Expression expr, FunctionVariables* func_vars) {
    char* function_name = expr.expression_value.function_name;
    for (size_t i = 0; i < expr.func_args.count; ++i) {
        switch (expr.func_args.items[i].expression_type) {
            case ET_VARIABLE: 
                char* var_name = expr.func_args.items[i].expression_value.variable_name;
                int var_idx = variable_get_index(func_vars, var_name);
                if (var_idx < 0) {
                    fprintf(stderr, "Calling to external function `%s` with undefined variable `%s`\n", function_name, var_name);
                    return 1;
                }
                printf("    #loading variable `%s`\n", var_name);
                if (var_idx > 0) {
                    printf("    movq -%d(%rbp), %s\n", (var_idx + 1) * 8, registers[i]);
                } else {
                    printf("    movq -8(%rbp), %s\n", registers[i]);
                }
                break;
            case ET_INT_LIT:
                printf("    movq $%d, %s\n", expr.func_args.items[i].expression_value.intlit, registers[i]);
            default:
                TODO("external function args\n");
                break;
        }
    }
    printf("    call %s\n", function_name);
    return 0;
}

int compile_function(Function* func) {
    // function setup
    printf(".globl %s\n", func->name);
    printf("%s:\n", func->name);
    printf("    push %rbp\n");
    printf("    movq %rsp, %rbp\n");
    // function body
    FunctionVariables func_vars = {0};
    
    for (size_t i = 0; i < func->count; ++i) {
        switch (func->items[i].expression_type) {
            case ET_FUNCTION_CALL:
                if (strcmp(func->items[i].expression_value.function_name, "set") == 0) {
                    if (f_set(func->items[i], &func_vars) != 0) return 1;
                } else {
                    if (f_ext(func->items[i], &func_vars) != 0) return 1;
                }
                break; 
            default:
                TODO("expression types\n");
        }
    }

    //function cleanup
    printf("    add $%zu, %rsp\n", func_vars.count * 8);
    printf("    pop %rbp\n");
    printf("    movq $0, %rax\n");
    printf("    ret\n");
}

// todo: move to string builder instead of stdout
int compile_program(Program* program) {
    int main_index = program_get_function_index(program, "main");
    if (main_index < 0) {
        compile_time_error("No main function defined.");
        return 1;
    }
    printf(".section .text\n");
    for (size_t i = 0; i < program->count; ++i) {
        if (compile_function(&program->items[i]) != 0) return 1;
    }
}

int main(void) {

    // hardcoded program for now, since no lexing/parsing yet
    Program p = {0};

    Function f = {0};
    f.name = "main";

    Expression expr1 = {0};
    expr1.expression_type = ET_FUNCTION_CALL;
    expr1.expression_value.function_name = "set";
    Expression fe1 = {
        .expression_type = ET_VARIABLE,
        .expression_value.variable_name = "a",
    };
    Expression fe2 = {
        .expression_type = ET_INT_LIT,
        .expression_value.intlit = 69,
    };

    da_append(&(expr1.func_args), fe1);
    da_append(&(expr1.func_args), fe2);
    da_append(&f, expr1);

    Expression expr2 = {0};
    expr2.expression_type = ET_FUNCTION_CALL;
    expr2.expression_value.function_name = "putchar";
    Expression fe21 = {
        .expression_type = ET_VARIABLE,
        .expression_value.variable_name = "a",
    };
    da_append(&(expr2.func_args), fe21);
    da_append(&f, expr2);


    da_append(&p, f);

    compile_program(&p);

    return 0;
}