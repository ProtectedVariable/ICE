//
// Created by Thomas Ibanez on 01.08.21.
//

#ifndef ICE_JSONPARSER_H
#define ICE_JSONPARSER_H


#include <Eigen/Core>
#include <nlohmann/json.hpp>
using json = nlohmann::json;

namespace ICE {
    class JsonParser {
    public:
        static Eigen::Vector3f parseVec3(const json &src) {
            return Eigen::Vector3f(src["x"],src["y"],src["z"]);
        }

        static Eigen::Vector4f parseVec4(const json &src) {
            return Eigen::Vector4f(src["x"],src["y"],src["z"],src["w"]);
        }
    };
}


#endif //ICE_JSONPARSER_H
