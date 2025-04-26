#ifndef INCLUDE_FLC_H
#define INCLUDE_FLC_H
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <stdbool.h>
#include <inttypes.h>

typedef enum ExpressionType {
	ET_INT_LIT,
	ET_FUNCTION_CALL,
	ET_VARIABLE,
} ExpressionType;

const char* ET_VIS[] = {
	"ET_INT_LIT",
	"ET_FUNCTION_CALL",
	"ET_VARIABLE",
};

typedef union ExpressionValue {
	int64_t intlit;
	char* function_name;
	char* variable_name;
} ExpressionValue;

typedef struct Expression Expression;

typedef struct FunctionArgs {
	Expression* items;
	size_t count;
	size_t capacity;
} FunctionArgs;

typedef struct Expression {
	ExpressionType expression_type;
	ExpressionValue expression_value;
	FunctionArgs func_args;
} Expression;

typedef struct Function {
	Expression* items;
	char* name;
	size_t count;
	size_t capacity;
} Function;

typedef struct Program {
	Function* items;
	size_t count;
	size_t capacity;
} Program;

typedef enum VariableType {
	VT_INT,
} VariableType;

typedef struct Variable {
	char* name;
	VariableType variable_type;
	size_t stack_index;
} Variable;

typedef struct FunctionVariables {
	Variable* items;
	size_t count;
	size_t capacity;
} FunctionVariables;

#define DA_INITIAL_CAPACITY 16

#define TODO(str) do {fprintf(stderr, "[TODO]: %s\n", str); exit(1);} while (0);

#define da_append(da, item) do { \
	if ((da)->count >= (da)->capacity) { \
		if ((da)->capacity == 0) { \
			(da)->capacity = DA_INITIAL_CAPACITY; \
			(da)->items = malloc(sizeof(*(da)->items) * (da)->capacity); \
		} else { \
			(da)->capacity *= 2; \
			(da)->items = realloc((da)->items, sizeof(*(da)->items) * (da)->capacity); \
		} \
	} \
	(da)->items[(da)->count++] = item; \
} while (0)


int variable_get_index(FunctionVariables* func_vars, char* name) {
	for (int i = 0; i < func_vars->count; ++i) {
		if (strcmp(func_vars->items[i].name, name) == 0) return i;
	}
	return -1;
}

int program_get_function_index(Program *program, char* function_name) {
	for (size_t i = 0; i < program->count; ++i) {
		if (strcmp(function_name, program->items[i].name) == 0) return i;
	}
	return -1;
}

typedef struct StringBuilder {
	char* items;
	size_t count;
	size_t capacity;
} StringBuilder;

#define array_len(xs) (sizeof(xs)/sizeof(xs[0]))
#define da_free(da) free((da).items)
#define sb_append_null(da) da_append((da), '\0');

void read_entire_file(StringBuilder* sb, char* file_path) {
	FILE* file = fopen(file_path, "r");
	if (!file) {
		fprintf(stderr, "[ERROR]: Could not open file %s\n", file_path);
		exit(1);
	}
	fseek(file, 0, SEEK_END);
	size_t size = ftell(file);
	fseek(file, 0, SEEK_SET);
	size_t new_count = sb->count + size;
	if (new_count > sb->capacity) {
		sb->items = realloc(sb->items, new_count);
		sb->capacity = new_count;
	}
	fread(sb->items + sb->count, size, 1, file);
	fclose(file);
	sb->count = new_count;
	sb_append_null(sb);
}

void sb_append_cstr(StringBuilder* sb, char* cstr) {
	int len = strlen(cstr);
	for (size_t i = 0; i < len; ++i) {
		da_append(sb, cstr[i]);
	}
}

#endif // INCLUDE_FLC_H