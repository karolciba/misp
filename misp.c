#include <stdio.h>
#include <string.h>
#include <stdlib.h>

enum ttype { Eol, LeftParen, RightParen, Atom, Comment, NewLine, Error } ttype;
enum otype { ONil, OPair, OAtom, OList, OFunc, ODict, OInt, OError=255} otype;

/* TODO: replace with pairs usage */
typedef struct token {
	enum ttype type;
	char * value;
	struct token * next;
} token;

struct pair;

typedef struct pair * (*fn_ptr)(struct pair * pair, struct pair * env);

typedef struct pair {
	enum otype type;
	union {
		struct pair * value;
		char * cvalue; /* debug purp */
		int ivalue;
		fn_ptr fvalue;
		/* TODO: iplement internal string / char * */
		/* union { char c0; char c1; char c2; char c4; } string; */
	};
	struct pair * next;
} pair;

typedef struct atom {
	enum otype type;
	char * value;
	struct pair * next;
} atom;


/* #define is_pair(obj) ((long)((pair *)obj)->next % 2 == 0) */
/* #define is_atom(obj) ((long)((pair *)obj)->next == 7) */
#define is_pair(obj) (obj->type == OPair)
#define is_atom(obj) (obj->type == OAtom)
#define is_int(obj) (obj->type == OInt)
#define is_func(obj) (obj->type == OFunc)
#define is_list(obj) (obj->type == OList)
#define is_dict(obj) (obj->type == ODict)
#define is_nil(obj) (obj->type == ONil || (obj->type == OPair && !obj->value && !obj->next))

#define DEBUG 0
#define info(...) if (DEBUG) printf(__VA_ARGS__)

pair * pair_new(enum otype type) {
	pair * pair = calloc(1, sizeof(pair));
	pair->type = type;
	return pair;
}

void pair_delete(pair * pair) {
	free(pair);
}

void pair_print(pair *);

void pair_print(pair * pair) {
	if (is_int(pair)) {
		info("<int>");
		printf("%ld",(long) pair->value);
	} else if (is_atom(pair)) {
		info("<atom>");
		printf("%s", (char *)pair->value);
	} else if (is_nil(pair)) {
		info("<nil>");
		printf("()");
	} else if (is_pair(pair)) {
		info("<pair>");
		if (pair->value) {
			pair_print(pair->value);
		}
		if (pair->next) {
			printf(" ");
			pair_print(pair->next);
		}
		info("</pair>");
	} else if (is_list(pair)) {
		info("<pair>");
		printf("(");
		if (pair->value) {
			pair_print(pair->value);
		}
		/* if (pair->next) { */
		/* 	pair_print(pair->next); */
		/* } */
		printf(")");
		info("</pair>");
	} else if (is_dict(pair)) {
			info("<dict>");
			printf("{");
			if (pair->value) {
				pair_print(pair->value);
			}
			if (pair->next) {
				printf(" ");
				pair_print(pair->next);
			}
			printf("}");
			info("</dict>");
	} else if (is_func(pair)) {
		info("<func>");
		printf("<#func>");
		info("</func>");
	} else {
		info("<?!>");
		printf("?!");
	}
}

pair * nil() {
	pair * pair = pair_new(OPair);
	return pair;
}

pair * cons(pair * car, pair * cdr) {
	pair * pair = pair_new(OPair);
	pair->value = car;
	pair->next = cdr;

	return pair;
}

pair * car(pair * p) {
	if (is_list(p) && p->value) {
		return p->value->value;
	}

	return p->value;
}

pair * cdr(pair * p) {
	pair * l = pair_new(OList);
	if (is_list(p) && p->value) {
		l->value = p->value->next;
		return l;
	}

	return p->next;
}


pair * func_plus(pair * param, pair * env) {
	(void) param;
	(void) env;
	pair * augent = car(param);
	pair * addend = car(cdr(param));
	pair * pair = pair_new(OInt);
	pair->ivalue = augent->ivalue + addend->ivalue;
	return pair;
}

void dict_set(pair * param) {
	// param: (dict_set <list> <key> <value>)
	// dictionary is a list ( (key value) (keyA valueA) ... )
	// TODO: consider keeping it sorted? O(n) -> ~O(n/2)
	pair * list = car(param); // pick dictionary object
	param = cdr(param); // peel off dictionary object

	atom * key = (atom *)car(param);  // pick key object
	/* param = cdr(param); // peel off key object */
	pair * value = cdr(param); // pick value object

	pair * pointer;
	atom * tkey;
	while ((pointer = car(list))) {
		tkey = (atom *)car(pointer);
		if (strcmp(tkey->value, key->value) == 0) {
			cdr(pointer)->value = value;
			return;
		}
		if (cdr(list)) {
            list = cdr(list);
		} else {
			list->next = pair_new(OPair);
			list = list->next;
			break;
		}
	}

	list->value = cons((pair *)key, value);
}

