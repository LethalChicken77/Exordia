#pragma once
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

#include "core/components/transform.hpp"

struct CameraProperties
{
    /// @brief Distance in world units to the near clipping plane.
    float near = 0.01f;
    /// @brief Distance in world units to the far clipping plane.
    float far = 1000.f; 
    // float whereverYouAre;
    
    /// @brief Vertical field of view.
    ///
    /// Perspective mode: Represents the vertical FoV angle in degrees.
    ///
    /// Orthographic mode: Represents the vertical height in world units.
    float vFov = 80.f;
    /// @brief Determines whether this is a perspective or orthographic camera.
    bool orthographic = false;
    /// @brief Whether to flip the near and far planes. Used for reversed depth buffers.
    bool reversedDepth = true;
};

class Camera
{
public:
    
    Camera();
    Camera(const CameraProperties properties);

    // Camera(const Camera&) = delete;
    // Camera& operator=(const Camera&) = delete;

    void setProperties(const CameraProperties properties);
    void setNear(const float near);
    void setFar(const float far);
    void setVfov(const float vfov);
    void setAspectRatio(const float aspectRatio);

    glm::mat4 getView() const { return view; }
    glm::mat4 getProjection() const { return projection; }
    glm::mat4 getViewProjection() const { return projection * glm::inverse(view); }

    void SetTransform(glm::mat4 transform) { view = transform; }
private:
    glm::mat4 perspective;
    glm::mat4 orthographic;

    glm::mat4 projection;
    glm::mat4 view;

    CameraProperties properties;
    float aspectRatio = 1; // Horizontal / Vertical

    void updateCamera();
};