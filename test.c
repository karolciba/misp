#include "core.c"

#define assert(NAME, EXPR) _assert(NAME, #EXPR, (EXPR), __func__, __FILE__, __LINE__)

static inline void _assert(const char * name, const char* expr_str, int expr, const char *func, const char * file, int line)
{
	fprintf(stderr, "%s: %s ", func, name);
	if (expr) {
		fprintf(stderr, "[OK]\n");
	} else {
		fprintf(stderr, "[FAIL] Assertion '%s' failed, file '%s' line '%d'.\n", expr_str, file, line);
	}
}


typedef void (*fn) ();

void test_tokenize() {
	char * c;
	pair * t;
	pair * expected;

	c = "(+ 1 2 3)";
	t = tokenize(c);
	printf("tokenized: ");
	print(t,nil());
	printf("\n");

	/* assert("t[0] == '('", t->type == TLeftParen); */
	/* t = t->next; */
	/* assert("t[1] == '+'", strcmp(t->cvalue,"+") == 0); */
	/* t = t->next; */
	/* assert("t[2] == '1'", strcmp(t->cvalue,"1") == 0); */
	/* t = t->next; */
	/* assert("t[3] == '2'", strcmp(t->cvalue,"2") == 0); */
	/* t = t->next; */
	/* assert("t[4] == '3'", strcmp(t->cvalue,"3") == 0); */
	/* t = t->next; */
	/* assert("t[5] == ')'", t->type == TRightParen); */
	/* assert("t[6] == NULL", t->next == NULL); */

	expected = list6(atom("("),
			atom("+"),
			atom("1"),
			atom("2"),
			atom("3"),
			atom(")"));
	printf("expected:  ");
	print(expected, nil());
	printf("\n");
	assert("input: (+ 1 2 3) => (list '+ 1 2 3))", is_true(fn_eq(list2(t,expected),nil())));
}

void test_read_atom() {
	pair * t;
	pair * e;

	t = read_atom(atom("5"), nil());
	e = integer(5);
	assert("input: 5 -> integer(5)", is_true(fn_eq(list2(t,e), nil())));

	t = read_atom(atom("foo"), nil());
	e = atom("foo");
	assert("input: foo -> atom(foo)", is_true(fn_eq(list2(t,e), nil())));

	t = read_atom(atom("foo"), nil());
	e = atom("bar");
	assert("input: foo !-> atom(bar)", is_nil(fn_eq(list2(t,e), nil())));
}

void test_read_list() {
	pair * t;
	pair * e;

	t = read_list(list2(atom("("),
				atom(")")),
			nil());
	e = list0();
	assert("input: () -> ()", is_true(fn_eq(list2(t,e), nil())));

	t = read_list(list5(atom("("),
				atom("+"),
				atom("1"),
				atom("2"),
				atom(")")),
			nil());
	e = list3(atom("+"),
			integer(1),
			integer(2));
	assert("input: (+ 1 2) -> (list + 1 2)", is_true(fn_eq(list2(t,e), nil())));

	t = read_list(list5(atom("("),
				atom("def!"),
				atom("foo"),
				atom("bar"),
				atom(")")),
			nil());
	e = list3(atom("def!"),
			atom("foo"),
			atom("bar"));
	assert("input: (def! foo bar) -> (list def foo bar)", is_true(fn_eq(list2(t,e), nil())));
}

void test_eval_ast() {
}

void test_eval() {
}

void test_car() {
	pair * a, * b, * test;

	a = atom("foo");
	b = atom("bar");
	test = cons(a,b);
	assert("(car (cons a b)) -> a", car(test) == a);

	test = list1(a);
	assert("(car (list a)) -> a", car(test) == a);

	test = list2(a, b);
	assert("(car (list a b)) -> a", car(test) == a);

}

