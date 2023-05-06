#pragma once

#include <memory>

#include "ComponentCamera.h"

#include "RenderList.h"
#include "Math.h"

class Framebuffer;
class Texture2D;
class ComponentCamera;
class Program;
class GaussianBlur;
class CameraUBO;

class PlanarReflectionPass
{
    typedef std::vector<std::unique_ptr<GaussianBlur> > GaussianChain;

    std::unique_ptr<Program>     program;
    std::unique_ptr<Framebuffer> frameBuffer;
    std::unique_ptr<Texture2D>   planarTex;
    std::unique_ptr<Texture2D>   planarDepthTex;
    std::unique_ptr<CameraUBO>   cameraUBO;
    RenderList                   objects;
    GaussianChain gaussian;

public:

    PlanarReflectionPass();
    ~PlanarReflectionPass();

    void execute();
    void Bind();

    void updateRenderList(ComponentCamera* camera);

    const RenderList& getRenderList() { return objects; }
    const Texture2D*  getPlanarTex() const {return planarTex.get();}

private:

    void createFrameBuffer();
    bool generateProgram();

private:
    ComponentCamera planarCamera;

};