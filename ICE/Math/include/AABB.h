//
// Created by Thomas Ibanez on 29.12.20.
//

#pragma once

#include <Eigen/Dense>
#include <vector>

namespace ICE {
class AABB {
   public:
    AABB(const Eigen::Vector3f& min, const Eigen::Vector3f& max);
    AABB(const std::vector<Eigen::Vector3f>& points);

    AABB scaledBy(const Eigen::Vector3f& scale) const;
    AABB translatedBy(const Eigen::Vector3f& tr) const;
    float getVolume() const;
    bool overlaps(const AABB& other) const;
    bool contains(const Eigen::Vector3f& point) const;
    AABB operator+(const AABB& other) const;
    AABB unionWith(const AABB& other) const;
    Eigen::Vector3f getCenter() const;

    const Eigen::Vector3f& getMin() const;

    const Eigen::Vector3f& getMax() const;

   private:
    Eigen::Vector3f min, max;
};
}  // namespace ICE
