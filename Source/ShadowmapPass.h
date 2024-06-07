#pragma once

#include <memory>

#include "BatchDrawCommands.h"
#include "Math.h"

class ComponentCamera;
class Framebuffer;
class Texture2D;
class Program;
class Buffer;
class GaussianBlur;

class ShadowmapPass
{
    std::unique_ptr<Framebuffer>    frameBuffer;
    std::unique_ptr<Texture2D>      depthTex;
    std::unique_ptr<Texture2D>      varianceTex;
    std::unique_ptr<Texture2D>      blurredTex;
    std::unique_ptr<Program>        program;
    std::unique_ptr<GaussianBlur>   blur;
    Frustum                         frustum;
    OBB                             lightOBB;
    float3                          sphereCenter;
    float                           sphereRadius;
    std::unique_ptr<Buffer>         cameraUBO;
    uint                            fbWidth = 0;
    uint                            fbHeight = 0;
    BatchDrawCommands               drawCommands;

public:
    ShadowmapPass();
    ~ShadowmapPass();


    void updateRenderList(const Frustum& culling, const float2& depthRange);
    void execute(uint width, uint height);

    void debugDraw();

    const Texture2D* getDepthTex() const {return depthTex.get();}
    const Texture2D* getVarianceTex() const {return blurredTex.get(); }
    const Frustum& getFrustum() const {return frustum;}

private:
    void updateFrustum(const Frustum& culling, const float2& depthRange);
    void createFramebuffer(uint width, uint height);
    void createProgram();
    void updateCameraUBO();
    void render();

};