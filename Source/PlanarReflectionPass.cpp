#include "Globals.h"

#include "PlanarReflectionPass.h"

#include "GaussianBlur.h"

#include "Application.h"
#include "ModuleRenderer.h"
#include "ModuleLevelManager.h"
#include "ModuleHints.h"
#include "BatchManager.h"
#include "IBLData.h"
#include "CameraUBO.h"
#include "LightManager.h"

#include "OGL.h"
#include "OpenGL.h"
#include "DebugDraw.h"

#include "../Game/Assets/Shaders/LocationsAndBindings.h"

#include<math.h>

#define DEFAULT_PLANAR_WIDTH 1024
#define DEFAULT_PLANAR_HEIGHT 512
#define DEFAULT_ROUGHNESS_LEVELS 7

PlanarReflectionPass::PlanarReflectionPass() : planarCamera(nullptr)
{
    cameraUBO = std::make_unique<CameraUBO>();
}

PlanarReflectionPass::~PlanarReflectionPass()
{
}

void PlanarReflectionPass::execute()
{
    if(std::get<bool>(App->hints->GetDHint(std::string("Planar reflection enabled"), true)) )
    {
        glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, "Planar Reflection");

        createFrameBuffer();
        generateProgram();

        program->Use();

        cameraUBO->Update(&planarCamera);
        cameraUBO->Bind();
        App->level->GetLightManager()->Bind();

        frameBuffer->Bind();
        glViewport(0, 0, DEFAULT_PLANAR_WIDTH, DEFAULT_PLANAR_HEIGHT);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        App->renderer->GetBatchManager()->DoRender(objects.GetOpaques(), 0);
        App->renderer->GetBatchManager()->DoRender(objects.GetTransparents(), BR_FLAG_KEEP_ORDER);

        App->level->GetSkyBox()->DrawEnvironment(planarCamera.GetProjectionMatrix(), planarCamera.GetViewMatrix());

        // Compute mip chain for roughness
        uint inWidth = DEFAULT_PLANAR_WIDTH;
        uint inHeight = DEFAULT_PLANAR_HEIGHT;
        for(uint i=0; i< DEFAULT_ROUGHNESS_LEVELS; ++i)
        {
            uint outWidth = inWidth >> 1;
            uint outHeight = inHeight >> 1;

            gaussian[i]->execute(planarTex.get(), planarTex.get(), GL_RGB8, GL_RGB, GL_UNSIGNED_INT, i, inWidth, inHeight, i + 1, outWidth, outHeight);

            inWidth = outWidth;
            inHeight = outHeight;
        }

        glPopDebugGroup();
    }
}

void PlanarReflectionPass::updateRenderList(ComponentCamera *camera)
{
    if(std::get<bool>(App->hints->GetDHint(std::string("Planar reflection enabled"), true)) )
    {
        float3 planePos = std::get<float3>(App->hints->GetDHint(std::string("Planar reflection plane pos"), float3::zero));
        float3 planeNormal = std::get<float3>(App->hints->GetDHint(std::string("Planar reflection plane normal"), float3::unitY));

        planarCamera.frustum = camera->frustum;

        float moveAmount = (planarCamera.frustum.pos - planePos).Dot(planeNormal) * 2.0f;
        planarCamera.frustum.pos = planarCamera.frustum.pos - planeNormal * moveAmount;

        // reflect front
        planarCamera.frustum.front.Normalize();
        float frontAmount = (planeNormal.Dot(planarCamera.frustum.front) * 2.0f);
        float3 newFront = planarCamera.frustum.front - planeNormal * frontAmount;
        newFront.Normalize();

        // reflect up
        float upAmount = (planeNormal.Dot(planarCamera.frustum.up) * 2.0f);
        float3 newUp = planarCamera.frustum.up - planeNormal * upAmount;
        newUp.Normalize();

        planarCamera.frustum.front = newFront;
        planarCamera.frustum.up = newUp;

        objects.UpdateFrom(planarCamera.frustum, App->level->GetRoot(), RENDERLIST_OBJ_ALL | RENDERLIST_OBJ_AVOID_PLANAR_REFLECTIONS);
    }
}

void PlanarReflectionPass::createFrameBuffer()
{
    if(!frameBuffer)
    {
		frameBuffer = std::make_unique<Framebuffer>();
        planarTex = std::make_unique<Texture2D>(DEFAULT_PLANAR_WIDTH, DEFAULT_PLANAR_HEIGHT, GL_RGB8, GL_RGB, GL_UNSIGNED_INT, nullptr, true);
        planarTex->SetTextureLodLevels(0, DEFAULT_ROUGHNESS_LEVELS-1);

        planarDepthTex = std::make_unique<Texture2D>(DEFAULT_PLANAR_WIDTH, DEFAULT_PLANAR_HEIGHT, GL_DEPTH_COMPONENT32F, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr, false);
        
        frameBuffer->AttachColor(planarTex.get(), 0, 0);
        frameBuffer->AttachDepthStencil(planarDepthTex.get(), GL_DEPTH_ATTACHMENT);
        assert(frameBuffer->Check() == GL_FRAMEBUFFER_COMPLETE);

        gaussian.reserve(DEFAULT_ROUGHNESS_LEVELS);
        for(uint i=0;i<DEFAULT_ROUGHNESS_LEVELS; ++i)
        {
            gaussian.push_back(std::make_unique<GaussianBlur>());
        }
    }
}

void PlanarReflectionPass::Bind()
{
    planarTex->Bind(PLANAR_REFLECTION_BINDING);
    
    float4x4 viewProj = planarCamera.frustum.ViewProjMatrix();
    glUniformMatrix4fv(PLANAR_REFLECTION_VIEWPROJ_LOCATION, 1, GL_TRUE, reinterpret_cast<float*>(&viewProj));
    glUniform1i(PLANAR_REFLECTION_LOD_LEVELS_LOCATION, int(DEFAULT_ROUGHNESS_LEVELS));

    float3 planeNormal = std::get<float3>(App->hints->GetDHint(std::string("Planar reflection plane normal"), float3::unitY));
    glUniform3fv(PLANAR_REFLECTION_NORMAL, 1, reinterpret_cast<float*>(&planeNormal));

    float distortion = std::get<float>(App->hints->GetDHint(std::string("Planar reflection distortion"), 0.0f));
    glUniform1f(PLANAR_REFLECTION_DISTORITION, distortion);
}

bool PlanarReflectionPass::generateProgram()
{
    bool ok = true;
    if(!program)
    {
        std::unique_ptr<Shader> vertex, fragment;

        vertex.reset(Shader::CreateVSFromFile("assets/shaders/defVS.glsl", 0, 0));

        ok = vertex->Compiled();

        if (ok)
        {
            const char* defines[] = {"#define DISABLE_SHADOW\n",  "#define DISABLE_SSAO\n"};
            fragment.reset(Shader::CreateFSFromFile("assets/shaders/defFS.glsl", defines, 2));

            ok = fragment->Compiled();
        }

        if (ok)
        {
            program = std::make_unique<Program>(vertex.get(), fragment.get(), "Planar Reflections");

            ok = program->Linked();
        }

        if (!ok)
        {
            program.release();
        }

    }

    return ok;
}
