#pragma once

#include <memory>

#include "RenderList.h"
#include "Math.h"

class ComponentCamera;
class Framebuffer;
class Texture2D;
class Program;
class Buffer;

class ShadowmapPass
{
    std::unique_ptr<Framebuffer> frameBuffer;
    std::unique_ptr<Texture2D>   depthTex;
    std::unique_ptr<Program>     program;
    RenderList                   objects;
    Frustum                      frustum;
    OBB                          lightOBB;
    float3                       sphereCenter;
    float                        sphereRadius;
    std::unique_ptr<Buffer>      cameraUBO;

public:
    ShadowmapPass();
    ~ShadowmapPass();

    void execute(ComponentCamera* camera);

    void debugDraw();

    const Texture2D* getDepthTex() const {return depthTex.get();}
    const Frustum& getFrustum() const {return frustum;}

private:
    void updateFrustum(ComponentCamera* camera);
    void createFramebuffer();
    void createProgram();
    void updateCameraUBO();

};