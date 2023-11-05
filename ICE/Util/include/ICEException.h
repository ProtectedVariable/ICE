//
// Created by Thomas Ibanez on 22.12.20.
//

#ifndef ICE_ICEEXCEPTION_H
#define ICE_ICEEXCEPTION_H

#include <stdexcept>

namespace ICE {
    class ICEException : public std::runtime_error {
        public:
        ICEException(const std::string &message) : std::runtime_error(message) {}
    };
}


#endif //ICE_ICEEXCEPTION_H
