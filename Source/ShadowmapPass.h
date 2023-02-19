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
    uint                         fbWidth = 0;
    uint                         fbHeight = 0;

public:
    ShadowmapPass();
    ~ShadowmapPass();


    void updateRenderList(const Frustum& culling);
    void execute(uint width, uint height);

    void debugDraw();

    const Texture2D* getDepthTex() const {return depthTex.get();}
    const Frustum& getFrustum() const {return frustum;}
    const RenderList& getRenderList() const { return objects; }

private:
    void updateFrustum(const Frustum& culling);
    void createFramebuffer(uint width, uint height);
    void createProgram();
    void updateCameraUBO();
    void render();

};