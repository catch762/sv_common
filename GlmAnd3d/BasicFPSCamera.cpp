#include "BasicFPSCamera.h"

glm::vec3 BasicFPSCamera::getPos() const
{
    return pos;
}
void BasicFPSCamera::setPos(glm::vec3 newPos)
{
    viewProjectionIsDirty = true;
    pos = newPos;
}

float BasicFPSCamera::getPitch() const
{
    return pitch;
}
float BasicFPSCamera::getYaw() const
{
    return yaw;
}
float BasicFPSCamera::getRoll() const
{
    return roll;
}

float BasicFPSCamera::getYFov() const
{
    return yFovRad;
}
void BasicFPSCamera::setYFov(float newYFovRad)
{
    yFovRad = newYFovRad;
    viewProjectionIsDirty = true;
}

float BasicFPSCamera::getAspect() const
{
    return aspect;
}

void BasicFPSCamera::setPitch(float newPitch)
{
    pitch = normalizeAngle_minusPi_Pi(newPitch);
    pitch = std::clamp(pitch, minPitch, maxPitch);

    viewProjectionIsDirty = true;
    rebuildQuaternionFromAngles();
}
void BasicFPSCamera::setYaw(float newYaw)
{
    yaw = normalizeAngle_minusPi_Pi(newYaw);

    viewProjectionIsDirty = true;
    rebuildQuaternionFromAngles();
}

void BasicFPSCamera::setRoll(float newRoll)
{
    roll = normalizeAngle_minusPi_Pi(newRoll);

    viewProjectionIsDirty = true;
    rebuildQuaternionFromAngles();
}

void BasicFPSCamera::rebuildQuaternionFromAngles()
{
    //Note:




    q_rotation = glm::quat({ pitch, yaw, 0 });

    glm::quat qRoll = glm::angleAxis(roll, getDir());

    q_rotation = qRoll * q_rotation;
}

glm::vec3 BasicFPSCamera::getPitchYawRoll() const
{
    return { getPitch(), getYaw(), getRoll() };
}

std::string BasicFPSCamera::toString() const
{
    auto [otherPitch, otherYaw] = dirToYawPitch(getDir());

    auto pitchYawRoll = getPitchYawRoll();
    auto pitchYawRollDegrees = glm::vec3(glm::degrees(pitchYawRoll.x),
        glm::degrees(pitchYawRoll.y),
        glm::degrees(pitchYawRoll.z));
    return std::format("POS {} PYR {} PY {} {}",
        ::toString(pos), ::toString(pitchYawRollDegrees), glm::degrees(otherPitch), glm::degrees(otherYaw));
}

glm::vec3 BasicFPSCamera::getDir() const
{
    return q_rotation * defaultForward;
}
glm::vec3 BasicFPSCamera::getDirUp() const
{
    return q_rotation * glm::vec3(0, 1, 0);
}
glm::vec3 BasicFPSCamera::getDirRight() const
{
    auto right = q_rotation * glm::vec3(1, 0, 0);

    return right;
}

void BasicFPSCamera::lookAtWithoutRoll(glm::vec3 lookAtPos)
{
    glm::vec3 newDir = lookAtPos - pos;

    if (glm::length(newDir) < 0.0000001f)
    {
        return;
    }

    newDir = glm::normalize(newDir);

    auto [newPitch, newYaw] = dirToYawPitch(newDir);

    setPitch(newPitch);
    setYaw(-newYaw);
    setRoll(0.0);
}

void BasicFPSCamera::moveBy(glm::vec3 movementRightUpForward)
{
    glm::vec3 movement = movementRightUpForward.x * getDirRight() +
        movementRightUpForward.y * getDirUp() +
        movementRightUpForward.z * getDir();

    setPos(getPos() + movement);
}

void BasicFPSCamera::addAngles(glm::vec3 pitchYawRollRadians)
{
    if (std::abs(pitchYawRollRadians.x) > 0.0000001)
    {
        setPitch(pitch + pitchYawRollRadians.x);
    }
    if (std::abs(pitchYawRollRadians.y) > 0.0000001)
    {
        setYaw(yaw + pitchYawRollRadians.y);
    }
    if (std::abs(pitchYawRollRadians.z) > 0.0000001)
    {
        setRoll(roll + pitchYawRollRadians.z);
    }
}

bool BasicFPSCamera::isCameraUpsideDown() const
{
    glm::vec3 up = getDirUp();          // local up in world space
    return up.y < 0.0f;                 // true if camera is pitched > 90° or < -90°
}

const glm::mat4& BasicFPSCamera::getViewProjection()
{
    if (viewProjectionIsDirty)
    {
        updateViewProjection();
    }
    return viewProjection;
}
const glm::mat4& BasicFPSCamera::getInvertedViewProjection()
{
    if (viewProjectionIsDirty)
    {
        updateViewProjection();
    }
    return invertedViewProjection;
}

std::optional<glm::vec2> BasicFPSCamera::worldToScreen11(glm::vec3 worldPoint)
{
    return ::worldToScreen11(getViewProjection(), worldPoint);
}

void BasicFPSCamera::setAspect(float newAspect)
{
    if (abs(aspect - newAspect) > 0.000001)
    {
        aspect = newAspect;
        viewProjectionIsDirty = true;
    }
}

void BasicFPSCamera::updateViewProjection()
{
    viewProjection = makeViewProjectionMatrix(pos, getDir(), getDirUp(), yFovRad, aspect, nearZ, farZ);
    invertedViewProjection = glm::inverse(viewProjection);
    viewProjectionIsDirty = false;
}