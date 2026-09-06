#pragma once
#include "Common.h"
#include "Logging.h"
#include "GlmUtils.h"

//returns Y or Z axis, which is fine for SOME algorithms that expect "upvec" in this format
glm::vec3 makeNonParallelUpvec(glm::vec3 vec);

//yaw will be [-180,180], correct pitch will be only in [-90 to 90]
//i mean its all in radians ofcourse
std::pair<float, float> dirToYawPitch(const glm::vec3& dir);

std::pair<glm::vec3, glm::vec3> makeUpAndRightVectors(glm::vec3 cameraForwardDir, float cameraRollRad);

glm::mat4 makeViewProjectionMatrix( glm::vec3  cameraPos,
                                    glm::vec3  cameraDir,
                                    glm::vec3  cameraDirUp,
                                    float      cameraYFOVRad,
                                    float      aspect,
                                    float      nearZ = 0.01f,
                                    float      farZ  = 1000.0f );



glm::mat4 makeViewProjectionMatrix( glm::vec3  cameraPos,
                                    float      cameraRollRad,
                                    glm::vec3  cameraDir,
                                    float      cameraYFOVRad,
                                    float      aspect,
                                    float      nearZ = 0.01f,
                                    float      farZ  = 1000.0f );


std::optional<glm::vec2> clipSpacePointToNDC2D(glm::vec4 clipSpacePoint);

// Returns screen coord of 'worldPos' in [-1,-1], [1, 1] range,
// or nullopt if point not in frustum
std::optional<glm::vec2> worldToScreen11(const glm::mat4& viewProjection,
                                               glm::vec3  worldPoint);

using ClipspaceLine = std::pair<glm::vec4, glm::vec4>;
SV_DECL_OPT(ClipspaceLine);

//both input and output is in clipspace.
//output is nullopt, if entire line is completely outside frustum
ClipspaceLineOpt clipClipspaceLineToFrustum(ClipspaceLine clipspaceLine);

Vec3Opt clipSpacePointToWorld(const glm::mat4& invertedVP, glm::vec4 clipSpacePoint);

Vec2PairOpt worldLineToScreen(const glm::mat4&  VP, 
                              glm::vec3         worldA, 
                              glm::vec3         worldB, 
                              ClipspaceLine*    outClippedLine = nullptr);

//returns clipped ndc screen coords + corresponding clipped to screen world coords 
std::optional<std::pair<Vec2Pair, Vec3Pair>> worldLineToScreenAndWorldClip( const glm::mat4&    VP, 
                                                                            const glm::mat4&    invertedVP, 
                                                                            glm::vec3           worldA, 
                                                                            glm::vec3           worldB );