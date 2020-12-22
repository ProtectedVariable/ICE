//
// Created by Thomas Ibanez on 22.12.20.
//

#include "ICEException.h"

namespace ICE {
    const char *ICEException::what() const noexcept {
        return exception::what();
    }
}