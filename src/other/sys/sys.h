#ifndef CONSOLE_H
#define CONSOLE_H

#include <stdbool.h>

void set_echo_enabled(bool enabled);
int read_key();
bool key_pressed();

void sleep_ms(int ms);

void init_rng();

#endif //CONSOLE_H
