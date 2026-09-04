#include "CameraUtils.h"

void BasicPlaneCamera::addAngles(glm::vec3 pitchYawRollRadians)
{
    planeStyleCamera_addAngles_v2(pitchYawRollRadians);
}

void BasicPlaneCamera::planeStyleCamera_addAngles_v1(glm::vec3 pitchYawRollRadians)
{
    viewProjectionIsDirty = true;

    const float deltaPitch = pitchYawRollRadians.x;
    const float deltaYaw = pitchYawRollRadians.y;
    const float deltaRoll = pitchYawRollRadians.z;

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

        //false = correct plane style
        bool selectWorldUp = false;

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

//if you add clamping pitch it would work, but i still need get/set angles.
void BasicPlaneCamera::planeStyleCamera_addAngles_v2(glm::vec3 pitchYawRollRadians)
{
    viewProjectionIsDirty = true;

    const float pitch = pitchYawRollRadians.x;
    const float yaw = pitchYawRollRadians.y;
    const float roll = pitchYawRollRadians.z;

    // 1. Yaw: rotate around world up (Y axis)
    // This is global yaw, typical for FPS cameras.
    glm::quat qYaw = glm::angleAxis(yaw, glm::vec3(0.0f, 1.0f, 0.0f));

    // 2. Roll: rotate around local forward axis.
    // Forward in camera space is typically -Z; we transform it by current rotation.
    glm::vec3 forward = q_rotation * glm::vec3(0.0f, 0.0f, -1.0f);
    glm::quat qRoll = glm::angleAxis(roll, forward);

    // 3. Pitch: rotate around local right axis.
    // Right in camera space is typically +X; transform by current rotation.
    glm::vec3 right = q_rotation * glm::vec3(1.0f, 0.0f, 0.0f);
    glm::quat qPitch = glm::angleAxis(pitch, right);

    // Combine rotations.
    // Order matters: we apply yaw (global), then roll and pitch in local space.
    // A common approach: q_new = qYaw * qRoll * qPitch * q_old
    // But since qRoll and qPitch are already in world space (axes transformed),
    // we can just left-multiply them onto the current orientation.
    glm::quat qNew = qYaw * qRoll * qPitch * q_rotation;

    // Normalize to avoid drift.
    q_rotation = glm::normalize(qNew);
}