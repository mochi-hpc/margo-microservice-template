#ifndef _LOGGING_H
#define _LOGGING_H

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "config.h"

#ifdef ALPHA_LOGGING_ENABLE_COLORS
#define RED    "\033[0;31m"
#define GREEN  "\033[0;32m"
#define YELLOW "\033[0;33m"
#define BLUE   "\033[0;34m"
#define RESET  "\033[0m"
#else
#define RED
#define GREEN
#define YELLOW
#define BLUE
#define RESET
#endif

static inline void log_format(
        const char* file, int line,
        const char* tag, const char* message, va_list args) {
    time_t now;
    time(&now);
    char* date = ctime(&now);
    date[strlen(date) - 1] = '\0';
    printf("[%s]\t[%s] [%s:%04d] ", tag, date, file, line);
    vprintf(message, args);
    printf("\n");
}

static inline void log_error(
        const char* file, int line,
        const char* message, ...) {
#ifdef ALPHA_LOG_ERROR
    va_list args;
    va_start(args, message);
    log_format(file, line, YELLOW "ERROR" RESET, message, args);
    va_end(args);
#else
    (void)file;
    (void)line;
    (void)message;
#endif
}

static inline void log_fatal(
        const char* file, int line,
        const char* message, ...) {
    va_list args;
    va_start(args, message);
    log_format(file, line, RED "FATAL" RESET, message, args);
    va_end(args);
    exit(-1);
}

static inline void log_info(
        const char* file, int line,
        const char* message, ...) {
#ifdef ALPHA_LOG_INFO
    va_list args;
    va_start(args, message);
    log_format(file, line, GREEN "INFO" RESET, message, args);
    va_end(args);
#else
    (void)file;
    (void)line;
    (void)message;
#endif
}

static inline void log_debug(
        const char* file, int line,
        const char* message, ...) {
#ifdef ALPHA_LOG_DEBUG
    va_list args;
    va_start(args, message);
    log_format(file, line, BLUE "DEBUG" RESET, message, args);
    va_end(args);
#else
    (void)file;
    (void)line;
    (void)message;
#endif
}

#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)
#define LOG_ERROR(...) log_error(__FILENAME__, __LINE__, __VA_ARGS__)
#define LOG_INFO(...)  log_info(__FILENAME__, __LINE__, __VA_ARGS__)
#define LOG_DEBUG(...) log_debug(__FILENAME__, __LINE__, __VA_ARGS__)
#define LOG_FATAL(...) log_fatal(__FILENAME__, __LINE__, __VA_ARGS__)

#endif
