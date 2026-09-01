#pragma once
#include "Common.h"
#include "Logging.h"
#include "GlmUtils.h"

//returns Y or Z axis, which is fine for SOME algorithms that expect "upvec" in this format
inline glm::vec3 makeNonParallelUpvec(glm::vec3 vec)
{
    vec = glm::normalize(vec);
    bool cameraDirLooksKindaLikeYAxis = abs(vec.y) > 0.999999;
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
                                            float      aspect,
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

    const glm::mat4 proj    = glm::perspective(cameraYFOVRad, aspect, nearZ, farZ);

    const glm::mat4 viewProjection = proj * view;
    return viewProjection;
}



inline glm::mat4 makeViewProjectionMatrix(  glm::vec3  cameraPos,
                                            float      cameraRollRad,
                                            glm::vec3  cameraDir,
                                            float      cameraYFOVRad,
                                            float      aspect,
                                            float      nearZ = 0.01f,
                                            float      farZ  = 1000.0f )
{
    auto [upVec, rightVec] = makeUpAndRightVectors(cameraDir, cameraRollRad);

    return makeViewProjectionMatrix(cameraPos, cameraDir, upVec, cameraYFOVRad, aspect, nearZ, farZ);
}


inline std::optional<glm::vec2> clipSpacePointToNDC2D(glm::vec4 clipSpacePoint)
{
    //i assume thats all i need to check
    if (std::abs(clipSpacePoint.w) < 0.000001f)
    {
        return std::nullopt;
    }

    const glm::vec3 ndc = glm::vec3(clipSpacePoint) / clipSpacePoint.w;

    return glm::vec2(ndc.x, ndc.y);
}

// Returns screen coord of 'worldPos' in [-1,-1], [1, 1] range,
// or nullopt if point not in frustum
inline std::optional<glm::vec2> worldToScreen11(const glm::mat4& viewProjection,
                                                glm::vec3        worldPoint)
{
    // Transform world position to clip space
    const glm::vec4 clipSpacePoint = viewProjection * glm::vec4(worldPoint, 1.0f);

    return clipSpacePointToNDC2D(clipSpacePoint);
}


using IntPlaneIndex = int; //[0, 5]
inline constexpr int PlanesCount = 6;

//Return value: distance from point to that plane.
//Positive value: point is on the inside of plane.
//Negative value: point is on the outside of plane
inline float signedDistToClipspacePlane(const glm::vec4& c, IntPlaneIndex plane)
{
    switch (plane) 
    {
        case 0: return c.w + c.x; // x >= -w
        case 1: return c.w - c.x; // x <=  w
        case 2: return c.w + c.y; // y >= -w
        case 3: return c.w - c.y; // y <=  w
        case 4: return c.w + c.z; // z >= -w
        case 5: return c.w - c.z; // z <=  w
    }

    SV_UNREACHABLE();
}

using ClipspaceLine = std::pair<glm::vec4, glm::vec4>;
SV_DECL_OPT(ClipspaceLine);

//both input and output is in clipspace.
//output is nullopt, if entire line is completely outside frustum
inline ClipspaceLineOpt clipClipspaceLineToFrustum(ClipspaceLine clipspaceLine)
{
    glm::vec4 A = clipspaceLine.first;
    glm::vec4 B = clipspaceLine.second;

    float planeDistA[PlanesCount]; 
    float planeDistB[PlanesCount];
    for (IntPlaneIndex i = 0; i < PlanesCount; ++i)
    {
        planeDistA[i] = signedDistToClipspacePlane(A, i);
        planeDistB[i] = signedDistToClipspacePlane(B, i);
    }

    //Basic checks if we accept or reject right away:
    {
        int AOutsidePlanesCount = 0;
        int BOutsidePlanesCount = 0;
        for (IntPlaneIndex i = 0; i < PlanesCount; ++i)
        {
            bool AOutside = planeDistA[i] < 0.0f;
            bool BOutside = planeDistB[i] < 0.0f;

            if (AOutside) AOutsidePlanesCount++;
            if (BOutside) BOutsidePlanesCount++;

            if (AOutside && BOutside)
            {
                // Both outside on at least one common plane -> trivial reject
                return std::nullopt;
            }
        }

        if (AOutsidePlanesCount == 0 && BOutsidePlanesCount == 0)
        {
            // Both fully inside -> trivial accept, line doesnt need clipping
            return clipspaceLine;
        }
    }

    // Clip the segment against each of the 6 frustum planes in clip space.
    // For plane i, the signed distance along the segment is:
    //   dist(t) = distA[i] + t * (distB[i] - distA[i]),  t in [0,1]
    // We need dist(t) >= 0 for all planes.

    float tEnter = 0.0f;
    float tExit = 1.0f;

    for (int i = 0; i < PlanesCount; ++i)
    {
        // Change of signed distance along the segment for this plane.
        float distDelta = planeDistB[i] - planeDistA[i];

        // We want: distA[i] + t * distDelta >= 0
        // Solve for t where dist(t) == 0:  tHit = -distA[i] / distDelta
        float tHit = -planeDistA[i] / distDelta;

        if (distDelta == 0.0f) {
            // Distance to this plane is constant along the segment.
            // If it were negative, the trivial reject test would have caught it already.
            continue;
        }

        if (distDelta < 0.0f)
        {
            // Distance decreases along the segment: we may exit the frustum through this plane.
            // The valid t-range ends at tHit (or earlier if another plane cuts sooner).
            if (tHit < tExit) {
                tExit = tHit;
            }
        }
        else
        {
            // Distance increases along the segment: we may enter the frustum through this plane.
            // The valid t-range starts at tHit (or later if another plane cuts later).
            if (tHit > tEnter) {
                tEnter = tHit;
            }
        }

        // If the entry point moves past the exit point, the segment has no visible part.
        if (tEnter > tExit) {
            return std::nullopt;
        }
    }

    // Clamp to [0,1] just in case of numeric noise
    tEnter = glm::clamp(tEnter, 0.0f, 1.0f);
    tExit = glm::clamp(tExit, 0.0f, 1.0f);

    glm::vec4 A_clipped = A + tEnter * (B - A);
    glm::vec4 B_clipped = A + tExit * (B - A);

    return ClipspaceLine{ A_clipped, B_clipped };
}

