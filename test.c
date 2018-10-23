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
	pair * param;
	pair * none = nil();

	left = integer(5);
	right = integer(5);
	assert("(eq? 5 5) -> true", is_true(fn_eq(list2(left, right), env)));

	left = integer(5);
	right = integer(4);
	assert("(eq? 5 4) -> false", is_nil(fn_eq(list2(left, right), env)));

	left = atom("foo");
	right = atom("foo");
	param = list2(left, right);
	assert("(eq? foo foo) -> true", is_true(fn_eq(list2(left, right), env)));

	left = atom("foo");
	right = atom("bar");
	param = list2(left, right);
	assert("(eq? foo bar) -> false", is_nil(fn_eq(list2(left, right), env)));
}

int main(int argc, char** argv) {
	(void) argc;
	(void) argv;

	fn tests[] = {
		test_list,
		test_fn_eg,
		NULL
	};


	fn (*ptr) = tests;

    do {
		(*ptr)();
	} while (*(++ptr));


	return 0;
}
