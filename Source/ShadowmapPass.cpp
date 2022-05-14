#include "Globals.h"

#include "ShadowmapPass.h"
#include "Application.h"
#include "ModuleRenderer.h"
#include "ModuleLevelManager.h"
#include "LightManager.h"
#include "DirLight.h"
#include "BatchManager.h"

#include "ComponentCamera.h"

#include "OGL.h"
#include "OpenGL.h"

#include "Leaks.h"

#include "../Game/Assets/Shaders/LocationsAndBindings.h"

#ifdef max 
#undef max
#endif 

#include <algorithm>

#define SHADOW_MAP_TEXTURE_SIZE 512

ShadowmapPass::ShadowmapPass()
{

}

ShadowmapPass::~ShadowmapPass()
{

}

void ShadowmapPass::execute(ComponentCamera* camera)
{
    createFramebuffer();
    createProgram();

    updateFrustum(camera);
    updateCameraUBO();

    objects.UpdateFrom(frustum, App->level->GetRoot(), RENDERLIST_OBJ_OPAQUE);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    cameraUBO->BindToPoint(CAMERA_UBO_BINDING);
    program->Use();
    frameBuffer->Bind();
    glViewport(0, 0, SHADOW_MAP_TEXTURE_SIZE, SHADOW_MAP_TEXTURE_SIZE);
    glClear(GL_DEPTH_BUFFER_BIT);

    App->renderer->GetBatchManager()->Render(objects.GetOpaques(), BR_FLAG_AVOID_UPDATE_MODEL_MATRIX);
}

void ShadowmapPass::updateFrustum(ComponentCamera* camera)
{
    float3 corner[8];

    camera->frustum.GetCornerPoints(corner);
    float3 center = camera->frustum.CenterPoint();

    float radius = 0.0f;

    for(uint i=0; i< 8; ++i)
    {
        radius = std::max(radius, center.Distance(corner[i]));
    }

    const DirLight* light = App->level->GetLightManager()->GetDirLight();
    float3 lightDir = light->GetDir();

    frustum.type  = FrustumType::OrthographicFrustum;
    frustum.pos   = center-lightDir*radius;
    frustum.front = lightDir;
    frustum.up    = light->GetUp(); // TODO: simplify for students
    frustum.nearPlaneDistance  = 0.0f;
    frustum.farPlaneDistance   = radius*2.0f;
    frustum.orthographicWidth  = radius*2.0f;
    frustum.orthographicHeight = radius*2.0f;
}

void ShadowmapPass::createFramebuffer()
{
    if (!frameBuffer)
    {
        frameBuffer = std::make_unique<Framebuffer>();
        depthTex = std::make_unique<Texture2D>(SHADOW_MAP_TEXTURE_SIZE, SHADOW_MAP_TEXTURE_SIZE, GL_DEPTH_COMPONENT32F, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr, false);

        frameBuffer->AttachDepthStencil(depthTex.get(), GL_DEPTH_ATTACHMENT);

        assert(frameBuffer->Check() == GL_FRAMEBUFFER_COMPLETE);
    }
}

void ShadowmapPass::createProgram()
{
    if(!program)
    {
        std::unique_ptr<Shader> vertex, fragment;

        vertex.reset(Shader::CreateVSFromFile("assets/shaders/defVS.glsl", 0, 0));

        bool ok = vertex->Compiled();

        if (ok)
        {
            fragment.reset(Shader::CreateFSFromFile("assets/shaders/shadowmap.glsl", 0, 0));

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

void ShadowmapPass::updateCameraUBO()
{
    struct CameraData
    {
        float4x4 proj     = float4x4::identity;
        float4x4 view     = float4x4::identity;
        float4   view_pos = float4::zero;
    } cameraData;

    if(!cameraUBO)
    {
        cameraUBO.reset(new Buffer(GL_UNIFORM_BUFFER, GL_DYNAMIC_DRAW, sizeof(cameraData), nullptr));
    }

    cameraData.proj     = frustum.ProjectionMatrix();
    cameraData.view     = frustum.ViewMatrix();
    cameraData.view_pos = float4(cameraData.view.RotatePart().Transposed().Transform(-cameraData.view.TranslatePart()), 1.0);

    cameraUBO->InvalidateData();
    cameraUBO->SetData(0, sizeof(CameraData), &cameraData);
}