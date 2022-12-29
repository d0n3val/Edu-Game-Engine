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
#include "DebugDraw.h"

#include "Leaks.h"

#include "../Game/Assets/Shaders/LocationsAndBindings.h"

#ifdef max 
#undef max
#endif 

#include <algorithm>


ShadowmapPass::ShadowmapPass()
{

}

ShadowmapPass::~ShadowmapPass()
{

}

void ShadowmapPass::execute(const Frustum& culling, uint width, uint height)
{
    createFramebuffer(width, height);
    createProgram();

    updateFrustum(culling);
    updateCameraUBO();
    updateRenderList();

    render();
}

void ShadowmapPass::debugDraw()
{
    float3 p[8];
    lightOBB.GetCornerPoints(p);
    std::swap(p[2], p[5]);
    std::swap(p[3], p[4]);
    std::swap(p[4], p[5]);
    std::swap(p[6], p[7]);
    dd::box(p, dd::colors::White);

    dd::sphere(sphereCenter, dd::colors::AntiqueWhite, sphereRadius);
}

void ShadowmapPass::updateFrustum(const Frustum& culling)
{
    float3 corner[8];

    culling.GetCornerPoints(corner);
    sphereCenter = culling.CenterPoint();

    sphereRadius = 0.0f;

    for(uint i=0; i< 8; ++i)
    {
        sphereRadius = std::max(sphereRadius, sphereCenter.Distance(corner[i]));
    }

    const DirLight* light = App->level->GetLightManager()->GetDirLight();
    float3 lightDir = light->GetDir();

    lightOBB.axis[0] = lightDir;
    lightOBB.axis[1] = light->GetUp();
    lightOBB.axis[2] = lightDir.Cross(light->GetUp());
    lightOBB.r = float3(sphereRadius);
    lightOBB.pos = sphereCenter;

    frustum.type               = FrustumType::OrthographicFrustum;
    frustum.pos                = sphereCenter-lightDir*sphereRadius;
    frustum.front              = lightDir;
    frustum.up                 = light->GetUp(); // TODO: simplify for students
    frustum.nearPlaneDistance  = 0.0f;
    frustum.farPlaneDistance   = sphereRadius*2.0f;
    frustum.orthographicWidth  = sphereRadius*2.0f;
    frustum.orthographicHeight = sphereRadius*2.0f;    
}

void ShadowmapPass::createFramebuffer(uint width, uint height)
{
    if (width != fbWidth || height != fbHeight)
    {
        if(!frameBuffer)
        {
            frameBuffer = std::make_unique<Framebuffer>();
        }

        frameBuffer->ClearAttachments();

        depthTex = std::make_unique<Texture2D>(width, height, GL_DEPTH_COMPONENT16_ARB, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr, false);

        depthTex->Bind(0);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
        depthTex->Unbind(0);

        frameBuffer->AttachDepthStencil(depthTex.get(), GL_DEPTH_ATTACHMENT);

        assert(frameBuffer->Check() == GL_FRAMEBUFFER_COMPLETE);

        fbWidth = width;
        fbHeight = height;
    }
}

void ShadowmapPass::createProgram()
{
    if(!program)
    {
        std::unique_ptr<Shader> vertex, fragment;

        const char *shadowMapMacros[] = {"#define SHADOW_MAP\n"};

        vertex.reset(Shader::CreateVSFromFile("assets/shaders/defVS.glsl", &shadowMapMacros[0], sizeof(shadowMapMacros)/sizeof(const char*)));

        bool ok = vertex->Compiled();

        if (ok)
        {
            fragment.reset(Shader::CreateFSFromFile("assets/shaders/shadowmap.glsl", &shadowMapMacros[0], sizeof(shadowMapMacros)/sizeof(const char*)));

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

void ShadowmapPass::updateRenderList()
{
    Plane obbPlanes[6];
    lightOBB.GetFacePlanes(obbPlanes);

    objects.UpdateFrom(obbPlanes, frustum.pos, App->level->GetRoot(), RENDERLIST_OBJ_OPAQUE);
}

void ShadowmapPass::render()
{
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    cameraUBO->BindToPoint(CAMERA_UBO_BINDING);
    program->Use();
    frameBuffer->Bind();
    glViewport(0, 0, fbWidth, fbHeight);
    glClear(GL_DEPTH_BUFFER_BIT);

    App->renderer->GetBatchManager()->Render(objects.GetOpaques(), BR_FLAG_AVOID_UPDATE_MODEL_MATRIX);
}