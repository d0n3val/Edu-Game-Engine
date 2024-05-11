#pragma once

#include "RenderList.h"
#include <memory>
#include "OGL.h"

class SpotLight;

class SpotShadowMapPass
{
    std::unique_ptr<Framebuffer>  frameBuffer;
    std::unique_ptr<Texture2D>    depthTex;
    std::unique_ptr<Program>      program;
    RenderList                    objects;
    Frustum                       frustum;
    std::unique_ptr<Buffer>       cameraUBO;
    uint                          fbWidth = 0;
    uint                          fbHeight = 0;


public:
    SpotShadowMapPass();
    ~SpotShadowMapPass();

    void updateRenderList();
    void execute(const SpotLight* light, uint width, uint height);

private:
    void createFramebuffer(uint width, uint height);
    void updateCameraUBO();
    void updateFrustum(const SpotLight* light);
    void createProgram();

};