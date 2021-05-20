#include "Globals.h"

#include "DepthPrepass.h"
#include "DefaultShader.h"

#include "OpenGL.h"
#include "OGL.h"

DepthPrepass::DepthPrepass()
{ç
    frameBuffer = std::move(std::make_unique<Framebuffer>());
}

void DepthPrepass::Execute(DefaultShader* shader, const RenderList& nodes, uint width, uint height)
{
    ResizeFrameBuffer(width, height);

    frameBuffer->ClearColor(0, {0.0f, 0.0f, 0.0f, 0.0f});
    frameBuffer->ClearColor(1, {0.0f, 0.0f, 0.0f, 0.0f});
    frameBuffer->ClearDepth(1.0f);
    frameBuffer->Bind();
    glViewport(0, 0, width, height);

    for(const TRenderInfo& lInfo : nodes.GetOpaques())
    {
        if(render_info.mesh)
        {
            shader->DepthPrepass(render_info.mesh);
        }
    }

    frameBuffer->Unbind();
}

void DepthPrepass::ResizeFrameBuffer(uint width, uint height)
{
    if(width != frameBufferWidth || height != frameBufferHeight)
    {ç
        frameBuffer->ClearAttachments();

        depthTexture  = std::move(std::make_unique<Texture2D>(width, height, GL_DEPTH_COMPONENT32, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr, false));
        posTexture    = std::move(std::make_unique<Texture2D>(width, height, GL_RGB32F, GL_RGB, GL_FLOAT, nullptr, false));
        normalTexture = std::move(std::make_unique<Texture2D>(width, height, GL_RGB32, GL_RGB, GL_UNSIGNED_INT, nullptr, false));

        frameBuffer->AttachColor(posTexture.get(), 0, 0); 
        frameBuffer->AttachColor(normalTexture.get(), 1, 0); 
        frameBuffer->AttachDepthStencil(depthTexture.get());

        frameBufferWidth = width;
        frameBufferHeight = height;
    }
}
