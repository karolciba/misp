#include <stdio.h>
#include <string.h>
#include <stdlib.h>

enum ttype { Eol, LeftParen, RightParen, Atom, Comment, NewLine, Error } ttype;
enum otype { ONil, OPair, OAtom, OList, OFunc, ODict, OTrue, OInt, OError=255} otype;

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
	int refs;
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

/* #define is_pair(obj) ((long)((pair *)obj)->next % 2 == 0) */
/* #define is_atom(obj) ((long)((pair *)obj)->next == 7) */
#define is_pair(obj) (obj->type == OPair)
#define is_atom(obj) (obj->type == OAtom)
#define is_int(obj) (obj->type == OInt)
#define is_func(obj) (obj->type == OFunc)
#define is_true(obj) (obj->type == OTrue)
#define is_list(obj) (obj->type == OList)
#define is_dict(obj) (obj->type == ODict)
static inline int is_nil( pair * obj) { return obj->type == OPair && obj->value == obj && obj->next == obj; };

#define DEBUG 0
#define info(...) if (DEBUG) printf(__VA_ARGS__)

pair * pair_new(enum otype type) {
	pair * pair = calloc(1, sizeof(pair));
	pair->type = type;
	/* one always need to call D_REF
	 * it won't work other way in c */
	pair->refs = 1;
	return pair;
}

pair * pair_new_val(enum otype type, void * ptr) {
	pair * pair = pair_new(type);
	pair->value = ptr;
	return pair;
}

void pair_delete(pair * pair);

static inline void I_REF(pair * pair) {
	pair->refs++;
}

static inline void D_REF(pair * pair) {
	return;
	if (!pair) return;
	pair->refs--;
	if (pair->refs == 0) {
		pair_delete(pair);
	}
}


void pair_delete(pair * pair) {
	if (!pair) {
		return;
	}

	if (is_atom(pair)) {
		free(pair->cvalue);
	} else if (is_list(pair)) {
		D_REF(pair->value);
		free(pair);
	} else if (is_dict(pair)) {
		D_REF(pair->value);
		free(pair);
	} else if (is_pair(pair)) {
		D_REF(pair->value);
		D_REF(pair->next);
	}

	free(pair);
}

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
	pair->value = pair;
	pair->next = pair;
	return pair;
}

pair * atom(char * str) {
	pair * pair = pair_new(OAtom);
	pair->cvalue = strdup(str);
	return pair;
}

pair * integer(int i) {
	pair * pair = pair_new(OInt);
	pair->ivalue = i;
	return pair;
}

pair * true() {
	pair * pair = pair_new(OTrue);
	return pair;
}

pair * cons(pair * car, pair * cdr) {
	pair * pair = pair_new(OPair);
	pair->value = car;
	pair->next = cdr;

	return pair;
}

#define list1(p1) list(cons(p1, nil()))
#define list2(p1,p2) list(cons(p1,cons(p2,nil())))
#define list3(p1,p2,p3) list(cons(p1,cons(p2,cons(p3,nil()))))
#define list4(p1,p2,p3,p4) list(cons(p1,cons(p2,cons(p3,cons(p4,nil())))))

pair * list(pair * param) {
	pair * ret = pair_new(OList);
	ret->value = param;
	return ret;
}

pair * car(pair * p) {
	if (is_list(p) && p->value) {
		return p->value->value;
	}

	if (!p->value) {
		return nil();
	}

	I_REF(p->value);
	return p->value;
}

pair * cdr(pair * p) {
	if (is_list(p) && p->value) {
		pair * l = pair_new(OList);
		l->value = p->value->next;
		return l;
	}

	if (!p->next) {
		return nil();
	}

	I_REF(p->next);
	return p->next;
}

pair * fn_eq(pair * param, pair * env) {
	pair * first = car(param);
	pair * second = car(cdr(param));
	pair * ret = nil();

	if (first->type != second->type) {
		// do nothing, return nil
	} else if (is_nil(first)) {
		ret = true();
	} else if (is_list(first) && is_pair(first)) {
		pair * left = fn_eq(list2(car(first),car(second)), env);
		pair * right = fn_eq(list2(cdr(first),cdr(second)), env);
		if (is_nil(left) && is_nil(right)) {
			ret = true();
		}
	} else if (is_atom(first) && strcmp(first->cvalue, second->cvalue) == 0) {
		ret = true();
	} else if (first->value == second->value) {
		ret = true();
	}

	return ret;
}

pair * fn_plus(pair * param, pair * env) {
	(void) env;
	pair * pair = pair_new(OInt);
	pair->ivalue = 0;
	while (!is_nil(param)) {
		pair_print(param);
		pair->ivalue += car(param)->ivalue;
		param = cdr(param);
	}
	return pair;
}

pair * fn_minus(pair * param, pair * env) {
	(void) env;
	pair * subtrahend = car(param);
	pair * minuend = car(cdr(param));
	pair * pair = pair_new(OInt);
	pair->ivalue = subtrahend->ivalue - minuend->ivalue;
	return pair;
}

pair * fn_mul(pair * param, pair * env) {
	(void) env;
	pair * pair = pair_new(OInt);
	pair->ivalue = 1;
	while (!is_nil(param)) {
		pair_print(param);
		pair->ivalue *= car(param)->ivalue;
		param = cdr(param);
	}
	return pair;
}

/*
 * (dict_set <dict> <key> <value>)
 * adds or sets new element under 'key' in a dictionary
 *
 * internally dictionary is stored as a list { (key value) (keyA valueA) ... }
 * TODO: fix - know its { key value key value ... }
 */
void dict_set(pair * param) {
	// TODO: extend syntex with { } notation
	// TODO: consider keeping it sorted? O(n) -> ~O(n/2)
	pair * list = car(param); // pick dictionary object
	param = cdr(param); // peel off dictionary object

	pair * key = car(param);  // pick key object
	pair * value = cdr(param); // pick value object

	pair * pointer;
	pair * tkey;

	while ((pointer = car(list)) && !is_nil(pointer)) {
		tkey = car(pointer);
		if (strcmp(tkey->cvalue, key->cvalue) == 0) {
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

/*
 * (dict_get <dict> <key>) -> <value>|<nil>
 * returns element form dictionary under a key or nil if key doesn't exist
 *
 * internally dictionary is stored as a list ( (key value) (keyA valueA) ... )
 */
pair *dict_get(pair * param) {
	pair * list = car(param);
	pair * key = cdr(param);

	pair * pointer;
	pair * tkey;
	while (list && (pointer = car(list))) {
		// TODO: implement and use eq? operator
		tkey = car(pointer);
		if (strcmp(tkey->cvalue, key->cvalue) == 0) {
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
		// TODO: add { } dictionary syntax
		// TODO: add :intern syntax
		// TODO: add 'quote or <quote> or /quote/ syntax
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
	(void) env;
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
		pair * e = NULL;
		pair * p;
		pair * t;
		while ((p = car(param))) {
			t = eval_ast(p, env);
			e = cons(t, e);
			param = cdr(param);
		}
		ev_list->value = e;
		pair * ret = f->fvalue(ev_list, env);
		return ret;
	}
	return param;
}

void print(pair * param, pair * env) {
	(void) env;
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
