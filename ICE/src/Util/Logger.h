//
// Created by Thomas Ibanez on 24.11.20.
//

#ifndef ICE_LOGGER_H
#define ICE_LOGGER_H

#include <string>

namespace ICE {
    class Logger {
    public:
        enum Severity { DEBUG = 0, VERBOSE = 1, INFO = 2, WARNING = 3, ERROR = 4, FATAL = 5 };
        static void Log(Severity s, const char* module, const char* fmt, ...);
        static Severity filter;
    };
}


#endif //ICE_LOGGER_H