inline Vec3Opt clipSpacePointToWorld(const glm::mat4& invertedVP, glm::vec4 clipSpacePoint)
{
    // Reject near-zero w to avoid division issues
    if (std::abs(clipSpacePoint.w) < 0.000001f)
    {
        return std::nullopt;
    }

    // Convert from clip space to world space (homogeneous)
    glm::vec4 worldHom = invertedVP * clipSpacePoint;

    if (std::abs(worldHom.w) < 0.000001f)
    {
        return std::nullopt;
    }

    glm::vec3 worldPos = glm::vec3(worldHom) / worldHom.w;
    return worldPos;
}

inline Vec2PairOpt worldLineToScreen(const glm::mat4& VP, glm::vec3 worldA, glm::vec3 worldB, ClipspaceLine* outClippedLine = nullptr)
{
    glm::vec4 clipspaceA = VP * glm::vec4(worldA, 1.0f);
    glm::vec4 clipspaceB = VP * glm::vec4(worldB, 1.0f);

    ClipspaceLineOpt clippedLine = clipClipspaceLineToFrustum({ clipspaceA, clipspaceB });
    if (!clippedLine)
    {
        return std::nullopt;
    }

    auto ndcA   = clipSpacePointToNDC2D(clippedLine->first);
    auto ndcB   = clipSpacePointToNDC2D(clippedLine->second);

    if (!ndcA || !ndcB) return std::nullopt;

    if (outClippedLine)
    {
        *outClippedLine = *clippedLine;
    }

    return Vec2Pair( ndcA.value(), ndcB.value() );
}

