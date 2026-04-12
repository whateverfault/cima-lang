#include "sys.h"

#include <stdint.h>
#include <time.h>

#if defined(_WIN32)
#include <windows.h>
#include <conio.h>

static DWORD old_mode;

#define get_pid() GetCurrentProcessId()
#endif
#if defined(__unix__) || defined(__unix) || defined(__APPLE__)
#include <termios.h>
#include <unistd.h>
#include <stdio.h>

static struct termios oldt;
#define get_pid() getpid()
#endif

static bool saved = false;

void set_echo_enabled(bool enabled) {
#if defined(_WIN32)
    HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
    DWORD mode;

    if (!saved) {
        GetConsoleMode(hStdin, &old_mode);
        saved = true;
    }

    mode = old_mode;

    if (!enabled) {
        mode &= ~ENABLE_ECHO_INPUT;
    }

    SetConsoleMode(hStdin, mode);
#endif
    
#if defined(__unix__) || defined(__unix) || defined(__APPLE__)
    struct termios newt;

    if (!saved) {
        tcgetattr(STDIN_FILENO, &oldt);
        saved = true;
    }

    newt = oldt;

    if (!enabled) {
        newt.c_lflag &= ~(ECHO);
    }

    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
#endif
}

void disable_raw_mode() {
#if defined(__unix__) || defined(__unix) || defined(__APPLE__)
    tcsetattr(STDIN_FILENO, TCSANOW, &orig_termios);
#endif
}

void enable_raw_mode() {
#if defined(__unix__) || defined(__unix) || defined(__APPLE__)
    struct termios raw;
    tcgetattr(STDIN_FILENO, &orig_termios);
    atexit(disable_raw_mode);

    raw = orig_termios;

    raw.c_lflag &= ~(ICANON | ECHO);
    
    raw.c_cc[VMIN] = 1;
    raw.c_cc[VTIME] = 0;

    tcsetattr(STDIN_FILENO, TCSANOW, &raw);
#endif
}

int read_key() {
    enable_raw_mode();
    
    int c;

#if defined(_WIN32)
    c = _getch();
#endif
#if defined(__unix__) || defined(__unix) || defined(__APPLE__)
    read(STDIN_FILENO, &c, 1);
#endif
    
    disable_raw_mode();
    
    return c;
}

void init_rng() {
    static bool initialized = false;
    if (initialized) return;
    initialized = true;
    
    uint64_t t = (uint64_t)time(NULL);
    uint64_t c = (uint64_t)clock();
    uint64_t pid = (uint64_t)get_pid();
    uint64_t addr = (uint64_t)(uintptr_t)&t;

    uint64_t seed = t ^ (c << 16) ^ (pid << 32) ^ addr;
    
    seed ^= seed >> 33;
    seed *= 0xff51afd7ed558ccdULL;
    seed ^= seed >> 33;
    seed *= 0xc4ceb9fe1a85ec53ULL;
    seed ^= seed >> 33;
    srand(seed);
}