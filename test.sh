#!/bin/bash
assert() {
  expected="$1"
  input="$2"

  ./9cc "$input" > tmp.s
  gcc -o tmp tmp.s
  ./tmp
  actual="$?"

  if [ "$actual" = "$expected" ]; then
    echo "$input => $actual"
  else
    echo "$input => $expected expected, but got $actual"
    exit 1
  fi
}

assert 0 '0;'
assert 42 '42;'
assert 21 '5+20-4;'
assert 41 ' 12 + 34 - 5 ;'
assert 30 '10 * (5 - 2);'
assert 4 '(3+5)/2;'
assert 1 '(-3+5)/+2;'
assert 10 '-10 +20;'
assert 10 '- -10;'
assert 10 '- - +10;'
assert 1 '10 == 10;'
assert 0 '10 == 9;'
assert 1 '10 < 12;'
assert 1 '-10 < 12;'
assert 0 '10 < -12;'
assert 0 '10 < 10;'
assert 0 '10 < 9;'
assert 1 '10 <= 12;'
assert 1 '10 <= 10;'
assert 0 '10 <= 9;'
assert 1 '10 > 9;'
assert 0 '10 > 10;'
assert 0 '10 > 12;'
assert 1 '10 >= 9;'
assert 1 '10 >= 10;'
assert 0 '10 >= 12;'
assert 1 'a = 1;a;'
assert 3 'a=1; b=2; c=3;'
assert 4 'a=3; b=a+2; a+b-4;'
assert 12 'a=3; b=a+2; c=a+b; a+b+c-4;'
assert 10 'a=10; return a;'
assert 3 'return 3;'
assert 12 'a=3; b=a+2; c=a+b; return a+b+c-4;'
assert 1 'foo=1; return foo;'
assert 2 'foo=1; bar = 3; return bar - foo;'
assert 2 'if (1) return 2; return 3;'
assert 3 'if (0) return 2; return 3;'
assert 2 'if (2-1) return 2; return 3;'
assert 3 'if (2-2) return 2; return 3;'
assert 5 'a = 1 * 2; if (a == 2) return 5; return 3;'
assert 3 'a = 2 * 2; if (a == 2) return 5; return 3;'
assert 1 'if (2 > 1) return 1; else return 2;'
assert 2 'if (2 < 1) return 1; else return 2;'

echo OK