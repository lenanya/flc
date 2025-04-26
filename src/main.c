#include "flc.h"
#include "parser.h"
#include <syscall.h>

void compile_time_error(char* error) {
	fprintf(stderr, "[ERROR] %s\n", error);
}

void compile_time_error_and_exit(char* error) {
	fprintf(stderr, "[ERROR] %s\n", error);
	exit(1);
}

// builtin function to set local variables :3
int f_set(Expression expr, FunctionVariables* func_vars, FILE* outfile) {
	if (!(expr.func_args.count == 2)) {
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
	fprintf(outfile, "\tsub $8, %rsp\n");
	int64_t value = expr.func_args.items[1].expression_value.intlit;
	fprintf(outfile, "\tmovq $%d, (%rsp)\n", value);
	return 0;
}

int f_ret(Expression expr, FunctionVariables* func_vars, FILE* outfile) {
	fprintf(outfile, "\tadd $%zu, %rsp\n", func_vars->count * 8);
	fprintf(outfile, "\tpop %rbp\n");
	fprintf(outfile, "\tmovq $0, %rax\n");
	fprintf(outfile, "\tret\n");
	return 0;
}

char* registers[] = {
	"%rdi",
	"%rsi",
	"%rdx",
	"%rcx",
	"%r8",
	"%r9",
};

// TODO: add all registers

void f_ext_undef_var(char* func_name, char* var_name) {
	fprintf(
		stderr,
		"Calling to external function `%s` with undefined variable `%s`\n",
		func_name,
		var_name
	);
}

int f_ext(Expression expr, FunctionVariables* func_vars, FILE* outfile) {
	char* function_name = expr.expression_value.function_name;
	for (size_t i = 0; i < expr.func_args.count; ++i) {
		Expression curr_expr = expr.func_args.items[i];
		switch (curr_expr.expression_type) {
			case ET_VARIABLE:
				char* var_name = curr_expr.expression_value.variable_name;
				int var_idx = variable_get_index(func_vars, var_name);
				if (var_idx < 0) {
					f_ext_undef_var(function_name, var_name);
					return 1;
				}
				if (var_idx > 0) {
					size_t offset = (var_idx + 1) * 8;
					char* reg = registers[i];
					fprintf(outfile, "\tmovq -%d(%rbp), %s\n", offset, reg);
				} else {
					fprintf(outfile, "\tmovq -8(%rbp), %s\n", registers[i]);
				}
				break;
			case ET_INT_LIT:
				int64_t int_lit = curr_expr.expression_value.intlit;
				fprintf(outfile, "\tmovq $%d, %s\n", int_lit, registers[i]);
				break;
			default:
				printf("%s\n", ET_VIS[curr_expr.expression_type]);
				TODO("external function args\n");
				break;
		}
	}
	fprintf(outfile, "\tcall %s\n", function_name);
	return 0;
}

int compile_function(Function* func, FILE* outfile) {
	// function setup
	fprintf(outfile, ".globl %s\n", func->name);
	fprintf(outfile, "%s:\n", func->name);
	fprintf(outfile, "\tpush %rbp\n");
	fprintf(outfile, "\tmovq %rsp, %rbp\n");
	// function body
	FunctionVariables func_vars = {0};

	for (size_t i = 0; i < func->count; ++i) {
		Expression curr_expr = func->items[i];
		switch (curr_expr.expression_type) {
			case ET_FUNCTION_CALL:
				char* function_name = curr_expr.expression_value.function_name;
				if (strcmp(function_name, "set") == 0) {
					if (f_set(curr_expr, &func_vars, outfile) != 0) 
						return 1;
				} else if (strcmp(function_name, "ret") == 0) {
					if (f_ret(curr_expr, &func_vars, outfile) != 0) 
						return 1;
				} else {
					if (f_ext(curr_expr, &func_vars, outfile) != 0) 
						return 1;
				}
				break;
			default:
				TODO("expression types\n");
		}
	}
	return 0;
}

// todo: move to string builder instead of stdout
int compile_program(Program* program, FILE* outfile) {
	int main_index = program_get_function_index(program, "main");
	if (main_index < 0) {
		compile_time_error("No main function defined.");
		return 1;
	}
	fprintf(outfile, ".section .text\n");
	for (size_t i = 0; i < program->count; ++i) {
		if (compile_function(&program->items[i], outfile) != 0) 
			return 1;
	}
	return 0;
}

int main(int argc, char** argv) {
	if (argc < 2) {
		fprintf(stderr, "[ERROR] Need to supply .fl file");
		return 1;
	}
	char* fl_file = argv[1];
	char* outfile = "flcout.S";
	char* executable = "flcout";
	if (argc > 3) {
		if (strcmp(argv[2], "-o") == 0) {
			executable = argv[3];
			outfile = malloc(strlen(executable) + 2);
			sprintf(outfile, "%s.S", executable);
		}
	}

	StringBuilder sb = {0};
	read_entire_file(&sb, fl_file);
	printf("[INFO] Read file %s\n", fl_file);

	Lexer lex = {0};
	create_lexer(&lex, sb.items);
	printf("[INFO] Tokenized, %d tokens\n", lex.tokens.count);

	dump_tokens(&lex);

	Program prog = {0};
	create_program(&prog, &lex);
	printf("[INFO] Parsed program, Compiling...\n");

	if (compile_program(&prog, fopen(outfile, "w"))) 
		return 1;

	printf("[INFO] Assembly written to %s\n", outfile);
	printf("[INFO] Compiling Assembly with CC\n");

	StringBuilder cmd = {0};
	sb_append_cstr(&cmd, "cc -no-pie ");
	sb_append_cstr(&cmd, outfile);
	sb_append_cstr(&cmd, " -o ");
	sb_append_cstr(&cmd, executable);
	sb_append_cstr(&cmd, "\n\0");

	printf("run this to get an executable:\n\n%s\n\n", cmd.items);

	return 0;
}