#include "Globals.h"

#include "DepthPrepass.h"
#include "DefaultShader.h"
#include "RenderList.h"
#include "Application.h"
#include "ModuleHints.h"

#include "OpenGL.h"
#include "OGL.h"

DepthPrepass::DepthPrepass()
{
    std::unique_ptr<Shader> fullScreenVS(Shader::CreateVSFromFile("assets/shaders/fullscreenVS.glsl"));
    std::unique_ptr<Shader> resolveFS(Shader::CreateFSFromFile("assets/shaders/resolveSSAOMS.glsl"));

    if(fullScreenVS->Compiled() && resolveFS->Compiled())
    {
        resolveProgram = std::make_unique<Program>(fullScreenVS.get(), resolveFS.get(), "resolve SSAO MS program");
    }

    assert(resolveProgram->Linked());

}

void DepthPrepass::Execute(DefaultShader* shader, const RenderList& nodes, uint width, uint height)
{
    bool msaa = App->hints->GetBoolValue(ModuleHints::ENABLE_MSAA);

    ResizeFrameBuffer(width, height, msaa);

    glViewport(0, 0, width, height);

    float clearColor[] = { 0.0f, 0.0f, 0.0f, 1.0f };
    frameBuffer->ClearColor(0, clearColor);
    frameBuffer->ClearColor(1, clearColor);
    frameBuffer->ClearDepth(1.0f);

    frameBuffer->Bind();
    
    for(const TRenderInfo& info : nodes.GetOpaques())
    {
        if(info.mesh)
        {
            shader->DepthPrePass(info.mesh);
        }
    }

    frameBuffer->Unbind();

    if(msaa)
    {
        resolveFB->Bind();
        resolveProgram->Use();
        resolveProgram->BindTextureFromName("depths", 0, depthTexture.get());
        resolveProgram->BindTextureFromName("positions", 1, posTextureMS.get());
        resolveProgram->BindTextureFromName("normals", 2, normalTextureMS.get());

        glDrawArrays(GL_TRIANGLES, 0, 3);
    }
}

void DepthPrepass::ResizeFrameBuffer(uint width, uint height, bool msaa)
{
    if(width != frameBufferWidth || height != frameBufferHeight || usedMSAA != msaa)
    {
        if (!frameBuffer)
        {
            frameBuffer = std::make_unique<Framebuffer>();
        }

        if(msaa)
        {
            frameBuffer->ClearAttachments();

            depthTexture = std::make_unique<Texture2D>(4, width, height, GL_DEPTH_COMPONENT32F, true);
            posTextureMS = std::make_unique<Texture2D>(4, width, height, GL_RGB32F, true);
            normalTextureMS = std::make_unique<Texture2D>(4, width, height, GL_RGB32F, true);

            depthTexture->SetWrapping(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
            posTextureMS->SetWrapping(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
            normalTextureMS->SetWrapping(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);

            frameBuffer->AttachColor(posTextureMS.get(), 0, 0);
            frameBuffer->AttachColor(normalTextureMS.get(), 1, 0);
            frameBuffer->AttachDepthStencil(depthTexture.get(), GL_DEPTH_ATTACHMENT);
            assert(frameBuffer->Check() == GL_FRAMEBUFFER_COMPLETE);

            if (!resolveFB)
            {
                resolveFB = std::make_unique<Framebuffer>();
            }

            resolveFB->ClearAttachments();

            posTexture    = std::make_unique<Texture2D>(width, height, GL_RGB32F, GL_RGB, GL_FLOAT, nullptr, false);
            normalTexture = std::make_unique<Texture2D>(width, height, GL_RGB32F, GL_RGB, GL_FLOAT, nullptr, false);
            posTexture->SetWrapping(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
            normalTexture->SetWrapping(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
            resolveFB->AttachColor(posTexture.get(), 0, 0);
            resolveFB->AttachColor(normalTexture.get(), 1, 0);

            assert(resolveFB->Check() == GL_FRAMEBUFFER_COMPLETE);
        }
        else
        {
            frameBuffer->ClearAttachments();

            depthTexture = std::make_unique<Texture2D>(width, height, GL_DEPTH_COMPONENT32F, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr, false);
            posTexture = std::make_unique<Texture2D>(width, height, GL_RGB32F, GL_RGB, GL_FLOAT, nullptr, false);
            normalTexture = std::make_unique<Texture2D>(width, height, GL_RGB32F, GL_RGB, GL_FLOAT, nullptr, false);

            depthTexture->SetWrapping(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
            posTexture->SetWrapping(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
            normalTexture->SetWrapping(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);

            frameBuffer->AttachColor(posTexture.get(), 0, 0);
            frameBuffer->AttachColor(normalTexture.get(), 1, 0);
            frameBuffer->AttachDepthStencil(depthTexture.get(), GL_DEPTH_ATTACHMENT);
            assert(frameBuffer->Check() == GL_FRAMEBUFFER_COMPLETE);
        }
        
        frameBufferWidth = width;
        frameBufferHeight = height;
        usedMSAA = msaa;
    }
}
