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

        static json dumpMat4(const Eigen::Matrix4f& mat) {
            json j = json::array();
            for (int i = 0; i < 4; ++i)
                for (int k = 0; k < 4; ++k)
                    j.push_back(mat(i, k));
            return j;
        }

        inline Eigen::Matrix4f readMat4(const json& j) {
            Eigen::Matrix4f mat;
            if (!j.is_array() || j.size() != 16)
                throw std::runtime_error("Invalid JSON for Eigen::Matrix4f");
            for (int i = 0; i < 4; ++i)
                for (int k = 0; k < 4; ++k)
                    mat(i, k) = j[i * 4 + k].get<float>();
            return mat;
        }
    };
}


#endif //ICE_JSONPARSER_H
