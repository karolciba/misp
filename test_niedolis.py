import unittest
from unittest import skip

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

    def test_parse_string(self):
        tokens = tokenize("(concat 'A 'B )")
        expected_ast = ["concat", "'A", "'B"]

        ast = parse(tokens)
        self.assertEqual(ast, expected_ast)

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