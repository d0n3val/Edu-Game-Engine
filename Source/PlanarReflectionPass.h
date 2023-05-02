#pragma once

#include <memory>

#include "ComponentCamera.h"

#include "Math.h"

class Framebuffer;
class Texture2D;
class ComponentCamera;
class GaussianBlur;

class PlanarReflectionPass
{
    typedef std::vector<std::unique_ptr<GaussianBlur> > GaussianChain;

    std::unique_ptr<Framebuffer> frameBuffer;
    std::unique_ptr<Texture2D> planarTex;
    std::unique_ptr<Texture2D> planarDepthTex;
    GaussianChain gaussian;

public:
    PlanarReflectionPass();
    ~PlanarReflectionPass();

    void execute(ComponentCamera* camera);
    void Bind();

    const Texture2D* getPlanarTex() const {return planarTex.get();}

private:

    void createFrameBuffer();
private:
    ComponentCamera planarCamera;

};