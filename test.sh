#!/bin/bash

cat <<EOF | gcc -xc -c -o tmp2.o -
int ret3() { return 3; }
int ret5() { return 5; }
int add(int x, int y) { return x + y; }
EOF

assert() {
  expected="$1"
  input="$2"

  ./9cc "$input" > tmp.s
  gcc -o tmp tmp.s tmp2.o
  ./tmp
  actual="$?"

  if [ "$actual" = "$expected" ]; then
    echo "$input => $actual"
  else
    echo "$input => $expected expected, but got $actual"
    exit 1
  fi
}

assert 0 'return 0;'
assert 42 'return 42;'
assert 21 'return 5+20-4;'
assert 41 'return  12 + 34 - 5 ;'
assert 30 'return 10 * (5 - 2);'
assert 4 'return (3+5)/2;'
assert 1 'return (-3+5)/+2;'
assert 10 'return -10 +20;'
assert 10 'return - -10;'
assert 10 'return - - +10;'
assert 1 'return 10 == 10;'
assert 0 'return 10 == 9;'
assert 1 'return 10 < 12;'
assert 1 'return -10 < 12;'
assert 0 'return 10 < -12;'
assert 0 'return 10 < 10;'
assert 0 'return 10 < 9;'
assert 1 'return 10 <= 12;'
assert 1 'return 10 <= 10;'
assert 0 'return 10 <= 9;'
assert 1 'return 10 > 9;'
assert 0 'return 10 > 10;'
assert 0 'return 10 > 12;'
assert 1 'return 10 >= 9;'
assert 1 'return 10 >= 10;'
assert 0 'return 10 >= 12;'
assert 1 'a = 1;return a;'
assert 4 'a=3; b=a+2; return a+b-4;'
assert 12 'a=3; b=a+2; c=a+b; return a+b+c-4;'
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
assert 5 'i = 0; while (i < 5) i = i + 1; return i;'
assert 10 'i = 0; a = 2; while (i < 10) i = i + a; return i;'
assert 10 'i = 100; while (i/2 > 5) i = i - 2; return i;'
assert 20 'i = 0; j = 0; for (i=0;i<10;i=i+1) j = j + 2; return j;'
assert 3 'for (;;) return 3; return 5;'
assert 1 '{ return 1; }'
assert 3 'return ret3();'
assert 5 'return ret5();'
assert 2 'return add(1,1);'
assert 5 'return add(2,3);'

echo OK