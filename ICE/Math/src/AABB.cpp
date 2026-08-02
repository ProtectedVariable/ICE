//
// Created by Thomas Ibanez on 29.12.20.
//

#include "AABB.h"

namespace ICE {

AABB::AABB(const Eigen::Vector3f &a, const Eigen::Vector3f &b) {
    // Set corners directly (no heap-allocated std::vector). cwiseMin/Max keeps min <= max
    // even when callers pass swapped corners (e.g. scaledBy with a negative scale).
    min = a.cwiseMin(b);
    max = a.cwiseMax(b);
    precomputeCenterAndExtent();
}
AABB::AABB(const std::vector<Eigen::Vector3f> &points) {
    if (points.empty()) {
        min = max = Eigen::Vector3f::Zero();
        precomputeCenterAndExtent();
        return;
    }
    min = points[0];
    max = points[0];
    for (const auto &v : points) {
        min = min.cwiseMin(v);
        max = max.cwiseMax(v);
    }
    precomputeCenterAndExtent();
}

AABB AABB::scaledBy(const Eigen::Vector3f &scale) const {
    return AABB(min.cwiseProduct(scale), max.cwiseProduct(scale));
}
AABB AABB::translatedBy(const Eigen::Vector3f &tr) const {
    return AABB(min + tr, max + tr);
}
float AABB::getVolume() const {
    return (max.x() - min.x()) * (max.y() - min.y()) * (max.z() - min.z());
}

bool AABB::overlaps(const AABB &other) const {
    return (min.x() <= other.max.x() && max.x() >= other.min.x()) && (min.y() <= other.max.y() && max.y() >= other.min.y())
        && (min.z() <= other.max.z() && max.z() >= other.min.z());
}

bool AABB::contains(const Eigen::Vector3f &point) const {
    return (point.x() >= min.x() && point.x() <= max.x()) && (point.y() >= min.y() && point.y() <= max.y())
        && (point.z() >= min.z() && point.z() <= max.z());
}

AABB AABB::operator+(const AABB &other) const {
    return unionWith(other);
}

AABB AABB::unionWith(const AABB &other) const {
    return AABB(min.cwiseMin(other.min), max.cwiseMax(other.max));
}

Eigen::Vector3f AABB::getCenter() const {
    return center;
}

Eigen::Vector3f AABB::getExtent() const {
    return extent;
}

const Eigen::Vector3f &AABB::getMin() const {
    return min;
}

const Eigen::Vector3f &AABB::getMax() const {
    return max;
}

void AABB::precomputeCenterAndExtent() {
    center = (min + max) * 0.5f;
    extent = (max - min) * 0.5f;
}
}  // namespace ICE