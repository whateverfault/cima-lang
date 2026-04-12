mkdir bin

cc -std=c11 -o .\bin\cima .\src\main.c .\src\lexer\lexer.c .\src\parser\parser.c .\src\executor\executor.c .\src\executor\funcs.c .\src\types\type.c .\src\types\type.c .src\other\console\console.c -I.\src\thirdparty -I.\src\