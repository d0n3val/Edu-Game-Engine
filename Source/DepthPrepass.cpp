#include "Globals.h"

#include "DepthPrepass.h"
#include "DefaultShader.h"
#include "RenderList.h"

#include "OpenGL.h"
#include "OGL.h"

DepthPrepass::DepthPrepass()
{
}

void DepthPrepass::Execute(DefaultShader* shader, const RenderList& nodes, uint width, uint height)
{
    ResizeFrameBuffer(width, height);

    float clearColor[] = { 0.0f, 0.0f, 0.0f, 1.0f };
    frameBuffer->ClearColor(0, clearColor);
    frameBuffer->ClearColor(1, clearColor);
    frameBuffer->ClearDepth(1.0f);

    frameBuffer->Bind();
    glViewport(0, 0, width, height);

    for(const TRenderInfo& info : nodes.GetOpaques())
    {
        if(info.mesh)
        {
            shader->DepthPrePass(info.mesh);
        }
    }

    frameBuffer->Unbind();
}

void DepthPrepass::ResizeFrameBuffer(uint width, uint height)
{
    if(width != frameBufferWidth || height != frameBufferHeight)
    {
        if (!frameBuffer)
        {
            frameBuffer = std::make_unique<Framebuffer>();
        }

        frameBuffer->ClearAttachments();

        depthTexture  = std::make_unique<Texture2D>(width, height, GL_DEPTH_COMPONENT32F, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr, false);
        posTexture    = std::make_unique<Texture2D>(width, height, GL_RGB32F, GL_RGB, GL_FLOAT, nullptr, false);
        normalTexture = std::make_unique<Texture2D>(width, height, GL_RGB8, GL_RGB, GL_UNSIGNED_INT, nullptr, false);
       
        frameBuffer->AttachColor(posTexture.get(), 0, 0); 
        frameBuffer->AttachColor(normalTexture.get(), 1, 0); 
        frameBuffer->AttachDepthStencil(depthTexture.get(), GL_DEPTH_ATTACHMENT);
        assert(frameBuffer->Check() == GL_FRAMEBUFFER_COMPLETE);

        frameBufferWidth = width;
        frameBufferHeight = height;
    }
}
