#include "CascadeShadowMap.h"

#include "OGL.h"
#include "OpenGL.h"

CascadeShadowMap::CascadeShadowMap()
{
}

CascadeShadowMap::~CascadeShadowMap()
{
}

void CascadeShadowMap::Update(ComponentCamera* camera)
{
    for(uint i=0; i<  CASCADE_COUNT; ++i)
    {
        UpdateCascade(i, camera);
        DrawCascade(i);
    }
}

void CascadeShadowMap::DrawCascade(uint index)
{
    cascades[i].frameBuffer->Bind();

    glViewport(0, 0, cascades[i].width, cascades[i].height);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    /* Draw
    App->programs->UseProgram("shadow", 0);

    glUniformMatrix4fv(App->programs->GetUniformLocation("proj"), 1, GL_TRUE, reinterpret_cast<const float*>(&cascades[i].proj));
    glUniformMatrix4fv(App->programs->GetUniformLocation("view"), 1, GL_TRUE, reinterpret_cast<const float*>(&cascades[i].view));

    DrawNodes(cascades[i].casters, &ModuleRenderer::DrawShadow);
    */
}

void CascadeShadowMap::UpdateCascade(uint index)
{
    // \TODO: Update cascade
}

void CascadeShadowMap::ResizeFrameBuffer(uint index)
{
    Cascade& cascade = cascades[index];

    if (!cascade.frameBuffer)
    {
        cascade.frameBuffer = std::make_unique<Framebuffer>();
    }

    cascade.frameBuffer->ClearAttachments();

    cascade.color = std::make_unique<Texture2D>(cascade.width, cascade.height, GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE, nullptr, false);
    cascade.depth = std::make_unique<Texture2D>(cascade.width, cascade.height, GL_DEPTH_COMPONENT32F, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr, false);

    cascade.frameBuffer->AttachColor(cascade.color.get(), 0, 0);
    cascade.frameBuffer->AttachDepthStencil(cascade.depth.get(), GL_DEPTH_ATTACHMENT);

    assert(cascade.frameBuffer->Check() == GL_FRAMEBUFFER_COMPLETE);
}