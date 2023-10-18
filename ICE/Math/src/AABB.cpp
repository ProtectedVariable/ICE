//
// Created by Thomas Ibanez on 29.12.20.
//

#include "AABB.h"

namespace ICE {

    AABB::AABB(const Eigen::Vector3f &min, const Eigen::Vector3f &max) : min(min), max(max) {}
    AABB::AABB(const std::vector<Eigen::Vector3f>& points) : AABB(points[0], points[0]) {
        for(auto v : points) {
            min = min.cwiseMin(v);
            max = max.cwiseMax(v);
        }
    }

    float AABB::getVolume() const {
        return (max.x() - min.x()) * (max.y() - min.y()) * (max.z() - min.z());
    }

    bool AABB::overlaps(const AABB &other) const {
        return (min.x() <= other.max.x() && max.x() >= other.min.x()) &&
                (min.y() <= other.max.y() && max.y() >= other.min.y()) &&
                (min.z() <= other.max.z() && max.z() >= other.min.z());
    }

    bool AABB::contains(const Eigen::Vector3f &point) const {
        return (point.x() >= min.x() && point.x() <= max.x()) &&
                (point.y() >= min.y() && point.y() <= max.y()) &&
                (point.z() >= min.z() && point.z() <= max.z());
    }

    AABB AABB::operator+(const AABB &other) const {
        return unionWith(other);
    }

    AABB AABB::unionWith(const AABB &other) const {
        return AABB(min.cwiseMin(other.min), max.cwiseMax(other.max));
    }

    Eigen::Vector3f AABB::getCenter() const {
        return (min + max) / 2;
    }

    const Eigen::Vector3f &AABB::getMin() const {
        return min;
    }

    const Eigen::Vector3f &AABB::getMax() const {
        return max;
    }
}