//returns clipped ndc screen coords + corresponding clipped to screen world coords 
inline std::optional<std::pair<Vec2Pair, Vec3Pair>> worldLineToScreenAndWorldClip(const glm::mat4& VP, const glm::mat4& invertedVP, glm::vec3 worldA, glm::vec3 worldB)
{
    ClipspaceLine clippedLine;
    Vec2PairOpt ndcCoords = worldLineToScreen(VP, worldA, worldB, &clippedLine);
    if (!ndcCoords)
    {
        return std::nullopt;
    }

    Vec3Opt clippedWorldA = clipSpacePointToWorld(invertedVP, clippedLine.first);
    if (!clippedWorldA)
    {
        return std::nullopt;
    }

    Vec3Opt clippedWorldB = clipSpacePointToWorld(invertedVP, clippedLine.second);
    if (!clippedWorldB)
    {
        return std::nullopt;
    }

    return std::make_pair(*ndcCoords, std::make_pair(*clippedWorldA, *clippedWorldB));
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

    float getPitch() const
    {
        return glm::pitch(q_rotation);
    }
    float getYaw() const
    {
        return glm::yaw(q_rotation);
    }
    float getRoll() const
    {
        return glm::roll(q_rotation);
    }
    glm::vec3 getPitchYawRoll() const
    {
        return glm::eulerAngles(q_rotation);
    }

    std::string toString() const
    {
        auto pitchYawRoll        = getPitchYawRoll();
        auto pitchYawRollDegrees = glm::vec3(glm::degrees(pitchYawRoll.x),
                                             glm::degrees(pitchYawRoll.y),
                                             glm::degrees(pitchYawRoll.z));
        return std::format("POS {} PITCH-YAW-ROLL P {}", ::toString(pos), ::toString(pitchYawRollDegrees));
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
        auto right = q_rotation * glm::vec3(1, 0, 0);

        //if (isCameraUpsideDown()) right *= -1.0f;

        return right;
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

    void addAngles(glm::vec3 pitchYawRollRadians)
    {
        planeStyleCamera_addAngles_v1(pitchYawRollRadians);
    }

    bool isCameraUpsideDown() const
    {
        glm::vec3 up = getDirUp();          // local up in world space
        return up.y < 0.0f;                 // true if camera is pitched > 90° or < -90°
    }

    // This is for the "Plane style camera", which means: if you keep changing pitch,
    // it should always lets you, so you may end up upside down, it should be unlimited.
    //
    // This specific version: doesnt work, see comment inside
    void planeStyleCamera_addAngles_v1(glm::vec3 pitchYawRollRadians)
    {
        viewProjectionIsDirty = true;

        const float deltaPitch  = pitchYawRollRadians.x;
        const float deltaYaw    = pitchYawRollRadians.y;
        const float deltaRoll   = pitchYawRollRadians.z;

        //Note that each rotation makes old values of getSomeDir() outdated.

        // 1) Roll around current local
        {
            glm::quat qRoll = glm::angleAxis(deltaRoll, getDir());
            q_rotation = qRoll * q_rotation;
            q_rotation = glm::normalize(q_rotation);
        }

        // 2) Pitch around current local right
        {
            glm::quat qPitch = glm::angleAxis(deltaPitch, getDirRight());
            q_rotation = qPitch * q_rotation;
            q_rotation = glm::normalize(q_rotation);
        }

        // 3) Yaw around WORLD up (after pitch)
        {

            // 1. add second camera and look at first, check if right vec is inverted
            // 2. maybe just restore roll with getDirUp approach? add get/set roll


            // No matter which up vector i pick, its wrong, but for different reasons:
            // 
            //  1) If i select worldUp:
            //      It renders correctly, but at some angles, when camera is flipped upside down,
            //      some controls are inverted
            //
            //  2) If i select local up vector getDirUp():
            //      It renders correctly, and controls are not flipped, but even if deltaRoll is always 0,
            //      changing pitch and yaw also adds roll quite quickly. I expect roll to stay exactly what it was,
            //      if i only change pitch and yaw
            bool selectWorldUp = true;

            glm::vec3 worldUp = glm::vec3(0, 1, 0);

            if (isCameraUpsideDown())
            {
                worldUp *= -1;
            }

            glm::vec3 upSelected = selectWorldUp ? worldUp : getDirUp();


            glm::quat qYaw = glm::angleAxis(deltaYaw, upSelected);
            q_rotation = qYaw * q_rotation;
            q_rotation = glm::normalize(q_rotation);
        }
    }

    const glm::mat4& getViewProjection()
    {
        if (viewProjectionIsDirty)
        {
            updateViewProjection();
        }
        return viewProjection;
    }
    const glm::mat4& getInvertedViewProjection()
    {
        if (viewProjectionIsDirty)
        {
            updateViewProjection();
        }
        return invertedViewProjection;
    }

    std::optional<glm::vec2> worldToScreen11(glm::vec3 worldPoint)
    {
        return ::worldToScreen11(getViewProjection(), worldPoint);
    }

    void setAspect(float newAspect)
    {
        if (abs(aspect - newAspect) > 0.000001)
        {
            aspect = newAspect;
            viewProjectionIsDirty = true;
        }
    }

private:
    void updateViewProjection()
    {
        viewProjection = makeViewProjectionMatrix(pos, getDir(), getDirUp(), yFovRad, aspect, nearZ, farZ);
        invertedViewProjection = glm::inverse(viewProjection);
        viewProjectionIsDirty = false;
    }

private:
    glm::vec3 pos        = {};
    glm::quat q_rotation = {};
    float	  yFovRad    = glm::radians(80.0f);
    float     nearZ      = 0.001f;
    float     farZ       = 1000.0f;
    float     aspect     = 1.0; //w/h
private:
    // Whenever there's change of camera position/orientation,
    // this flag is set, and then VP will be lazy-updated
    bool        viewProjectionIsDirty = true;
    glm::mat4   viewProjection;
    glm::mat4   invertedViewProjection;

private:
    static const inline glm::vec3 defaultForward = glm::vec3(0.0f, 0.0f, -1.0f);
};