#include <logger.h>

Logger::Logger(Stream *stream, LogLevel level): stream_(stream), level_(level) {};

void Logger::debug(const String message) {
    if (level_ <= DEBUG) { stream_->println("DEBUG: " + message); }
}

void Logger::info(const String message) {
    if (level_ <= INFO) { stream_->println("INFO: " + message); }
}

void Logger::warn(const String message) {
    if (level_ <= WARN) { stream_->println("WARN: " + message); }
}

void Logger::error(const String message) {
    if (level_ <= ERROR) { stream_->println("ERROR: " + message); }
}
