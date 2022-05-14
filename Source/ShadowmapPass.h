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
    std::unique_ptr<Buffer>      cameraUBO;

public:
    ShadowmapPass();
    ~ShadowmapPass();

    void execute(ComponentCamera* camera);

    const Texture2D* getDepth() const {return depthTex.get();}

private:
    void updateFrustum(ComponentCamera* camera);
    void createFramebuffer();
    void createProgram();
    void updateCameraUBO();

};