void test_cdr() {
	pair * a, * b, * test;

	a = atom("foo");
	b = atom("bar");
	test = cons(a,b);
	assert("(cdr (cons a b)) -> b", cdr(test) == b);

	test = list1(a);
	assert("(cdr (list a)) -> ()", is_nil(cdr(test)));

	test = list2(a, b);
	assert("(cdr (list a b)) -> (b) ; list", is_list(cdr(test)));
	assert("(cdr (list a b)) -> (b) ; containing b", car(cdr(test)) == b);
	assert("(cdr (cdr (list a b))) -> () ; empty list, aka nil", is_nil(cdr(cdr(test))));

	test = list3(a,b,b);
	assert("(cdr (list a b nil())) -> (b) ; list", is_list(cdr(test)));
	assert("(car(cdr (list a b nil()))) -> b", car(cdr(test)) == b);
}

void test_list() {
	pair * l;
	int test = 0;

	l = list1(pair_new(OPair));
	test = 0;
	if (is_list(l) && car(l)->type==OPair && is_nil(car(cdr(l))))
		test = 1;

	assert("list creation 1 element", test);

	l = list2(pair_new(OPair),pair_new(OAtom));
	test = 0;
	if (is_list(l) && car(l)->type==OPair && car(cdr(l))->type == OAtom)
		test = 1;

	assert("list creation 2 elements", test);
}

void test_fn_eg() {

	pair * left;
	pair * right;
	pair * env = nil();

	left = integer(5);
	right = integer(5);
	assert("(eq? 5 5) -> true", is_true(fn_eq(list2(left, right), env)));

	left = integer(5);
	right = integer(4);
	assert("(eq? 5 4) -> false", is_nil(fn_eq(list2(left, right), env)));

	left = atom("foo");
	right = atom("foo");
	assert("(eq? foo foo) -> true", is_true(fn_eq(list2(left, right), env)));

	left = atom("foo");
	right = atom("bar");
	assert("(eq? foo bar) -> false", is_nil(fn_eq(list2(left, right), env)));

	left = nil();
	right = nil();
	assert("(eq? () ()) -> true", is_true(fn_eq(list2(left, right), env)));

	left = nil();
	right = atom("foo");
	assert("(eq? () foo) -> false", is_nil(fn_eq(list2(left, right), env)));

	left = atom("foo");
	right = nil();
	assert("(eq? foo ()) -> false", is_nil(fn_eq(list2(left, right), env)));

	left = list2(atom("foo"), atom("bar"));
	right = list2(atom("foo"), atom("bar"));
	assert("(eq? (foo bar) (foo bar)) -> true", is_true(fn_eq(list2(left, right), env)));

	left = list2(atom("foo"), atom("bar"));
	right = list2(atom("fuu"), atom("bar"));
	assert("(eq? (foo bar) (fuu bar)) -> false", is_nil(fn_eq(list2(left, right), env)));

	left = list2(atom("foo"), atom("bar"));
	right = list2(atom("foo"), atom("baz"));
	assert("(eq? (foo bar) (foo baz)) -> false", is_nil(fn_eq(list2(left, right), env)));

	left = list2(atom("foo"), atom("bar"));
	right = nil();
	assert("(eq? (foo bar) ()) -> false", is_nil(fn_eq(list2(left, right), env)));

	left = nil();
	right = list2(atom("foo"), atom("bar"));
	assert("(eq? (foo bar) ()) -> false", is_nil(fn_eq(list2(left, right), env)));
}

void test_fn_plus() {
	pair * param;
	pair * ans;
	pair * env = nil();

	param = list1(nil());
	ans = fn_plus(param, env);
	assert("(+ ()) -> 0", ans->ivalue == 0);

	param = list1(integer(1));
	ans = fn_plus(param, env);
	assert("(+ 1) -> 1", ans->ivalue == 1);

	param = list3(integer(1),integer(2),integer(3));
	ans = fn_plus(param, env);
	assert("(+ 1 2 3) -> 6", ans->ivalue == 6);

	param = list3(integer(2),integer(-2),integer(3));
	ans = fn_plus(param, env);
	assert("(+ 2 -2 3) -> 3", ans->ivalue == 3);

}

int main(int argc, char** argv) {
	(void) argc;
	(void) argv;

	fn tests[] = {
		test_tokenize,
		test_read_atom,
		test_read_list,
		test_eval_ast,
		test_eval,
		test_car,
		test_cdr,
		test_list,
		test_fn_eg,
		test_fn_plus,
		NULL
	};


	fn (*ptr) = tests;

	do {
		(*ptr)();
	} while (*(++ptr));


	return 0;
}
