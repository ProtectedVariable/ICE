//
// Created by Thomas Ibanez on 24.11.20.
//

#include "Logger.h"

namespace ICE {
    const char* severityString[6] = {
        "Debug",
        "Verbose",
        "Info",
        "Warning",
        "Error",
        "Fatal"
    };

    Logger::Severity Logger::filter = Logger::DEBUG;

    void Logger::Log(ICE::Logger::Severity s, const char *module, const char *fmt, ...) {
        if(s >= filter) {
            va_list args;
            va_start(args, fmt);
            const char *color;
            switch (s) {
                case DEBUG:
                    color = "[0;36m";
                    break;
                case VERBOSE:
                    color = "[0m";
                    break;
                case INFO:
                    color = "[0;35m";
                    break;
                case WARNING:
                    color = "[0;33m";
                    break;
                case ERROR:
                    color = "[0;31m";
                    break;
                case FATAL:
                    color = "[0;41m";
                    break;
            }
            printf("\033%s[%s]{%s}\t\t", color, severityString[s], module);
            vprintf(fmt, args);
            printf("\033[0m\n");
            va_end(args);
        }
    }
}