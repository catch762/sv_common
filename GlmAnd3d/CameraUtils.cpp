#include "CameraUtils.h"

//returns Y or Z axis, which is fine for SOME algorithms that expect "upvec" in this format
glm::vec3 makeNonParallelUpvec(glm::vec3 vec)
{
    vec = glm::normalize(vec);
    bool cameraDirLooksKindaLikeYAxis = abs(vec.y) > 0.999999;
    return cameraDirLooksKindaLikeYAxis ? glm::vec3(0, 0, 1) :
        glm::vec3(0, 1, 0);
}

//yaw will be [-180,180], correct pitch will be only in [-90 to 90]
//i mean its all in radians ofcourse
std::pair<float, float> dirToYawPitch(const glm::vec3& dir)
{
    // dir must be normalized
    // yaw: angle around Y, measured from -Z towards +X
    float yaw = std::atan2(dir.x, -dir.z);

    // pitch: angle up/down from XZ plane
    float pitch = std::asin(dir.y); // dir.y == sin(pitch)

    return { pitch, yaw };
}

std::pair<glm::vec3, glm::vec3> makeUpAndRightVectors(glm::vec3 cameraForwardDir, float cameraRollRad)
{
    cameraForwardDir = glm::normalize(cameraForwardDir);

    // Start with a world-up that is not parallel to forward
    glm::vec3 worldUp = makeNonParallelUpvec(cameraForwardDir);

    // Initial right and up (no roll yet)
    glm::vec3 right = glm::normalize(glm::cross(worldUp, cameraForwardDir));
    glm::vec3 up = glm::normalize(glm::cross(cameraForwardDir, right));

    // Apply roll: rotate (right, up) around 'forward' by 'cameraRollRad'
    float c = std::cos(cameraRollRad);
    float s = std::sin(cameraRollRad);

    glm::vec3 rolledRight = right * c + up * s;
    glm::vec3 rolledUp = up * c - right * s;

    // Optional: re-normalize to fight drift
    return { glm::normalize(rolledUp),
             glm::normalize(-rolledRight) }; //note the minus
}

glm::mat4 makeViewProjectionMatrix(glm::vec3  cameraPos,
    glm::vec3  cameraDir,
    glm::vec3  cameraDirUp,
    float      cameraYFOVRad,
    float      aspect,
    float      nearZ,
    float      farZ)
{
    cameraDir = glm::normalize(cameraDir);
    cameraDirUp = glm::normalize(cameraDirUp);

    const glm::vec3 lookAtPos = cameraPos + cameraDir;
    const glm::mat4 view = glm::lookAt(cameraPos, lookAtPos, cameraDirUp);

    // Projection matrix: we only need a symmetric perspective with given FOV.
    // Aspect ratio is irrelevant for NDC x/y in [-1,1] if we treat the
    // projection as square; we can just use aspect = 1.
    // Near/far can be arbitrary positive values as long as near < far.

    const glm::mat4 proj = glm::perspective(cameraYFOVRad, aspect, nearZ, farZ);

    const glm::mat4 viewProjection = proj * view;
    return viewProjection;
}



glm::mat4 makeViewProjectionMatrix(glm::vec3  cameraPos,
    float      cameraRollRad,
    glm::vec3  cameraDir,
    float      cameraYFOVRad,
    float      aspect,
    float      nearZ,
    float      farZ)
{
    auto [upVec, rightVec] = makeUpAndRightVectors(cameraDir, cameraRollRad);

    return makeViewProjectionMatrix(cameraPos, cameraDir, upVec, cameraYFOVRad, aspect, nearZ, farZ);
}


std::optional<glm::vec2> clipSpacePointToNDC2D(glm::vec4 clipSpacePoint)
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
std::optional<glm::vec2> worldToScreen11(const glm::mat4& viewProjection,
                                               glm::vec3  worldPoint)
{
    // Transform world position to clip space
    const glm::vec4 clipSpacePoint = viewProjection * glm::vec4(worldPoint, 1.0f);

    return clipSpacePointToNDC2D(clipSpacePoint);
}


using IntPlaneIndex = int; //[0, 5]
constexpr int PlanesCount = 6;

//Return value: distance from point to that plane.
//Positive value: point is on the inside of plane.
//Negative value: point is on the outside of plane
float signedDistToClipspacePlane(const glm::vec4& c, IntPlaneIndex plane)
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
ClipspaceLineOpt clipClipspaceLineToFrustum(ClipspaceLine clipspaceLine)
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

Vec3Opt clipSpacePointToWorld(const glm::mat4& invertedVP, glm::vec4 clipSpacePoint)
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

Vec2PairOpt worldLineToScreen(const glm::mat4& VP, glm::vec3 worldA, glm::vec3 worldB, ClipspaceLine* outClippedLine)
{
    glm::vec4 clipspaceA = VP * glm::vec4(worldA, 1.0f);
    glm::vec4 clipspaceB = VP * glm::vec4(worldB, 1.0f);

    ClipspaceLineOpt clippedLine = clipClipspaceLineToFrustum({ clipspaceA, clipspaceB });
    if (!clippedLine)
    {
        return std::nullopt;
    }

    auto ndcA = clipSpacePointToNDC2D(clippedLine->first);
    auto ndcB = clipSpacePointToNDC2D(clippedLine->second);

    if (!ndcA || !ndcB) return std::nullopt;

    if (outClippedLine)
    {
        *outClippedLine = *clippedLine;
    }

    return Vec2Pair(ndcA.value(), ndcB.value());
}

//returns clipped ndc screen coords + corresponding clipped to screen world coords 
std::optional<std::pair<Vec2Pair, Vec3Pair>> worldLineToScreenAndWorldClip( const glm::mat4& VP,
                                                                            const glm::mat4& invertedVP,
                                                                            glm::vec3 worldA,
                                                                            glm::vec3 worldB )
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