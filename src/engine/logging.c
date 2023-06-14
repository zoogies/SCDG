#include "engine.h"

#ifdef _WIN32
#include <windows.h>
#endif

// ANSI escape codes for color.
#define RED     "\x1b[31m"
#define GREEN   "\x1b[32m"
#define YELLOW  "\x1b[33m"
#define BLUE    "\x1b[34m"
#define MAGENTA "\x1b[35m"
#define CYAN    "\x1b[36m"
#define RESET   "\x1b[0m"

FILE *logFile = NULL;

#ifdef _WIN32
void enableVirtualTerminal() {
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD dwMode = 0;

    GetConsoleMode(hOut, &dwMode);
    dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    SetConsoleMode(hOut, dwMode);
}
#endif

void print_colored(const char *color_code, const char *text) {
    printf("%s%s" RESET, color_code, text);
}

void log_init(){
    #ifdef _WIN32
    enableVirtualTerminal();
    #endif

    logFile = fopen(getPath("data/log.txt"), "w");
    if (logFile == NULL) {
        printf("Error opening logfile\n");
    }

    // Write a message to the log file
    fprintf(logFile, "This is a log message.\n");
    print_colored(RED, "This is a red message.\n");
    // Close the log file
    fclose(logFile);
}

// methods in here to print out with colors properly in cross platform way and log plaintext
// should console output only on debug mode?
// does the console open blank even with no prinf? if so then keep debug output just bc its cool but if not then remove printf unless debug for clean look