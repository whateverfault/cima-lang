mkdir bin

clang -lm -std=c11 -Wpedantic -fsanitize=address -g -O0 -o .\bin\cima_dbg .\src\main.c .\src\lexer\lexer.c .\src\parser\parser.c .\src\executor\executor.c .\src\executor\funcs.c .\src\types\type.c .src\other\console\console.c -I.\src\thirdparty -I.\src\