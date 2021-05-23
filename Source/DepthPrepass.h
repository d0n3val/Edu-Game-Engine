#ifndef _DEPTH_PREPASS_H_
#define _DEPTH_PREPASS_H_

#include <memory>

class Framebuffer;
class Texture2D;
class DefaultShader;
class RenderList;

class DepthPrepass
{
    std::unique_ptr<Framebuffer> frameBuffer;
    std::unique_ptr<Texture2D>   depthTexture;
    std::unique_ptr<Texture2D>   posTexture;
    std::unique_ptr<Texture2D>   normalTexture;

    uint                         frameBufferWidth = 0;
    uint                         frameBufferHeight = 0;

public:

    DepthPrepass();

    void Execute(DefaultShader* shader, const RenderList& nodes, uint width, uint height);

    const Texture2D* getDepthTexture() const { return depthTexture.get(); }
    const Texture2D* getPositionTexture() const { return posTexture.get(); }
    const Texture2D* getNormalTexture() const { return normalTexture.get(); }
    uint             getWidth() const {return frameBufferWidth; }
    uint             getHeight() const {return frameBufferHeight; }

private:

    void ResizeFrameBuffer(uint width, uint height);
};

#endif _DEPTH_PREPASS_H_
