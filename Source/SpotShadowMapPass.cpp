#include "Globals.h"

#include "Application.h"
#include "ModuleRenderer.h"
#include "ModuleLevelManager.h"
#include "BatchManager.h"
#include "SpotShadowMapPass.h"
#include "SpotLight.h"
#include "DebugDraw.h"
#include "OpenGL.h"
#include "../Game/Assets/Shaders/LocationsAndBindings.h"

SpotShadowMapPass::SpotShadowMapPass()
{

}

SpotShadowMapPass::~SpotShadowMapPass()
{

}

void SpotShadowMapPass::updateRenderList()
{

}

void SpotShadowMapPass::execute(const SpotLight* light, uint width, uint height)
{
    glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, "SpotShadowMapPass");

    createFramebuffer(width, height);
    createProgram();
    updateFrustum(light);
    updateCameraUBO();

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    cameraUBO->BindToPoint(CAMERA_UBO_BINDING);
    program->Use();

    frameBuffer->Bind();
    glViewport(0, 0, width, height);
    glClear(GL_DEPTH_BUFFER_BIT);

    App->renderer->GetBatchManager()->DoRender(objects.GetOpaques(), 0);

    glPopDebugGroup();
}

void SpotShadowMapPass::createFramebuffer(uint width, uint height)
{
    if (width != fbWidth || height != fbHeight)
    {
        if (!frameBuffer)
        {
            frameBuffer = std::make_unique<Framebuffer>();
        }

        frameBuffer->ClearAttachments();

        depthTex = std::make_unique<Texture2D>(width, height, GL_DEPTH_COMPONENT32, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr, false);
        frameBuffer->AttachDepthStencil(depthTex.get(), GL_DEPTH_ATTACHMENT);

        assert(frameBuffer->Check() == GL_FRAMEBUFFER_COMPLETE);

        fbWidth = width;
        fbHeight = height;
    }
}

void SpotShadowMapPass::updateFrustum(const SpotLight* light)
{
    frustum.type = FrustumType::PerspectiveFrustum;
    frustum.pos = light->GetTransform().TranslatePart();
    frustum.front = -light->GetTransform().Col3(1);
    frustum.up = light->GetTransform().Col3(2);
    frustum.nearPlaneDistance = 0.1f;
    frustum.farPlaneDistance = light->GetDistance();
    frustum.verticalFov = light->GetOutterCutoff() * 2.0f;
    frustum.horizontalFov = light->GetOutterCutoff() * 2.0f;

    /*
    float4x4 matrix = frustum.ProjectionMatrix() * frustum.ViewMatrix();
    matrix.Inverse();
    dd::frustum(matrix, dd::colors::Yellow);
    dd::line(frustum.pos, frustum.pos + frustum.front * frustum.farPlaneDistance, dd::colors::Red);
    */

    objects.UpdateFrom(frustum, App->level->GetRoot(), RENDERLIST_OBJ_OPAQUE);
}

void SpotShadowMapPass::updateCameraUBO()
{
    struct CameraData
    {
        float4x4 proj = float4x4::identity;
        float4x4 view = float4x4::identity;
        float4   view_pos = float4::zero;
    } cameraData;

    if (!cameraUBO)
    {
        cameraUBO.reset(new Buffer(GL_UNIFORM_BUFFER, GL_DYNAMIC_DRAW, sizeof(cameraData), nullptr));
    }

    cameraData.proj = frustum.ProjectionMatrix();
    cameraData.view = frustum.ViewMatrix();
    cameraData.view_pos = float4(cameraData.view.RotatePart().Transposed().Transform(-cameraData.view.TranslatePart()), 1.0);

    cameraUBO->InvalidateData();
    cameraUBO->SetData(0, sizeof(CameraData), &cameraData);
}

void SpotShadowMapPass::createProgram()
{
    if (!program)
    {
        std::unique_ptr<Shader> vertex, fragment;

        const char* shadowMapMacros[] = { "#define SHADOW_MAP\n" };

        vertex.reset(Shader::CreateVSFromFile("assets/shaders/defVS.glsl", &shadowMapMacros[0], sizeof(shadowMapMacros) / sizeof(const char*)));

        bool ok = vertex->Compiled();

        if (ok)
        {
            fragment.reset(Shader::CreateFSFromFile("assets/shaders/shadowmap.glsl", &shadowMapMacros[0], sizeof(shadowMapMacros) / sizeof(const char*)));

            ok = fragment->Compiled();
        }

        if (ok)
        {
            program = std::make_unique<Program>(vertex.get(), fragment.get(), "ShadowMap");

            ok = program->Linked();
        }

        if (!ok)
        {
            program.release();
        }
    }
}


