#pragma once
#include "Common.h"
#include "Logging.h"
#include "GlmUtils.h"

//returns Y or Z axis, which is fine for SOME algorithms that expect "upvec" in this format
inline glm::vec3 makeNonParallelUpvec(glm::vec3 vec)
{
    vec = glm::normalize(vec);
    bool cameraDirLooksKindaLikeYAxis = abs(vec.y) > 0.99999;
    return cameraDirLooksKindaLikeYAxis ?   glm::vec3(0, 0, 1) :
                                            glm::vec3(0, 1, 0);
}

inline std::pair<glm::vec3, glm::vec3> makeUpAndRightVectors(glm::vec3 cameraForwardDir, float cameraRollRad)
{
    cameraForwardDir = glm::normalize(cameraForwardDir);

    // Start with a world-up that is not parallel to forward
    glm::vec3 worldUp = makeNonParallelUpvec(cameraForwardDir);

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

// "Plane" means its angle-unconstrained, i.e. if you keep 
// increasing pitch you ll end up looking upside down.
class BasicPlaneCamera
{
public:
    BasicPlaneCamera() = default;

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
        return q_rotation * defaultForward;
    }
    glm::vec3 getDirUp() const
    {
        return q_rotation * glm::vec3(0, 1, 0);
    }
    glm::vec3 getDirRight() const
    {
        return q_rotation * glm::vec3(1, 0, 0);
    }
    void lookAtWithoutRoll(glm::vec3 lookAtPos)
    {
        glm::vec3 newDir = lookAtPos - pos;

        if (glm::length(newDir) < 0.0000001f)
        {
            return;
        }

        newDir = glm::normalize(newDir);
        
        auto [upVec, rightVec] = makeUpAndRightVectors(newDir, 0);
        
        q_rotation = glm::quatLookAt(newDir, upVec);

        viewProjectionIsDirty = true;
    }
    void moveBy(glm::vec3 movementRightUpForward)
    {
        glm::vec3 movement = movementRightUpForward.x * getDirRight () +
                             movementRightUpForward.y * getDirUp    () +
                             movementRightUpForward.z * getDir      ();

        setPos(getPos() + movement);
    }

    void addAngles(glm::vec3 eulerPitchYawRollRadians)
    {
        q_rotation *= glm::quat(eulerPitchYawRollRadians);
        viewProjectionIsDirty = true;
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
        viewProjection = makeViewProjectionMatrix(pos, getDir(), getDirUp(), yFovRad, nearZ, farZ);
    }

private:
    glm::vec3 pos        = {};
    glm::quat q_rotation = {};
    float	  yFovRad    = glm::radians(80.0f);
    float     nearZ      = 0.001f;
    float     farZ       = 1000.0f;

private:
    // Whenever there's change of camera position/orientation,
    // this flag is set, and then VP will be lazy-updated
    bool        viewProjectionIsDirty = true;
    glm::mat4   viewProjection;

private:
    static const inline glm::vec3 defaultForward = glm::vec3(0.0f, 0.0f, -1.0f);
};