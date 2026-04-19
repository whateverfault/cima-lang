mkdir bin

clang -lm -std=c11 -Wpedantic -fsanitize=address,undefined -fsanitize-address-use-after-scope -fno-omit-frame-pointer -Wno-strict-prototypes -g -O1 -o .\bin\cima_dbg .\src\main.c .\src\lexer\lexer.c .\src\parser\parser.c .\src\executor\executor.c .\src\executor\funcs\funcs.c .\src\executor\types\type.c .\src\other\sys\sys.c .\src\other\built_in.c -I.\src\thirdparty -I.\src\