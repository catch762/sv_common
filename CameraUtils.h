#pragma once
#include "Common.h"
#include "Logging.h"
#include "GlmUtils.h"

inline std::pair<glm::vec3, glm::vec3> makeUpAndRightVectors(glm::vec3 cameraForwardDir, float cameraRollRad)
{
    cameraForwardDir = glm::normalize(cameraForwardDir);

    // Start with a world-up that is not parallel to forward
    bool cameraDirLooksKindaLikeYAxis = abs(cameraForwardDir.y) > 0.9;
    glm::vec3 worldUp = cameraDirLooksKindaLikeYAxis ?  glm::vec3(0, 0, 1) :
                                                        glm::vec3(0, 1, 0);

    // Initial right and up (no roll yet)
    glm::vec3 right = glm::normalize(glm::cross(worldUp, cameraForwardDir));
    glm::vec3 up    = glm::normalize(glm::cross(cameraForwardDir, right));

    // Apply roll: rotate (right, up) around 'forward' by 'cameraRollRad'
    float c = std::cos(cameraRollRad);
    float s = std::sin(cameraRollRad);

    glm::vec3 rolledRight   = right * c + up * s;
    glm::vec3 rolledUp      = up * c - right * s;

    // Optional: re-normalize to fight drift
    return { glm::normalize(rolledUp), 
             glm::normalize(-rolledRight) }; //note the minus
}

inline glm::mat4 makeViewProjectionMatrix(  glm::vec3  cameraPos,
                                            glm::vec3  cameraDir,
                                            glm::vec3  cameraDirUp,
                                            float      cameraYFOVRad,
                                            float      nearZ = 0.01f,
                                            float      farZ  = 1000.0f )
{
    cameraDir   = glm::normalize(cameraDir);
    cameraDirUp = glm::normalize(cameraDirUp);

    const glm::vec3 lookAtPos = cameraPos + cameraDir;
    const glm::mat4 view      = glm::lookAt(cameraPos, lookAtPos, cameraDirUp);

    // Projection matrix: we only need a symmetric perspective with given FOV.
    // Aspect ratio is irrelevant for NDC x/y in [-1,1] if we treat the
    // projection as square; we can just use aspect = 1.
    // Near/far can be arbitrary positive values as long as near < far.
    const float     aspect  = 1.0f; // square frustum for pure NDC
    const glm::mat4 proj    = glm::perspective(cameraYFOVRad, aspect, nearZ, farZ);

    const glm::mat4 viewProjection = proj * view;
    return viewProjection;
}


inline glm::mat4 makeViewProjectionMatrix(  glm::vec3  cameraPos,
                                            float      cameraRollRad,
                                            glm::vec3  cameraDir,
                                            float      cameraYFOVRad,
                                            float      nearZ = 0.01f,
                                            float      farZ  = 1000.0f )
{
    auto [upVec, rightVec] = makeUpAndRightVectors(cameraDir, cameraRollRad);

    return makeViewProjectionMatrix(cameraPos, cameraDir, upVec, cameraYFOVRad, nearZ, farZ);
}



// Returns screen coord of 'worldPos' in [-1,-1], [1, 1] range,
// or nullopt if point not in frustum
inline std::optional<glm::vec2> worldToScreen11(const glm::mat4& viewProjection,
                                                glm::vec3        worldPoint)
{
    // Transform world position to clip space
    const glm::vec4 clipSpacePoint = viewProjection * glm::vec4(worldPoint, 1.0f);

    // Perspective divide to NDC
    if (std::abs(clipSpacePoint.w) < 0.0000001f)
    {
        return std::nullopt;
    }

    const glm::vec3 ndc = glm::vec3(clipSpacePoint) / clipSpacePoint.w;

    // Check if inside the canonical view volume [-1, 1]^3
    if ( std::abs(ndc.x) > 1.0f ||
         std::abs(ndc.y) > 1.0f ||
         std::abs(ndc.z) > 1.0f )
    {
        return std::nullopt;
    }

    return glm::vec2(ndc.x, ndc.y);
}

class BasicCamera
{
public:
    BasicCamera() = default;

    glm::vec3 getPos() const
    {
        return pos;
    }
    void setPos(glm::vec3 newPos)
    {
        viewProjectionIsDirty = true;
        pos = newPos;
    }

    glm::vec3 getDir() const
    {
        return dir;
    }
    void setDir(glm::vec3 newDir)
    {
        if (glm::length(newDir) < 0.000001) return;

        viewProjectionIsDirty = true;
        dir = newDir;
    }

    std::pair<glm::vec3, glm::vec3> getUpAndRightDirs()
    {
        return makeUpAndRightVectors(getDir(), getRoll());
    }

    void moveBy(glm::vec3 movementRightUpForward)
    {
        auto [upDir, rightDir] = getUpAndRightDirs();

        glm::vec3 movement = movementRightUpForward.x * rightDir +
                             movementRightUpForward.y * upDir    +
                             movementRightUpForward.z * getDir();

        setPos(getPos() + movement);
    }

    float getRoll() const
    {
        return rollRad;
    }
    void setRoll(float newRollRad)
    {
        viewProjectionIsDirty = true;
        rollRad = newRollRad;
    }

    float getPitch() const
    {
        return std::asin(glm::clamp(dir.y, -1.0f, 1.0f));
    }
    void setPitch(float newPitch)
    {
        float curPitch = getPitch();
        float deltaPitch = newPitch - curPitch;

        auto [upDir, rightDir] = getUpAndRightDirs();

        glm::vec3 rotatedDir = glm::angleAxis(deltaPitch, rightDir) * getDir();

        SV_LOG(std::format("pitch [{}] new [{}] delta [{}] curdir [{}] rotdir [{}]", 
                            curPitch, newPitch, deltaPitch, getDir(), rotatedDir));

        setDir(rotatedDir);
    }

    float getYaw() const
    {
        // Project dir onto XZ plane
        float x = dir.x;
        float z = dir.z;
        return std::atan2(x, -z); // typical: -Z is forward, +X is right
    }
    void setYaw(float newYaw)
    {
        float deltaYaw = newYaw - getYaw();

        auto [upDir, rightDir] = getUpAndRightDirs();

        glm::vec3 rotatedDir = glm::angleAxis(deltaYaw, upDir) * getDir();

        setDir(rotatedDir);
    }


    void lookAt(glm::vec3 worldPoint)
    {
        glm::vec3 vecToPoint = worldPoint - pos;
        if (glm::length(vecToPoint) < 0.0000001) return;

        setDir(glm::normalize(vecToPoint));
        setRoll(0);
    }

    const glm::mat4& getViewProjection()
    {
        if (viewProjectionIsDirty)
        {
            updateViewProjection();
            viewProjectionIsDirty = false;
        }
        return viewProjection;
    }

    std::optional<glm::vec2> worldToScreen11(glm::vec3 worldPoint)
    {
        return ::worldToScreen11(getViewProjection(), worldPoint);
    }

private:
    void updateViewProjection()
    {
        viewProjection = makeViewProjectionMatrix(pos, rollRad, dir, yFovRad, nearZ, farZ);
    }

private:
    glm::vec3 pos        = {};
    glm::vec3 dir        = glm::vec3(0, 0, -1);
    float     rollRad    = 0;
    float	  yFovRad    = glm::radians(45.0f);

    float     nearZ      = 0.001f;
    float     farZ       = 1000.0f;

private:
    //Whenever there's change which changes
    bool        viewProjectionIsDirty = true;
    glm::mat4   viewProjection;
};