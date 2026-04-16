#include "camera.hpp"

Camera::Camera()
{
    CameraProperties properties{};
    setProperties(properties);
}

Camera::Camera(const CameraProperties properties)
{
    setProperties(properties);
}

void Camera::setProperties(const CameraProperties properties)
{
    this->properties = properties;
    updateCamera();
}

void Camera::setNear(const float near)
{
    properties.near = near;
    updateCamera();
}

void Camera::setFar(const float far)
{
    properties.far = far;
    updateCamera();
}

void Camera::setVfov(const float vfov)
{
    properties.vFov = vfov;
    updateCamera();
}

void Camera::setAspectRatio(const float _aspectRatio)
{
    aspectRatio = _aspectRatio;
    // std::cout << "Aspect ratio: " << aspectRatio << std::endl;
    updateCamera();
}

void Camera::updateCamera()
{
    float near = properties.reversedDepth ? properties.far : properties.near;
    float far = properties.reversedDepth ? properties.near : properties.far;
    if(properties.orthographic)
    {
        float top = properties.vFov * 0.5f;
        float bottom = -top;
        float right = aspectRatio * top;
        float left = -right;

        projection = glm::mat4(
            2.f / (right - left), 0, 0, 0,
            0, 2.f / (bottom - top), 0, 0,
            0, 0, 1.f / (far - near), 0,
            -(right + left) / (right - left), -(bottom + top) / (bottom - top), near / (far - near), 1.f
        );
    }
    else
    {
        float fovY = 2.0f / (glm::tan(glm::radians(properties.vFov * 0.5f)));
        if(!properties.infiniteFarPlane)
            projection = glm::mat4(
                fovY / (aspectRatio), 0, 0, 0,
                0, -fovY, 0, 0,
                0, 0, far / (far - near), 1.f,
                0, 0, -near * far / (far - near), 0
            );
        else
        {
            near = properties.near;
            // fovY = fovY * near;
            projection = glm::mat4(
                fovY / aspectRatio, 0, 0, 0,
                0, -fovY, 0, 0,
                0, 0, 0, 1,
                0, 0, near, 0
            );
        }
    }

    // projection = orthographic * perspective;
}