pair *dict_get(pair * param) {
	// (dict_get <list> <key>) -> <value>/<nil>
	// dictionary is a list ( (key value) (keyA valueA) ... )
	pair * list = car(param);
	atom * key = (atom *)cdr(param);

	pair * pointer;
	atom * tkey;
	while (list && (pointer = car(list))) {
		tkey = (atom *)car(pointer);
		if (strcmp(tkey->value, key->value) == 0) {
			return cdr(pointer);
		}
		list = cdr(list);
	}

	return pair_new(OPair);
}


token * token_new() {
	token * token = calloc(1, sizeof(token));
	return token;
}

void token_delete(token * token) {
	if (token) {
		if (token->value) {
			free(token->value);
		}
		free(token);
	}
}

void token_delete_recursive(token * head) {
	token *next = head->next;
	while (head) {
		token_delete(head);
		head = next;
		next = head->next;
	}
}

/* TODO: split into print & recursive */
void token_print_recursive(token * token) {
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
			while (*param && *param != ' ' && *param != '\t' && *param != ')' && *param != '\n' && *param != '\r') {
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

pair * read_atom(token * head, pair * env) {
	pair * obj = pair_new(OInt);

	long i;
	char * endptr;

	/* try to parse a number */
	i = strtol(head->value, &endptr, 10);
	/* if not a number */
	if (*endptr != '\0') {
		obj->type = OAtom;
		obj->value = (pair *)strdup(head->value);
		return obj;
	}

	obj->value = (pair *) i;
	return obj;
}

pair * read_form(token *, pair * env);

pair * read_list(token * head, pair * env) {
	/* skip LeftParen */
	/* TODO: sanity check */

	head = head->next;
	pair * top = pair_new(OList);

	if (head && head->type == RightParen) {
		return top;
	}

	pair * pointer = pair_new(OPair);
	top->value = pointer;
	pair * element;
	while (head && head->type != RightParen) {
		element = read_form(head, env);
		pointer->value = element;
		if (!head->next || head->next->type == RightParen) {
			break;
		}
		pointer->next = pair_new(OPair);
		pointer = pointer->next;
		head = head->next;
	}

	return top;
}

pair * read_form(token * head, pair * env) {
	if (head->type == LeftParen) {
		return read_list(head, env);
	}

	return read_atom(head, env);
}

pair * read(char * param, pair * env) {
	token * head = tokenize(param);
	pair * obj = read_form(head, env);
	return obj;

	/* int len = strlen(param); */
	/* char *out = malloc(len+1); */
	/* char *cpy = out; */
	/*  */
	/* while (*param) { */
	/* 	*cpy++ = *param++; */
	/* } */
	/* return out; */
}

pair * eval(pair * param, pair * env);

pair * eval_ast(pair * param, pair * env) {
	if (is_atom(param)) {
		/* look into env */
		pair *value;
		pair *k = pair_new(OAtom);
		k->value = param->value;
		value = dict_get(cons(env,k));
		if (!is_nil(value)) {
			return value;
		}
		return param;

	} else if (is_list(param)) {
		pair * ret = eval(car(param), env);
		param = cdr(param);
		while (param) {
			ret = cons(ret, eval(car(param), env));
			param = cdr(param);
		}
		return ret;

	} else if (is_int(param)) {
		return param;
	} else {
		printf("Uknown param");
		return pair_new(OPair);
    }
}

pair * eval(pair * param, pair * env) {
	if (!is_list(param)) {
		return eval_ast(param, env);
	} else if (is_nil(param)) {
		return param;
	} else {
		pair * f = eval_ast(car(param), env);
		/* get params */
		param = cdr(param);
        pair * ev_list = pair_new(OList);
        pair * e;
        pair * p;
		while (p = car(param)) {
			e = cons(eval_ast(p, env), e);
            param = cdr(param);
		}
		ev_list->value = e;
		pair * ret = f->fvalue(ev_list, env);
		return ret;
	}
	return param;
}

void print(pair * param, pair * env) {
	pair_print(param);
	/* token_print_recursive(head); */
	/* printf("%s\n", (char *)param); */
	/* return (char*)param; */
}

void rep(void * param, pair * env) {
	param = read(param, env);
	param = eval(param, env);
	print(param, env);
}



int main(int argc, char** argv) {
	(void) argc;
	(void) argv;

	char * str = NULL;
	size_t size = 0;

	pair * env = pair_new(ODict);
	pair * k = pair_new(OAtom);
	k->value = (pair *)"+";
	pair * v = pair_new(OFunc);
	v->value = (pair *)func_plus;
	pair * k2 = pair_new(OAtom);
	k2->value = (pair *)"-";

	dict_set(cons(env, cons(k,v)));
	dict_set(cons(env, cons(k2,v)));

	print(env, env);
	printf("\n");
	print(dict_get(cons(env,k)), env);

	printf("user> ");
	while (getline(&str, &size, stdin) != -1) {
		rep(str, env);
		printf("\r\n");
		printf("user> ");
	}

	free(str);

	return 0;
}
