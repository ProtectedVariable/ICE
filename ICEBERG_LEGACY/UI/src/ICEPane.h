//
// Created by Thomas Ibanez on 29.11.20.
//

#ifndef ICE_ICEPANE_H
#define ICE_ICEPANE_H

namespace ICE {
    class ICEPane {
    public:
        virtual void initialize() = 0;
        virtual bool render() = 0;
    };
}

#endif //ICE_ICEPANE_H
