//
// Created by Thomas Ibanez on 21.11.20.
//

#ifndef ICE_CAMERA_H
#define ICE_CAMERA_H

#include <Eigen/Dense>

//Projection parameters
#define ICE_CAMERA_FOV      0x0
#define ICE_CAMERA_AR       0x1
//Common parameters
#define ICE_CAMERA_NEAR     0x2
#define ICE_CAMERA_FAR      0x3
//Orthographic parameters
#define ICE_CAMERA_TOP      0x0
#define ICE_CAMERA_BOTTOM   0x1
#define ICE_CAMERA_LEFT     0x4
#define ICE_CAMERA_RIGHT    0x5

namespace ICE {
    enum ProjectionType { Perspective, Orthographic };

    struct CameraParameters {
        float intrinsic[6];
        ProjectionType type;
    };

    class Camera {
    public:
        Camera(const CameraParameters &parameters);

        const Eigen::Matrix4f &getProjection() const;

    private:
        CameraParameters parameters;
        float zoom;
        Eigen::Matrix4f projection;
    };
}


#endif //ICE_CAMERA_H
