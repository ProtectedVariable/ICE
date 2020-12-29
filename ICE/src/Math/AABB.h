//
// Created by Thomas Ibanez on 29.12.20.
//

#ifndef ICE_AABB_H
#define ICE_AABB_H

#include <Eigen/Dense>
#include <vector>

namespace ICE {
    class AABB {
    public:
        AABB(const Eigen::Vector3f& min, const Eigen::Vector3f& max);
        AABB(const std::vector<Eigen::Vector3f>& points);

        float getVolume();
        bool overlaps(const AABB& other);
        bool contains(const Eigen::Vector3f& point);
        AABB operator+(const AABB& other);
        AABB unionWith(const AABB& other);
        Eigen::Vector3f getCenter();

        const Eigen::Vector3f &getMin() const;

        const Eigen::Vector3f &getMax() const;

    private:
        Eigen::Vector3f min, max;
    };
}


#endif //ICE_AABB_H
