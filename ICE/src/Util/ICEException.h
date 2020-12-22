//
// Created by Thomas Ibanez on 22.12.20.
//

#ifndef ICE_ICEEXCEPTION_H
#define ICE_ICEEXCEPTION_H

#include <exception>

namespace ICE {
    class ICEException : public std::exception {
    public:
        const char *what() const _NOEXCEPT override;
    };
}


#endif //ICE_ICEEXCEPTION_H
