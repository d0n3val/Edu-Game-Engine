#pragma once

#include <memory>

#include "RenderList.h"
#include "Math.h"

#include "ShadowmapPass.h"

class CascadeShadowPass
{
public:
    enum { CASCADE_COUNT = 3};

private:
    ShadowmapPass cascades[CASCADE_COUNT];

public:

    CascadeShadowPass();
    ~CascadeShadowPass();

    void updateRenderList(const Frustum& culling);
    void execute();
    void debugDraw();

    const Texture2D* getDepthTex(uint index) const {return cascades[index].getDepthTex();}
    const Frustum& getFrustum(uint index) const {return cascades[index].getFrustum();}



};