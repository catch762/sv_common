#pragma once
#include "CameraUtils.h"

//******************************************************************************************
// Notes on glm behaviour, assuming q is glm::quat:
// 
// 1. Getting angles:
//      glm::pitch(q)   - OK, [-180 to 180]
//      glm::yaw(q)     - SHIT, [0 to 90 to 0 to -90 to 0]
//      glm::roll(q)    - OK, [-180 to 180]
// 
// 2. Setting angles like this:
//      q = glm::quat({pitch, yaw, roll});
//          -> Yaw is around global Y, pitch around local right, and roll around global Z.
// 
// 2. Plane-style rotation in local coord system:
//      q *= glm::quat(glm::vec3(deltaPitch, deltaYaw, deltaRoll));
//      It works, but issue is, getting new std angles after this operation.
//******************************************************************************************

//******************************************************************************************
// So, BasicFPSCamera class.
// 
// "FPS" means its like in first person shooters, i.e. pitch is limited by ~[-90, 90] degrees;
// There is however 'roll' setting, so you still may end up upside down (with inverted controls).
// 
// It stores angle as floats, and reconstructs a quaternion each time you change an angle.
// 
// "FPS style" camera is opposed to what i would call "free-flight plane camera"
//******************************************************************************************
class BasicFPSCamera
{
public:
    BasicFPSCamera() = default;

    //*********************************
    // Note! Rotation order is:
    //      - yaw around global Y
    //      - pitch around local right
    //      - roll around local forward
    //*********************************

    glm::vec3   getPos() const;
    void        setPos(glm::vec3 newPos);
    void        moveBy(glm::vec3 localright_globalup_localforward);

    float       getPitch() const;
    float       getYaw() const;
    float       getRoll() const;
    glm::vec3   getPitchYawRoll() const;

    void        setPitch(float newPitch);
    void        setYaw(float newYaw);
    void        setRoll(float newRoll);
    void        addAngles(glm::vec3 pitchYawRollRadians);
    void        lookAtWithoutRoll(glm::vec3 lookAtPos); //roll will be set to zero

    glm::vec3   getDir() const;
    glm::vec3   getDirUp() const;
    glm::vec3   getDirRight() const;

    float       getYFov() const;
    void        setYFov(float newYFovRad);
    float       getAspect() const;
    void        setAspect(float newAspect);

    std::string toString() const;

    const glm::mat4&            getViewProjection();
    const glm::mat4&            getInvertedViewProjection();
    std::optional<glm::vec2>    worldToScreen11(glm::vec3 worldPoint);

private:
    bool isCameraUpsideDown() const;
    void rebuildQuaternionFromAngles();
    void updateViewProjection();

private:
    glm::vec3   pos         = {0,0,0};
    float	    yFovRad     = glm::radians(60.0f);
    float       nearZ       = 0.001f;
    float       farZ        = 1000.0f;
    float       aspect      = 1.0; //w/h

    float       yaw         = 0.0f; //1st rotation: yaw around global Y
    float       pitch       = 0.0f; //2nd rotation: pitch around local right
    float       roll        = 0.0f; //3rd rotation: roll around local forward
    glm::quat   q_rotation  = {};

private:
    // Whenever there's change of camera position/orientation,
    // this flag is set, and then matrices will be lazy-updated
    bool        viewProjectionIsDirty = true;
    glm::mat4   viewProjection;
    glm::mat4   invertedViewProjection; //needed to convert clipspace coords back to world

private:
    static constexpr float      minPitch = -glm::radians(89.0f);
    static constexpr float      maxPitch = glm::radians(89.0f);
    static constexpr glm::vec3  defaultForward = glm::vec3(0.0f, 0.0f, -1.0f);
};