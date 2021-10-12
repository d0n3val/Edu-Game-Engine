#ifndef _CASCADE_SHADOW_MAP_H_
#define _CASCADE_SHADOW_MAP_H_

#include "Math.h"
#include <memory>

class Framebuffer;
class Texture2D;
class ComponentCamera;

class CascadeShadowMap
{
    struct Cascade
    {
        std::unique_ptr<Framebuffer> frameBuffer;
        std::unique_ptr<Texture2D>   color;
        std::unique_ptr<Texture2D>   depth;

        float4x4 viewProj;
        uint width = 0;
        uint height = 0;
    };

    enum { CASCADE_COUNT = 4 };

    Cascade cascades[CASCADE_COUNT];

public:

    CascadeShadowMap();
    ~CascadeShadowMap();

    void Update(ComponentCamera* camera);

private:

    void UpdateCascade(uint index, ComponentCamera* camera);
    void DrawCascade(uint index);
    void ResizeFrameBuffer(uint index);
};

#endif /* _CASCADE_SHADOW_MAP_H_ */


