#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

enum ttype { Eol, LeftParen, RightParen, Atom, Comment, NewLine, Error } ttype;
enum otype { ONil, OList, OAtom, OError} otype;

typedef struct token {
	enum ttype type;
	char * value;
	struct token * next;
} token;

typedef struct object {
	enum otype type;
	token * token;
	struct object * next;
} object;

token* token_new() {
	token* token = calloc(1, sizeof(token));
	return token;
}

void token_delete(token* token) {
	if (token) {
		if (token->value) {
			free(token->value);
		}
		free(token);
	}
}

void token_delete_recursive(token* head) {
	token *next = head->next;
	while (head) {
		token_delete(head);
		head = next;
		next = head->next;
	}
}

void token_print_recursive(token* token) {
	while (token) {
		/* printf(">"); */
		switch(token->type) {
			case Eol:
				break;
			case LeftParen:
				printf("(");
				break;
			case RightParen:
				printf(")");
				break;
			case Atom:
				printf("%s", token->value);
				break;
			case Comment:
				printf("%s", token->value);
				break;
			case NewLine:
				printf("\n");
				break;
			case Error:
				printf("Error %s\n", token->value);
				break;
		}
		if (token->next
				&& token->next->type != RightParen
				&& token->type != LeftParen
				&& token->next->type != NewLine) {
			printf(" ");
		}
		/* printf("<"); */
		token = token->next;
	}
}

token * tokenize(char * param) {
	/* Algorithm invariant - start with dummy token, at the end remove */
	token *head = token_new();
	token *current = head;
	token *next = NULL;
	int indent = 0;

	while (*param) {
		if (*param == ' '
		    || *param == '\t') {
			param++;
		} else if (*param == '(') {
			param++;
			indent++;
			next = token_new();
			next->type = LeftParen;
			current->next = next;
			current = next;
		} else if (*param == ')') {
			param++;
			indent--;
			next = token_new();
			next->type = RightParen;
			current->next = next;
			current = next;
		} else if (*param == '"') {
			int len = 1;
			char *pointer = param;
			param++; /* consume first quote */
			/* TODO: change into one pointer, length, and assigment */
			while (*param != '"' && *(param-1) != '\\') {
				param++;
				len++;
			}
			param++; /* consume last quote */
			len++;
			char *value = malloc(len + 1);
			value[len] = '\0';
			strncpy(value, pointer, len);

			next = token_new();
			next->type = Atom;
			next->value = value;
			current->next = next;
			current = next;
		} else if (*param == ';') {
			int len = 0;
			char *pointer = param;
			while (*param != '\n' && *param != '\r') {
				param++;
				len++;
			}
			while (*param == '\n' && *param == '\r') {
				param++;
			}
			char *value = malloc(len + 1);
			value[len] = '\0';
			strncpy(value, pointer, len);
			next = token_new();
			next->type = Comment;
			next->value = value;
			current->next = next;
			current = next;
		} else if (*param == '\r' || *param == '\n') {
			param++;
			while (*param == '\n' && *param == '\r') {
				param++;
			}
			next = token_new();
			next->type = NewLine;
			current->next = next;
			current = next;
		} else {
			int len = 0;
			char *pointer = param;
			while (*param && *param != ' ' && *param != '\t' && *param != ')') {
				param++;
				len++;
			}
			char * value = malloc(len+1);
			strncpy(value, pointer, len);
			next = token_new();
			next->type = Atom;
			next->value = value;
			current->next = next;
			current = next;
		}

	}

	if (indent) {
		token * error = token_new();
		error->type = Error;
		char * value = malloc(100);
		sprintf(value, "Unmached parentheses: %d", indent);
		error->value = value;
		return error;
	}

	/* Remove invariant dummy head, and return real head */
	current = head->next;
	token_delete(head);
	return current;
}

object * read_atom(token * head) {
	return NULL;
}

object * read_list(token * head) {
	return NULL;
}

object * read_form(token * head) {
	return NULL;
}

token * read(char * param) {
	token * head = tokenize(param);
	return head;

	/* int len = strlen(param); */
	/* char *out = malloc(len+1); */
	/* char *cpy = out; */
	/*  */
	/* while (*param) { */
	/* 	*cpy++ = *param++; */
	/* } */
	/* return out; */
}

void* eval(void * param) {
	return param;
}

void print(token* head) {
	token_print_recursive(head);
	/* printf("%s\n", (char *)param); */
	/* return (char*)param; */
}

void rep(void * param) {
	param = read(param);
	param = eval(param);
	print(param);
}



int main(int argc, char** argv) {
	(void) argc;
	(void) argv;

	char * str = NULL;
	size_t size = 0;


	printf("user> ");
	while (getline(&str, &size, stdin) != -1) {
		rep(str);
		printf("user> ");
	}

	free(str);

	return 0;
}
