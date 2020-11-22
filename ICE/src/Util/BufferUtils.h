//
// Created by Thomas Ibanez on 22.11.20.
//

#ifndef ICE_BUFFERUTILS_H
#define ICE_BUFFERUTILS_H

#include <Eigen/Dense>
#include <vector>

namespace ICE {
    class BufferUtils {
    public:
        //TODO: Might be able to make it all in one with VectorXd and the data() function
        static float* CreateFloatBuffer(const std::vector<Eigen::Vector3d>& vectors) {
            float* buffer = static_cast<float *>(malloc(sizeof(float) * 3 * vectors.size()));
            uint32_t i = 0;
            for(const auto &v : vectors) {
                buffer[i] = (float) v.x();
                buffer[i+1] = (float) v.y();
                buffer[i+2] = (float) v.z();
                i += 3;
            }
            return buffer;
        }

        static float* CreateFloatBuffer(const std::vector<Eigen::Vector2d>& vectors) {
            auto* buffer = static_cast<float *>(malloc(sizeof(float) * 2 * vectors.size()));
            uint32_t i = 0;
            for(const auto &v : vectors) {
                buffer[i] = (float) v.x();
                buffer[i+1] = (float) v.y();
                i += 2;
            }
            return buffer;
        }

        static int* CreateIntBuffer(const std::vector<Eigen::Vector3i>& vectors) {
            auto* buffer = static_cast<int *>(malloc(sizeof(int) * 3 * vectors.size()));
            uint32_t i = 0;
            for(const auto &v : vectors) {
                buffer[i] = v.x();
                buffer[i+1] = v.y();
                buffer[i+2] = v.z();
                i += 3;
            }
            return buffer;
        }
    };
}


#endif //ICE_BUFFERUTILS_H
