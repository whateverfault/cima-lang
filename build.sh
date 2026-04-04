mkdir -p bin

gcc -o ./bin/calc ./src/main.c ./src/lexer/lexer.c ./src/parser/parser.c ./src/executor/executor.c ./src/executor/funcs.c ./src/types/type.c -I./src/thirdparty -I./src/