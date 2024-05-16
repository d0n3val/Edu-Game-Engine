#include "Globals.h"

#include "Application.h"
#include "ModuleRenderer.h"
#include "LightManager.h"
#include "ModuleHints.h"
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

void SpotShadowMapPass::execute(SpotLight* light, uint width, uint height)
{

    LightManager* lightManager = App->level->GetLightManager();

    if(lightManager->GetNumSpotLights() > 0)
    {
        glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, "SpotShadowMapPass");

        createProgram();

        generators.resize(lightManager->GetNumSpotLights());

        for (uint i = 0; i < lightManager->GetNumSpotLights(); ++i)
        {
            SpotLight *light = lightManager->GetSpotLight(i);

            if(light->GetEnabled() && light->GetCastShadows())
            {
                Generator &generator = generators[i];

                uint shadowSize = light->GetShadowSize();

                program->Use();
                glEnable(GL_DEPTH_TEST);
                glDepthFunc(GL_LESS);

                generator.createFramebuffer(shadowSize);
                generator.updateFrustum(light);
                generator.updateCameraUBO();

                generator.getCameraUBO()->BindToPoint(CAMERA_UBO_BINDING);
                generator.getFrameBuffer()->Bind();

                glViewport(0, 0, shadowSize, shadowSize);
                glClear(GL_DEPTH_BUFFER_BIT);

                App->renderer->GetBatchManager()->DoRender(generator.getObjects().GetOpaques(), 0);

                generator.blurTextures(shadowSize);

                light->SetShadowDepth(generator.getShadowDepth());
                light->SetShadowVariance(generator.getShadowVariance());
                light->SetShadowViewProj(generator.getFrustum().ViewProjMatrix());
            }
            else
            {
                light->SetShadowDepth(nullptr);
                light->SetShadowViewProj(float4x4::identity);
            }

        }

        glPopDebugGroup();
    }
}

void SpotShadowMapPass::Generator::createFramebuffer(uint size)
{
    if (size != fbSize)
    {
        if (!frameBuffer)
        {
            frameBuffer = std::make_unique<Framebuffer>();
        }

        frameBuffer->ClearAttachments();

        depthTex = std::make_unique<Texture2D>(size, size, GL_DEPTH_COMPONENT32, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr, false);
        varianceTex = std::make_unique<Texture2D>(size, size, GL_RG32F, GL_RG, GL_FLOAT, nullptr, false);
        blurredTex = std::make_unique<Texture2D>(size, size, GL_RG32F, GL_RG, GL_FLOAT, nullptr, false);

        frameBuffer->AttachColor(varianceTex.get(), 0, 0);
        frameBuffer->AttachDepthStencil(depthTex.get(), GL_DEPTH_ATTACHMENT);

        assert(frameBuffer->Check() == GL_FRAMEBUFFER_COMPLETE);

        fbSize = size;
    }
}

void SpotShadowMapPass::Generator::updateFrustum(const SpotLight* light)
{
    // NOTE: Angle for quat inside circle
    float radius = tanf(light->GetOutterCutoff()) * light->GetMaxDistance();
    float l = sqrtf(2.0)*radius;
    float halfL = l/2.0f;
    float angle = atan2f(halfL, light->GetMaxDistance());
    
    ///float innerAngle = atanf(radius*radius/light->GetDistance() * sqrtf(2.0f));
    frustum.type = FrustumType::PerspectiveFrustum;
    frustum.pos = light->GetTransform().TranslatePart();
    frustum.front = -light->GetTransform().Col3(1);
    frustum.up = light->GetTransform().Col3(2);
    frustum.nearPlaneDistance = light->GetMinDistance();
    frustum.farPlaneDistance = light->GetMaxDistance();
    //frustum.verticalFov = innerAngle * 2.0f;
    //frustum.horizontalFov = innerAngle * 2.0f;

    //frustum.verticalFov = angle * 2.0f;
    //frustum.horizontalFov = angle * 2.0f;

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

void SpotShadowMapPass::Generator::updateCameraUBO()
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

void SpotShadowMapPass::Generator::blurTextures(uint shadowSize)
{
    if (!blur)
    {
        blur = std::make_unique<GaussianBlur>();
    }

    blur->execute(varianceTex.get(), blurredTex.get(), GL_RG32F, GL_RG, GL_FLOAT, 0, shadowSize, shadowSize, 0, shadowSize, shadowSize);
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

