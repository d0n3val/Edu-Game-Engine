#ifndef _DEPTH_PREPASS_H_
#define _DEPTH_PREPASS_H_

#include <memory>

class Framebuffer;
class Texture2D;
class DefaultShader;
class RenderList;
class Program;

class DepthPrepass
{
    std::unique_ptr<Framebuffer> frameBuffer;
    std::unique_ptr<Framebuffer> resolveFB;

    std::unique_ptr<Texture2D>   depthTexture;
    std::unique_ptr<Texture2D>   posTextureMS;
    std::unique_ptr<Texture2D>   normalTextureMS;

    std::unique_ptr<Texture2D>   posTexture;
    std::unique_ptr<Texture2D>   normalTexture;

    std::unique_ptr<Program>     resolveProgram;

    uint                         frameBufferWidth = 0;
    uint                         frameBufferHeight = 0;
    bool                         usedMSAA = false;

public:

    DepthPrepass();

    void Execute(DefaultShader* shader, const RenderList& nodes, uint width, uint height);

    const Texture2D* getDepthTexture() const { return depthTexture.get(); }

    const Texture2D* getPositionTexture() const { return posTexture.get(); }
    const Texture2D* getNormalTexture() const { return normalTexture.get(); }

    uint             getWidth() const {return frameBufferWidth; }
    uint             getHeight() const {return frameBufferHeight; }

private:

    void ResizeFrameBuffer(uint width, uint height, bool msaa);
};

#endif _DEPTH_PREPASS_H_
