#include <time.h>
#include <stdio.h>

#include "logging.h"
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
int linesWritten = 0;

#ifdef _WIN32
void enableVirtualTerminal() {
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD dwMode = 0;

    GetConsoleMode(hOut, &dwMode);
    dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    SetConsoleMode(hOut, dwMode);
}
#endif

const char* getTimestamp() {
    static char datetime_str[20];
    time_t now = time(NULL);
    struct tm local_time = *localtime(&now);

    strftime(datetime_str, sizeof(datetime_str), "%Y-%m-%d %H:%M:%S", &local_time);

    return datetime_str;
}

void openLog(){
    // open the log file
    logFile = fopen(getPath("data/debug.log"), "a");
    if (logFile == NULL) {
        printf("%sError opening logfile\n",RED);
    }
}

void closeLog(){
    fclose(logFile);
}

void logMessage(enum logLevel level, const char *text){
    openLog();

    switch (level) {
        case debug:
            fprintf(logFile, "[%s] [DEBUG]: %s", getTimestamp(), text);
            printf("%s[%s] [%sDEBUG%s]: %s", RESET, getTimestamp(), MAGENTA, RESET, text);
            break;
        case info:
            fprintf(logFile, "[%s] [INFO]:  %s", getTimestamp(), text);
            printf("%s[%s] [%sINFO%s]:  %s", RESET, getTimestamp(), GREEN, RESET, text);
            break;
        case warning:
            fprintf(logFile, "[%s] [WARNING]: %s", getTimestamp(), text);
            printf("%s[%s] [%sWARNING%s]: %s", YELLOW, getTimestamp(), YELLOW, RESET, text);
            break;
        case error:
            fprintf(logFile, "[%s] [ERROR]: %s", getTimestamp(), text);
            printf("%s[%s] [%sERROR%s]: %s", RED, getTimestamp(), RED, RESET, text);
            break;
    }
    linesWritten++;
    closeLog();
}

void log_init(){
    // windows specific tweak to enable ansi colors
    #ifdef _WIN32
    enableVirtualTerminal();
    #endif

    // open log file the first time in w mode to overwrite any existing log
    logFile = fopen(getPath("data/debug.log"), "w");
    if (logFile == NULL) {
        printf("%sError opening logfile\n",RED);
    }
    closeLog();
    logMessage(info, "Logging initialized\n");
    linesWritten=1; // reset our counter because not all outputs have actually been written to the log file yet
}

void log_shutdown(){
    logMessage(info, "Logging shutdown\n");
    linesWritten++;
}

// methods in here to print out with colors properly in cross platform way and log plaintext
// should console output only on debug mode?
// does the console open blank even with no prinf? if so then keep debug output just bc its cool but if not then remove printf unless debug for clean look

// basically any time i want to output anything i will call the correct function for its output type in
// this file and then this file handes if we are logging or just outputting to the term on neither
// in the future if no output means no open console window then we disable for production builds

// also need to go through the whole project and get some better error handling at the same time.
// make a pass and log everything important as well as handle any errors which need logged too

// go through and change naming conventions to be underscore lowercase instead of camel case and match c style better
// counter for log lines written (times opened closed)

// add ability to supress below a certain log level

// force errors and warnings to be all caps

// add translucent black panel and cleanup debug overlays