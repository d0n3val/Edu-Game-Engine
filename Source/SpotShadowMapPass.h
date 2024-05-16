#pragma once

#include "RenderList.h"
#include <memory>
#include "OGL.h"
#include "GaussianBlur.h"

class SpotLight;

class SpotShadowMapPass
{
    std::unique_ptr<Program> program;

    class Generator
    {
        std::unique_ptr<Framebuffer> frameBuffer;
        std::unique_ptr<Texture2D> depthTex;
        std::unique_ptr<Texture2D> varianceTex;
        std::unique_ptr<Texture2D> blurredTex;
        std::unique_ptr<Buffer>    cameraUBO;
        std::unique_ptr<GaussianBlur> blur;
        RenderList objects;
        Frustum frustum;
        uint fbSize = 0;

    public:
        Generator() {}        
        Generator(Generator && other)
        {
            frameBuffer.swap(other.frameBuffer);
            depthTex.swap(other.depthTex);
            varianceTex.swap(other.varianceTex);
            blurredTex.swap(other.blurredTex);
            cameraUBO.swap(other.cameraUBO);
        }

        void createFramebuffer(uint size);
        void updateCameraUBO();
        void updateFrustum(const SpotLight *light);
        void blurTextures(uint shadowSize);

        const Framebuffer* getFrameBuffer() const { return frameBuffer.get();}
        const Buffer* getCameraUBO() const { return cameraUBO.get();}
        const RenderList& getObjects() const { return objects; }

        const Texture2D* getShadowDepth() const { return depthTex.get();}
        const Texture2D* getShadowVariance() const { return blurredTex.get();}
        const Frustum& getFrustum() { return frustum; }
    };

    std::vector<Generator> generators;


public:
    SpotShadowMapPass();
    ~SpotShadowMapPass();

    void updateRenderList();
    void execute(SpotLight* light, uint width, uint height);

private:
    void createProgram();

};