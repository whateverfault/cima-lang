mkdir bin

cc -Wno-incompatible-pointer-types -std=c11 -o .\bin\cima .\src\main.c .\src\lexer\lexer.c .\src\parser\parser.c .\src\executor\executor.c .\src\executor\funcs\funcs.c .\src\executor\types\type.c .\src\other\sys\sys.c .\src\other\built_in.c -I.\src\thirdparty -I.\src\