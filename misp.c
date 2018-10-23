#include "core.c"

int main(int argc, char** argv) {
	(void) argc;
	(void) argv;

	char * str = NULL;
	size_t size = 0;

	pair * env = pair_new(ODict);
	dict_set(cons(env,
	              cons(
	                   pair_new_val(OAtom, "+"),
	                   pair_new_val(OFunc, fn_plus))));
	dict_set(cons(env,
	              cons(
	                   pair_new_val(OAtom, "-"),
	                   pair_new_val(OFunc, fn_minus))));
	dict_set(cons(env,
	              cons(
	                   pair_new_val(OAtom, "*"),
	                   pair_new_val(OFunc, fn_mul))));

	print(env, env);
	printf("\n");
	/* print(dict_get(cons(env, pair_new_val(OAtom, "-"))), env); */

	printf("user> ");
	while (getline(&str, &size, stdin) != -1) {
		rep(str, env);
		printf("\r\n");
		printf("user> ");
	}

	free(str);

	return 0;
}
