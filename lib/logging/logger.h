#pragma once
#include <Arduino.h>

enum LogLevel {
    DEBUG,
    INFO,
    WARN,
    ERROR,
    NONE
};

class Logger {
private:
    Stream *stream_;
    LogLevel level_;
public:
    Logger(Stream *stream, LogLevel level);
    void debug(const String message);
    void info(const String message);
    void warn(const String message);
    void error(const String message);
};