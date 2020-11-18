import unittest
from unittest import skip
from unittest.mock import MagicMock

from niedolis import *


class TestNiedolis(unittest.TestCase):
    def setUp(self):
        pass

    def test_tokenize(self):
        line = "(concat 'A 'B ( concat 'E 'C) 'A (concat 'D 'L 'O))"
        expected_tokens = [ "(", "concat", "'A", "'B",
                        "(", "concat", "'E", "'C", ")",
                        "'A",
                        "(", "concat", "'D", "'L", "'O", ")",
                   ")"]

        tokens = tokenize(line)
        self.assertEqual(tokens, expected_tokens)

    def test_tokenize_quoted_list(self):
        line = "(concat 'A 'B '( concat 'E 'C) 'A (concat 'D 'L 'O))"
        expected_tokens = [ "(", "concat", "'A", "'B",
                            "(", "quote", "concat", "'E", "'C", ")",
                            "'A",
                            "(", "concat", "'D", "'L", "'O", ")",
                            ")"]

        tokens = tokenize(line)
        self.assertEqual(tokens, expected_tokens)

    def test_parse_list(self):
        tokens = tokenize("(concat 'A 'B '( concat 'E 'C) 'A (concat 'D 'L 'O))")
        expected_ast = [
            "concat", "'A", "'B", ['quote', 'concat', "'E", "'C"], "'A", ['concat', "'D", "'L", "'O"]
        ]

        ast = parse(tokens)
        self.assertEqual(ast, expected_ast)

    def test_atom_int(self):
        self.assertEqual(5, atom('5'))
        self.assertEqual(-2, atom('-2'))
        self.assertEqual(0, atom('0'))
        self.assertEqual(123456, atom('123456'))

    def test_atom_str(self):
        self.assertEqual('foobar', atom('foobar'))
        self.assertEqual('+', atom('+'))
        self.assertEqual('A', atom('A'))
        self.assertEqual('QUOTE', atom('QUOTE'))
        self.assertEqual("'Q", atom("'Q"))

    def test_parse_string(self):
        tokens = tokenize("(concat 'A 'B )")
        expected_ast = ["concat", "'A", "'B"]

        ast = parse(tokens)
        self.assertEqual(ast, expected_ast)

    def test_parse_integer(self):
        tokens = tokenize("(+ 1 2 )")
        expected_ast = ["+", 1, 2]

        ast = parse(tokens)
        self.assertEqual(expected_ast, ast)

    def test_fail_not_closed_paren_parse_list(self):
        tokens = tokenize("(concat 'A 'B '( concat 'E 'C) 'A (concat 'D 'L 'O)")

        with self.assertRaises(Exception):
            parse(tokens)

    @skip
    def test_fail_too_many_paren_parse_list(self):
        tokens = tokenize("(concat 'A 'B ))")

        with self.assertRaises(Exception):
            ast = parse(tokens)

    def test_eval_simple_concat(self):
        tokens = tokenize("(concat 'A 'B)")
        ast = parse(tokens)

        result = eval(ast)
        self.assertEqual(result, "AB")

    def test_eval_nested_concat(self):
        tokens = tokenize("(concat 'A 'B (concat 'C 'D))")
        ast = parse(tokens)

        result = eval(ast)
        self.assertEqual(result, "ABCD")

    def test_eval_simple_additon(self):
        tokens = tokenize("(+ 7 8)")
        ast = parse(tokens)

        result = eval(ast)
        self.assertEqual(15, result)

    def test_eval_simple_many_arguments_additon(self):
        tokens = tokenize("(+ 1 1 1 1 1)")
        ast = parse(tokens)

        result = eval(ast)
        self.assertEqual(5, result)


    def test_eval_negative_additon(self):
        tokens = tokenize("(+ 7 -8)")
        ast = parse(tokens)

        result = eval(ast)
        self.assertEqual(-1, result)

    def test_eval_nested_addion(self):
        tokens = tokenize("(+ 1 2 (+ 3 4 5))")
        ast = parse(tokens)

        result = eval(ast)
        self.assertEqual(15, result)

    def test_null_predicate(self):
        tokens = tokenize("(null? ())")
        ast = parse(tokens)

        result = eval(ast)
        self.assertEqual("'true", result)

    def test_false_null_predicate_if_statement(self):
        tokens = tokenize("(if (null? '(abd)) 'true 'false)")
        ast = parse(tokens)
        result = eval(ast)

        self.assertEqual("false", result)

    def test_true_null_predicate_if_statement(self):
        tokens = tokenize("(if (null? ()) 'true 'false)")
        ast = parse(tokens)
        result = eval(ast)

        self.assertEqual("true", result)

    def test_simple_true_predicate_statement(self):
        tokens = tokenize("(if 'true 'true 'false)")
        ast = parse(tokens)
        result = eval(ast)

        self.assertEqual("true", result)

    def test_complex_true_predicate_statement(self):
        tokens = tokenize("(if (+ 4 5) 'true 'false)")
        ast = parse(tokens)
        result = eval(ast)

        self.assertEqual("true", result)

    def test_true_predicate_statement_executed_default(self):
        tokens = tokenize("(if (null? ()) (+ 1 2) (concat '1 '2))")
        ast = parse(tokens)
        add_mock = MagicMock(return_value=3)
        concat_mock = MagicMock(return_value="12")
        env = {'+': add_mock,
               'concat': concat_mock}
        result = eval(ast, env)

        self.assertEqual(3, result)
        add_mock.assert_called_once()
        concat_mock.assert_not_called()

    def test_false_predicate_statement_executed_alternative(self):
        tokens = tokenize("(if (null? ('foo)) (+ 1 2) (concat '1 '2))")
        ast = parse(tokens)
        add_mock = MagicMock(return_value=5)
        concat_mock = MagicMock(return_value="12")
        env = {'+': add_mock,
               'concat': concat_mock}
        result = eval(ast, env)

        self.assertEqual("12", result)
        add_mock.assert_not_called()
        concat_mock.assert_called_once()

    def test_lambda_created(self):
        tokens = tokenize("(lambda (a b) (+ a b))")
        ast = parse(tokens)

        result = eval(ast)
        self.assertIsNotNone(result)

    def test_lambda_call(self):
        tokens = tokenize("((lambda (a b) (+ a b)) 1 2)")
        ast = parse(tokens)

        result = eval(ast)
        self.assertEqual(3, result)

    def test_define_statement(self):
        tokens = tokenize("(define 'x 5)")
        ast = parse(tokens)
        env = {}

        eval(ast, env)
        self.assertIn('x', env)
        self.assertEqual(5, env['x'])

    def test_use_defined_variable(self):
        env = {k: v for k, v in ENV.items()}
        eval(parse(tokenize("(define 'x 5)")), env)
        eval(parse(tokenize("(define 'y 2)")), env)

        tokens = tokenize("(+ x y)")
        ast = parse(tokens)

        result = eval(ast, env)
        self.assertEqual(7, result)

    def test_use_defined_lambda(self):
        env = {k: v for k, v in ENV.items()}
        eval(parse(tokenize("(define 'add "
                            "        (lambda (a b)"
                            "                (+ a b)"
                            "        )"
                            ")")), env)
        eval(parse(tokenize("(define 'y 2)")), env)

        tokens = tokenize("(add 1 y)")
        ast = parse(tokens)

        result = eval(ast, env)
        self.assertEqual(3, result)