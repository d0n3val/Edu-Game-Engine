#include "Globals.h"
#include "ModuleRenderer.h"
#include "ModulePrograms.h"
#include "ModuleLevelManager.h"
#include "ModuleResources.h"
#include "ModuleHints.h"
#include "ModuleEditor.h"

#include "DefaultShaderLocations.h"
#include "PostprocessShaderLocations.h"

#include "GameObject.h"

#include "ComponentMeshRenderer.h"

#include "ComponentCamera.h"
#include "ComponentAnimation.h"
#include "ComponentParticleSystem.h"
#include "ComponentTrail.h"
#include "BatchManager.h"
#include "Postprocess.h"

#include "ResourceMesh.h"
#include "ResourceMaterial.h"
#include "ResourceTexture.h"

#include "AmbientLight.h"
#include "DirLight.h"
#include "PointLight.h"
#include "SpotLight.h"
#include "Skybox.h"

#include "Application.h"

#include "OpenGL.h"
#include "DebugDraw.h"

#include "imgui/imgui.h"

#include "SOIL2.h"

#include <string>
#include <functional>
#include <algorithm>

#include "Leaks.h"

ModuleRenderer::ModuleRenderer() : Module("renderer")
{
    batch_manager = std::make_unique<BatchManager>();
    postProcess = std::make_unique<Postprocess>();
}

bool ModuleRenderer::Init(Config* config /*= nullptr*/)
{
    LoadDefaultShaders();
    postProcess->Init();

    cameraUBO.reset(new Buffer(GL_UNIFORM_BUFFER, GL_DYNAMIC_DRAW, sizeof(cameraData), nullptr));

    for(uint i=0; i< CASCADE_COUNT; ++i)
    {
        glGenFramebuffers(1, &cascades[i].fbo);
        glGenFramebuffers(1, &cascades[i].sq_fbo);
        glGenFramebuffers(1, &cascades[i].blur_fbo_0);
        glGenFramebuffers(1, &cascades[i].blur_fbo_1);
    }

	return true;
}

ModuleRenderer::~ModuleRenderer()
{
    for(uint i=0; i< CASCADE_COUNT; ++i)
    {
        if(cascades[i].fbo != 0)
        {
            glDeleteFramebuffers(1, &cascades[i].fbo);
        }

        if(cascades[i].tex != 0)
        {
            glDeleteTextures(1, &cascades[i].tex);
        }

        if(cascades[i].sq_fbo != 0)
        {
            glDeleteFramebuffers(1, &cascades[i].sq_fbo);
        }

        if(cascades[i].sq_tex != 0)
        {
            glDeleteTextures(1, &cascades[i].sq_tex);
        }

        if(cascades[i].blur_fbo_0 != 0)
        {
            glDeleteFramebuffers(1, &cascades[i].blur_fbo_0);
        }

        if(cascades[i].blur_tex_0 != 0)
        {
            glDeleteTextures(1, &cascades[i].blur_tex_0);
        }

        if(cascades[i].blur_fbo_1 != 0)
        {
            glDeleteFramebuffers(1, &cascades[i].blur_fbo_1);
        }

        if(cascades[i].blur_tex_1 != 0)
        {
            glDeleteTextures(1, &cascades[i].blur_tex_1);
        }
    }

}

void ModuleRenderer::Draw(ComponentCamera* camera, ComponentCamera* culling, unsigned fbo, unsigned width, unsigned height)
{
	cameraData.proj   = camera->GetProjectionMatrix();	
	cameraData.view   = camera->GetViewMatrix();

    cameraUBO->SetData(0, sizeof(CameraData), &cameraData);
    cameraUBO->BindToTargetIdx(0);

    float3 view_pos = cameraData.view.RotatePart().Transposed().Transform(-cameraData.view.TranslatePart());

    render_list.UpdateFrom(culling, App->level->GetRoot()); // App->level->quadtree.root);

    if(App->hints->GetBoolValue(ModuleHints::ENABLE_SHADOW_MAPPING))
    {
        ShadowPass(camera, width, height);
    }

    ColorPass(cameraData.proj, cameraData.view, view_pos, fbo, width, height);
}

void ModuleRenderer::DrawForSelection(ComponentCamera* camera)
{
	float4x4 proj   = camera->GetProjectionMatrix();	
	float4x4 view   = camera->GetViewMatrix();

    render_list.UpdateFrom(camera, App->level->GetRoot());

    SelectionPass(proj, view);
}

void ModuleRenderer::ShadowPass(ComponentCamera* camera, unsigned width, unsigned height)
{
    static const uint cascasdes_res_cte[] = {ModuleHints::SHADOW_CASCADE_0_RES, ModuleHints::SHADOW_CASCADE_1_RES, ModuleHints::SHADOW_CASCADE_2_RES };

    float2 shadow_res[3] =  { App->hints->GetFloat2Value(ModuleHints::SHADOW_CASCADE_0_RES), 
                              App->hints->GetFloat2Value(ModuleHints::SHADOW_CASCADE_1_RES), 
                              App->hints->GetFloat2Value(ModuleHints::SHADOW_CASCADE_2_RES)};

    int periods[] = { App->hints->GetIntValue(ModuleHints::SHADOW_CASCADE_0_UPDATE),  
                      App->hints->GetIntValue(ModuleHints::SHADOW_CASCADE_1_UPDATE), 
                      App->hints->GetIntValue(ModuleHints::SHADOW_CASCADE_2_UPDATE) };

    for(uint i=0; i<  CASCADE_COUNT; ++i)
    {
        cascades[i].tick = ((cascades[i].tick+1)%periods[i]);

        if(cascades[i].tick == 0)
        {
            if (App->hints->GetBoolValue(ModuleHints::UPDATE_SHADOW_VOLUME))
            {
                ComputeDirLightShadowVolume(camera, i);
            }

            GenerateShadowFBO(cascades[i], int(shadow_res[i].x), int(shadow_res[i].y));

            glBindFramebuffer(GL_FRAMEBUFFER, cascades[i].fbo);

            glViewport(0, 0, int(shadow_res[i].x), int(shadow_res[i].y));
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


            App->programs->UseProgram("shadow", 0);

            //float4x4 camera_proj   = camera->GetProjectionMatrix();	
            //float4x4 camera_view   = camera->GetViewMatrix();

            glUniformMatrix4fv(App->programs->GetUniformLocation("proj"), 1, GL_TRUE, reinterpret_cast<const float*>(&cascades[i].proj));
            glUniformMatrix4fv(App->programs->GetUniformLocation("view"), 1, GL_TRUE, reinterpret_cast<const float*>(&cascades[i].view));
            //glUniformMatrix4fv(App->programs->GetUniformLocation("camera_proj"), 1, GL_TRUE, reinterpret_cast<const float*>(&camera_proj));
            //glUniformMatrix4fv(App->programs->GetUniformLocation("camera_view"), 1, GL_TRUE, reinterpret_cast<const float*>(&camera_view));

            if(App->hints->GetBoolValue(ModuleHints::ENABLE_SHADOW_FRONT_CULLING))
            {
                glCullFace(GL_FRONT);
            }

            DrawNodes(cascades[i].casters, &ModuleRenderer::DrawShadow);

            if (App->hints->GetBoolValue(ModuleHints::ENABLE_SHADOW_FRONT_CULLING))
            {
                glCullFace(GL_BACK);
            }

            if (App->hints->GetBoolValue(ModuleHints::SHADOW_ENABLE_SOFT))
            {
                BlurShadow(i);
            }
        }

        if (App->hints->GetBoolValue(ModuleHints::SHOW_SHADOW_CLIPPING))
        {
            math::float3 p[8];
            cascades[i].world_bb.GetCornerPoints(p);
            std::swap(p[2], p[5]);
            std::swap(p[3], p[4]);
            std::swap(p[4], p[5]);
            std::swap(p[6], p[7]);
            dd::box(p, i == 0 ? dd::colors::Red : (i == 1 ? dd::colors::Green : dd::colors::Blue));

            cascades[i].frustum.GetCornerPoints(p);
            std::swap(p[2], p[5]);
            std::swap(p[3], p[4]);
            std::swap(p[4], p[5]);
            std::swap(p[6], p[7]);
            dd::box(p, dd::colors::White);
        }
    }
}

void ModuleRenderer::ColorPass(const float4x4& proj, const float4x4& view, const float3& view_pos, unsigned fbo, unsigned width, unsigned height)
{
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glViewport(0, 0, width, height);

    App->level->GetSkyBox()->Draw(proj, view);

    uint flags = 0;

    if (App->hints->GetBoolValue(ModuleHints::ENABLE_SHADOW_MAPPING))
    {
        flags |= (1 << 0);

        if (App->hints->GetBoolValue(ModuleHints::SHADOW_SHOW_CASCADES))
        {
            flags |= (1 << 1);
        }

        if(App->hints->GetBoolValue(ModuleHints::SHADOW_ENABLE_SOFT))
        {
            flags |= (1 << 2);
        }
    }

    // Set camera uniforms shared for all
    App->programs->UseProgram("default", flags);

    App->programs->BindUniformBlock("default", flags, "camera", 0);

    glUniform3f(App->programs->GetUniformLocation("view_pos"), view_pos.x, view_pos.y, view_pos.z);

    if(App->hints->GetBoolValue(ModuleHints::ENABLE_SHADOW_MAPPING))
    {
        float2 shadow_res[3] = { App->hints->GetFloat2Value(ModuleHints::SHADOW_CASCADE_0_RES),
                          App->hints->GetFloat2Value(ModuleHints::SHADOW_CASCADE_1_RES),
                          App->hints->GetFloat2Value(ModuleHints::SHADOW_CASCADE_2_RES) };


        glUniformMatrix4fv(App->programs->GetUniformLocation("light_proj[0]"), 1, GL_TRUE, reinterpret_cast<const float*>(&cascades[0].proj));
        glUniformMatrix4fv(App->programs->GetUniformLocation("light_proj[1]"), 1, GL_TRUE, reinterpret_cast<const float*>(&cascades[1].proj));
        glUniformMatrix4fv(App->programs->GetUniformLocation("light_proj[2]"), 1, GL_TRUE, reinterpret_cast<const float*>(&cascades[2].proj));
        glUniformMatrix4fv(App->programs->GetUniformLocation("light_view[0]"), 1, GL_TRUE, reinterpret_cast<const float*>(&cascades[0].view));
        glUniformMatrix4fv(App->programs->GetUniformLocation("light_view[1]"), 1, GL_TRUE, reinterpret_cast<const float*>(&cascades[1].view));
        glUniformMatrix4fv(App->programs->GetUniformLocation("light_view[2]"), 1, GL_TRUE, reinterpret_cast<const float*>(&cascades[2].view));

        glUniform2fv(App->programs->GetUniformLocation("map_size[0]"), 1, &shadow_res[0].x);
        glUniform2fv(App->programs->GetUniformLocation("map_size[1]"), 1, &shadow_res[1].x);
        glUniform2fv(App->programs->GetUniformLocation("map_size[2]"), 1, &shadow_res[2].x);

        glUniform1f(App->programs->GetUniformLocation("shadow_bias"), App->hints->GetFloatValue(ModuleHints::SHADOW_BIAS));
        glUniform1i(App->programs->GetUniformLocation("kernel_half_size"), App->hints->GetIntValue(ModuleHints::SHADOW_PCF_SIZE));
        if(App->hints->GetBoolValue(ModuleHints::SHADOW_ENABLE_SOFT) )
        {
            glActiveTexture(GL_TEXTURE8);
            glBindTexture(GL_TEXTURE_2D, cascades[0].blur_tex_1);
            glUniform1i(App->programs->GetUniformLocation("shadow_map[0]"), 8);

            glActiveTexture(GL_TEXTURE9);
            glBindTexture(GL_TEXTURE_2D, cascades[1].blur_tex_1);
            glUniform1i(App->programs->GetUniformLocation("shadow_map[1]"), 9);

            glActiveTexture(GL_TEXTURE9+1);
            glBindTexture(GL_TEXTURE_2D, cascades[2].blur_tex_1);
            glUniform1i(App->programs->GetUniformLocation("shadow_map[2]"), 10);
        }
        else
        {
            glActiveTexture(GL_TEXTURE8);
            glBindTexture(GL_TEXTURE_2D, cascades[0].tex);
            glUniform1i(App->programs->GetUniformLocation("shadow_map[0]"), 8);

            glActiveTexture(GL_TEXTURE9);
            glBindTexture(GL_TEXTURE_2D, cascades[1].tex);
            glUniform1i(App->programs->GetUniformLocation("shadow_map[1]"), 9);

            glActiveTexture(GL_TEXTURE9+1);
            glBindTexture(GL_TEXTURE_2D, cascades[2].tex);
            glUniform1i(App->programs->GetUniformLocation("shadow_map[2]"), 10);
        }
    }

    App->programs->UnuseProgram();

    App->programs->UseProgram("particles", 0);
    glUniformMatrix4fv(App->programs->GetUniformLocation("proj"), 1, GL_TRUE, reinterpret_cast<const float*>(&proj));
    glUniformMatrix4fv(App->programs->GetUniformLocation("view"), 1, GL_TRUE, reinterpret_cast<const float*>(&view));
    App->programs->UnuseProgram();

    App->programs->UseProgram("trails", 0);
    glUniformMatrix4fv(App->programs->GetUniformLocation("proj"), 1, GL_TRUE, reinterpret_cast<const float*>(&proj));
    glUniformMatrix4fv(App->programs->GetUniformLocation("view"), 1, GL_TRUE, reinterpret_cast<const float*>(&view));
    App->programs->UnuseProgram();

	App->programs->UseProgram("default_batch", flags);
	glUniformMatrix4fv(App->programs->GetUniformLocation("proj"), 1, GL_TRUE, reinterpret_cast<const float*>(&proj));
	glUniformMatrix4fv(App->programs->GetUniformLocation("view"), 1, GL_TRUE, reinterpret_cast<const float*>(&view));
	App->programs->UnuseProgram();


    // Render Batches
    //DrawBatches(opaque_nodes, flags);
    DrawNodes(render_list.GetOpaques(), &ModuleRenderer::DrawColor);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    //DrawBatches(transparent_nodes, flags);
    DrawNodes(render_list.GetTransparents(), &ModuleRenderer::DrawColor);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void ModuleRenderer::SelectionPass(const float4x4& proj, const float4x4& view)
{
    // Set camera uniforms shared for all
    App->programs->UseProgram("selection", 0);

    glUniformMatrix4fv(App->programs->GetUniformLocation("proj"), 1, GL_TRUE, reinterpret_cast<const float*>(&proj));
    glUniformMatrix4fv(App->programs->GetUniformLocation("view"), 1, GL_TRUE, reinterpret_cast<const float*>(&view));

    DrawNodes(render_list.GetOpaques(), &ModuleRenderer::DrawSelection);
    DrawNodes(render_list.GetTransparents(), &ModuleRenderer::DrawSelection);
}

void ModuleRenderer::DrawBatches(NodeList& nodes, uint render_flags)
{
	for(auto it = nodes.begin(); it != nodes.end(); )
	{
        const TRenderInfo& render_info = *it;
        if(render_info.mesh != nullptr && render_info.mesh->GetBatchIndex() != UINT_MAX && render_info.mesh->GetBatchObjectIndex() != UINT_MAX)
        {
            batch_manager->AddToRender(render_info.mesh->GetBatchIndex(), render_info.mesh->GetBatchObjectIndex());
            it = nodes.erase(it);
        }
        else
        {
            ++it;
        }
    }

    App->programs->UseProgram("default_batch", render_flags); // enable batch
    batch_manager->DoRender();
    App->programs->UnuseProgram();

}

void ModuleRenderer::DrawNodes(const NodeList& nodes, void (ModuleRenderer::*drawer)(const TRenderInfo&)) 
{
	for(NodeList::const_iterator it = nodes.begin(), end = nodes.end(); it != end; ++it)
	{
        (this->*drawer)(*it);
    }
}

void ModuleRenderer::DrawColor(const TRenderInfo& render_info)
{    
    if(render_info.mesh)
    {
        DrawMeshColor(render_info.mesh);
    }
    else if(render_info.particles && render_info.particles->GetVisible())
    {
        DrawParticles(render_info.particles);
    }
    else if(render_info.trail && render_info.trail)
    {
        DrawTrails(render_info.trail);
    }
}

void ModuleRenderer::DrawShadow(const TRenderInfo& render_info)
{
    if(render_info.mesh /*&& render_info.mesh->cast_shadows*/)
    {
        render_info.mesh->DrawShadowPass();
    }
}

void ModuleRenderer::DrawSelection(const TRenderInfo& render_info)
{
    App->programs->UseProgram("selection", 0);

    uint uid = render_info.go->GetUID();

	glUniform1f(1, *((float*)&uid));

    if(render_info.mesh)
    {
        // update selection uniform
        render_info.mesh->Draw();
    }
    else if(render_info.particles)
    {
        // update selection uniform
        render_info.particles->Draw(App->hints->GetBoolValue(ModuleHints::SHOW_PARTICLE_BILLBOARDS));
    }
    else if(render_info.trail && render_info.trail)
    {
        // update selection uniform
        render_info.trail->Draw();
    }
}

void ModuleRenderer::DrawMeshColor(const ComponentMeshRenderer* mesh)
{
    uint flags = 0;

    if(App->hints->GetBoolValue(ModuleHints::ENABLE_SHADOW_MAPPING))
    {
        flags |= (1<< 0);

        if(App->hints->GetBoolValue(ModuleHints::SHADOW_SHOW_CASCADES))
        {
            flags |= (1<< 1);
        }

        if(App->hints->GetBoolValue(ModuleHints::SHADOW_ENABLE_SOFT))
        {
            flags |= (1 << 2);
        }
    }

    App->programs->UseProgram("default",  flags);

    UpdateLightUniform();
    mesh->Draw();
}

void ModuleRenderer::DrawParticles(ComponentParticleSystem* particles)
{
    App->programs->UseProgram("particles", 0);
    particles->Draw(App->hints->GetBoolValue(ModuleHints::SHOW_PARTICLE_BILLBOARDS));
}

void ModuleRenderer::DrawTrails(ComponentTrail* trail)
{
    App->programs->UseProgram("trails", 0);
    trail->Draw();
}

void ModuleRenderer::LoadDefaultShaders()
{
    const char* default_macros[]	= { "#define SHADOWS_ENABLED 1 \n" , "#define SHOW_CASCADES 1 \n", "#define ENABLE_SOFT 1 \n"}; 
    const unsigned num_default_macros  = sizeof(default_macros)/sizeof(const char*);

    App->programs->Load("default", "Assets/Shaders/default.vs", "Assets/Shaders/default.ggx.fs", default_macros, num_default_macros);
    App->programs->Load("default_batch", "Assets/Shaders/default_batch.vs", "Assets/Shaders/default_batch.fs", default_macros, num_default_macros);

    const char* macros[]		  = { "#define BLOOM 1 \n", "#define GAMMA 1\n" }; 
    const unsigned num_macros     = sizeof(macros)/sizeof(const char*);

    App->programs->Load("postprocess", "Assets/Shaders/postprocess.vs", "Assets/Shaders/postprocess.fs", macros, num_macros);
    App->programs->Load("skybox", "Assets/Shaders/skybox.vs", "Assets/Shaders/skybox.fs", nullptr, 0);
    App->programs->Load("particles", "Assets/Shaders/particles.vs", "Assets/Shaders/particles.fs", nullptr, 0);
    App->programs->Load("trails", "Assets/Shaders/trails.vs", "Assets/Shaders/trails.fs", nullptr, 0);
    App->programs->Load("shadow", "Assets/Shaders/shadow.vs", "Assets/Shaders/shadow.fs", nullptr, 0);

    const char* gaussian_macros[]       = { "#define HORIZONTAL 1 \n" }; 
    const unsigned num_gaussian_macros  = sizeof(gaussian_macros)/sizeof(const char*);

    App->programs->Load("gaussian", "Assets/Shaders/postprocess.vs", "Assets/Shaders/gaussian.fs", gaussian_macros, num_gaussian_macros);
    App->programs->Load("chebyshev", "Assets/Shaders/fullscreenVS.glsl", "Assets/Shaders/chebyshev.fs", nullptr, 0, nullptr);

    const char* bloom_macros[]       = { "#define MSAA 1 \n" }; 
    const unsigned num_bloom_macros  = sizeof(bloom_macros)/sizeof(const char*);

    App->programs->Load("bloom", "Assets/Shaders/postprocess.vs", "Assets/Shaders/bloom.fs", bloom_macros, num_bloom_macros);

    const char* show_uv_macros[]       = { "#define TEXCOORD1 1 \n" }; 
    const unsigned num_uv_macros  = sizeof(show_uv_macros)/sizeof(const char*);
    App->programs->Load("show_uvs", "Assets/Shaders/show_uvs.vs", "Assets/Shaders/show_uvs.fs", show_uv_macros, num_uv_macros);
    App->programs->Load("selection", "Assets/Shaders/selection.vs", "Assets/Shaders/selection.fs", nullptr, 0);
    App->programs->Load("color", "Assets/Shaders/color.vs", "Assets/Shaders/color.fs", nullptr, 0);
    App->programs->Load("grid", "Assets/Shaders/gridVS.glsl", "Assets/Shaders/gridPS.glsl", nullptr, 0);
    App->programs->Load("showtexture", "Assets/Shaders/fullscreenVS.glsl", "Assets/Shaders/fullscreenTextureFS.glsl", nullptr, 0);
    App->programs->Load("convert_texture", "Assets/Shaders/fullscreenVS.glsl", "Assets/Shaders/convertMetallicTextureFS.glsl", nullptr, 0);
    //App->programs->Load("postprocess", "Assets/Shaders/fullscreenVS.glsl", "Assets/Shaders/postprocess.glsl", nullptr, 0);
}

void ModuleRenderer::UpdateLightUniform() const
{
    const AmbientLight* ambient = App->level->GetAmbientLight();
    const DirLight* directional = App->level->GetDirLight();

    float3 dir = directional->GetDir();

    glUniform3fv(AMBIENT_COLOR_LOC, 1, (const float*)&ambient->GetColor());
    glUniform3fv(DIRECTIONAL_DIR_LOC, 1, (const float*)&dir);
    glUniform3fv(DIRECTIONAL_COLOR_LOC, 1, (const float*)&directional->GetColor());

    uint num_point = min(App->level->GetNumPointLights(), MAX_NUM_POINT_LIGHTS);
    uint count = 0;

    for(uint i=0; i< num_point; ++i)
    {
        const PointLight* light = App->level->GetPointLight(i);

        if(light->GetEnabled())
        {
            glUniform3fv(POINT0_POSITION_LOC+count*3, 1, (const float*)&light->GetPosition());
            glUniform3fv(POINT0_COLOR_LOC+count*3, 1, (const float*)&light->GetColor());
            glUniform3f(POINT0_ATTENUATION_LOC+count*3, light->GetConstantAtt(), light->GetLinearAtt(), light->GetQuadricAtt());

            ++count;
        }
    }

    for(uint i=count; i < MAX_NUM_POINT_LIGHTS; ++i)
    {
        glUniform3f(POINT0_COLOR_LOC+i*3, 0.0f, 0.0f, 0.0f);
        glUniform3f(POINT0_ATTENUATION_LOC+i*3, 1.0f, 0.0f, 0.0f);
    }

    glUniform1ui(NUM_POINT_LIGHT_LOC, count);

    uint num_spot = min(App->level->GetNumSpotLights(), MAX_NUM_SPOT_LIGHTS);
    count = 0;

    for(uint i=0; i< num_spot; ++i)
    {
        const SpotLight* light = App->level->GetSpotLight(i);

        if(light->GetEnabled())
        {
            glUniform3fv(SPOT0_POSITION_LOC+count*6, 1, (const float*)&light->GetPosition());
            glUniform3fv(SPOT0_DIRECTION_LOC+count*6, 1, (const float*)&light->GetDirection());
            glUniform3fv(SPOT0_COLOR_LOC+count*6, 1, (const float*)&light->GetColor());
            glUniform3f(SPOT0_ATTENUATION_LOC+count*6, light->GetConstantAtt(), light->GetLinearAtt(), light->GetQuadricAtt());
            glUniform1f(SPOT0_INNER_LOC+count*6, cos(light->GetInnerCutoff()));
            glUniform1f(SPOT0_OUTTER_LOC+count*6, cos(light->GetOutterCutoff()));
            ++count;
        }
    }

    for(uint i=count; i < MAX_NUM_SPOT_LIGHTS; ++i)
    {
        glUniform3f(SPOT0_COLOR_LOC+i*6, 0.0f, 0.0f, 0.0f);
        glUniform3f(SPOT0_ATTENUATION_LOC+i*6, 1.0f, 0.0f, 0.0f);
    }

    glUniform1ui(NUM_SPOT_LIGHT_LOC, count);
}

void ModuleRenderer::DrawDebug()
{
    DebugDrawOBB();
}

void ModuleRenderer::DebugDrawOBB(const NodeList& objects)
{
    for(const TRenderInfo& render_info : objects)
    {
        AABB local_bounding = render_info.go->GetLocalBBox();

        if(local_bounding.IsFinite())
        {
            float4x4 transform = render_info.go->GetGlobalTransformation();

            OBB global_bounding = local_bounding.Transform(transform);

            math::float3 corners[8];
            global_bounding.GetCornerPoints(corners);
            std::swap(corners[2], corners[5]);
            std::swap(corners[3], corners[4]);
            std::swap(corners[4], corners[5]);
            std::swap(corners[6], corners[7]);

            dd::box(corners, dd::colors::Sienna); 

        }
    }

}

void ModuleRenderer::DebugDrawOBB()
{
    DebugDrawOBB(render_list.GetOpaques());
    DebugDrawOBB(render_list.GetTransparents());
}

void ModuleRenderer::DebugDrawAnimation()
{
    DebugDrawAnimation(App->level->GetRoot());
}

void ModuleRenderer::DebugDrawAnimation(const GameObject* go)
{
    const ComponentAnimation* animation = go->FindFirstComponent<ComponentAnimation>();
    if(animation && animation->GetDebugDraw())
    {
        DebugDrawHierarchy(go);
    }
    else
    {
        for(std::list<GameObject*>::const_iterator it = go->childs.begin(), end = go->childs.end(); it != end; ++it)
        {
            DebugDrawAnimation((*it));
        }
    }
}

void ModuleRenderer::DebugDrawHierarchy(const GameObject* go)
{
    const float4x4& transform = go->GetGlobalTransformation();

    if(go->GetParent())
    {
        const float4x4& parent_transform = go->GetParent()->GetGlobalTransformation();

        dd::line(parent_transform.TranslatePart(), transform.TranslatePart(), dd::colors::Blue, 0, false);
		//dd::axisTriad(transform, 1.3f,  13.f, 0, false);
    }

    for(std::list<GameObject*>::const_iterator it = go->childs.begin(), end = go->childs.end(); it != end; ++it)
    {
        DebugDrawHierarchy((*it));
    }
}

void ModuleRenderer::DebugDrawTangentSpace()
{
	/*
    for(NodeList::iterator it = opaque_nodes.begin(), end = opaque_nodes.end(); it != end; ++it)
    {
        if(it->material && it->material->GetDDTangent())
        {
            const ResourceMesh* mesh = it->mesh->GetResource();

            if(mesh && (mesh->attribs & ResourceMesh::ATTRIB_TANGENTS) != 0 && (mesh->attribs& ResourceMesh::ATTRIB_NORMALS))
            {
                DebugDrawTangentSpace(mesh, it->transform);
            }
        }
    }

    for(NodeList::iterator it = transparent_nodes.begin(), end = transparent_nodes.end(); it != end; ++it)
    {
        if(it->material && it->material->GetDDTangent())
        {
            const ResourceMesh* mesh = it->mesh->GetResource();

            if(mesh && (mesh->attribs & ResourceMesh::ATTRIB_TANGENTS) != 0 && (mesh->attribs& ResourceMesh::ATTRIB_NORMALS))
            {
                DebugDrawTangentSpace(mesh, it->transform);
            }
        }
    }
	*/
}

void ModuleRenderer::DebugDrawTangentSpace(const ResourceMesh* mesh, const float4x4& transform)
{
    float metric_proportion = App->hints->GetFloatValue(ModuleHints::METRIC_PROPORTION);

    for(unsigned i = 0, count = mesh->num_vertices; i < count; ++i)
    {
        float3 position  = transform.TransformPos(mesh->src_vertices[i]);
        float3 normal    = transform.TransformDir(mesh->src_normals[i]);
        float3 tangent   = transform.TransformDir(mesh->src_tangents[i]);
        float3 bitangent = normal.Cross(tangent);

        float4x4 tbn(float4(tangent, 0.0f), float4(bitangent, 0.0f), float4(normal, 0.0f), float4(position, 1.0f));

        dd::axisTriad(tbn, metric_proportion*0.1f*0.1f, metric_proportion*0.1f, 0);
    }
}

void ModuleRenderer::BlurShadow(uint index)
{
    glBindVertexArray(0);


    glBindFramebuffer(GL_FRAMEBUFFER, cascades[index].sq_fbo);
    {
        App->programs->UseProgram("chebyshev", 0);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, cascades[index].tex);
        glUniform1i(App->programs->GetUniformLocation("image"), 0); 
        glDrawArrays(GL_TRIANGLES, 0, 3);
    }

    float weights[] = { 0.38774f,	0.24477f, 0.06136f };


    glBindFramebuffer(GL_FRAMEBUFFER, cascades[index].blur_fbo_0);
    {
        App->programs->UseProgram("gaussian", 1);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, cascades[index].sq_tex);
        glUniform1i(App->programs->GetUniformLocation("image"), 0); 
        glUniform1fv(App->programs->GetUniformLocation("weight"), sizeof(weights)/sizeof(float), weights);
        glDrawArrays(GL_TRIANGLES, 0, 3);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, cascades[index].blur_fbo_1);
    {
        App->programs->UseProgram("gaussian", 0);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, cascades[index].blur_tex_0);
        glUniform1i(App->programs->GetUniformLocation("image"), 0); 
        glUniform1fv(App->programs->GetUniformLocation("weight"), sizeof(weights)/sizeof(float), weights);
        glDrawArrays(GL_TRIANGLES, 0, 3);
        glDrawArrays(GL_TRIANGLES, 0, 3);
    }


    glBindVertexArray(0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    App->programs->UnuseProgram();
}

void ModuleRenderer::ComputeDirLightShadowVolume(ComponentCamera* camera, uint index)
{
    float2 depth(0, 0);

    switch(index)
    {
        case 2:
            depth.x = App->hints->GetFloat2Value(ModuleHints::SHADOW_CASCADE_0_DEPTH).y+
                      App->hints->GetFloatValue(ModuleHints::SHADOW_CASCADE_1_DEPTH);
            depth.y = depth.x+App->hints->GetFloatValue(ModuleHints::SHADOW_CASCADE_2_DEPTH);
            break;
        case 1:
            depth.x = App->hints->GetFloat2Value(ModuleHints::SHADOW_CASCADE_0_DEPTH).y;
            depth.y += depth.x+App->hints->GetFloatValue(ModuleHints::SHADOW_CASCADE_1_DEPTH);
            break;
        case 0:
            depth = App->hints->GetFloat2Value(ModuleHints::SHADOW_CASCADE_0_DEPTH);
            break;
    }

    const DirLight* light = App->level->GetDirLight();

	if (light == nullptr)
    {
        cascades[index].proj = float4x4::identity;
        cascades[index].view = float4x4::identity;
    }
    else
	{
        float3 front        = light->GetDir();
        float3 up           = light->GetUp();
        Quat light_rotation = Quat::LookAt(-float3::unitZ, front, float3::unitY, up); 

        cascades[index].casters.clear();
        cascades[index].frustum = camera->frustum;
        cascades[index].frustum.nearPlaneDistance = depth[0];
        cascades[index].frustum.farPlaneDistance = depth[1];

        CalcLightCameraBBox(light_rotation, camera, depth[0], depth[1], cascades[index].aabb);
        CalcLightObjectsBBox(light_rotation, front, cascades[index].aabb, cascades[index].casters);

        cascades[index].world_bb = cascades[index].aabb.Transform(light_rotation);
        float3 center = cascades[index].aabb.CenterPoint();

        // \note: Orthographic projection is invalid z [0..1] like DirectX
        Frustum frustum; 
        frustum.type               = FrustumType::OrthographicFrustum;
        frustum.pos                = light_rotation.Transform(float3(center.x, center.y, cascades[index].aabb.maxPoint.z));
        frustum.front              = front;
        frustum.up                 = up;
        frustum.nearPlaneDistance  = 0.0f;
        frustum.farPlaneDistance   = (cascades[index].aabb.maxPoint.z - cascades[index].aabb.minPoint.z);
        frustum.orthographicWidth  = (cascades[index].aabb.maxPoint.x - cascades[index].aabb.minPoint.x);
        frustum.orthographicHeight = (cascades[index].aabb.maxPoint.y - cascades[index].aabb.minPoint.y);

        //float4x4 persp_mtx = ComputePerspShadowMtx(camera->GetPos(), camera->GetFront(), front, frustum.ViewMatrix().RotatePart());

        /*
        cascades[index].proj = SetOrtho(-frustum.orthographicWidth/2, frustum.orthographicWidth/2,
                                        -frustum.orthographicHeight/2, frustum.orthographicHeight/2, 
                                        frustum.nearPlaneDistance, frustum.farPlaneDistance);*/

        cascades[index].proj = frustum.ProjectionMatrix();
        cascades[index].view = frustum.ViewMatrix();
    }
}

float4x4 ModuleRenderer::ComputePerspShadowMtx(const float3& view_pos, const float3& view_dir, const float3& light_dir, const float3x3& light_view)
{
    // Apply gl frustum
    
    float3 left_dir = light_dir.Cross(view_dir);
    float3 up_dir = left_dir.Cross(light_dir).Normalized();
    
    float4x4 view_transform = float4x4::LookAt(-float3::unitZ, light_dir, float3::unitY, up_dir);
    float4x4 warp = float4x4::identity;
    float _far = 20.0f;
    float _near = 2.0f;

    warp.At(1, 1) = (_far + _near) / (_far - _near);
    warp.At(1, 3) = -2.0f * _far * _near / (_far - _near);
    warp.At(3, 1) = 1.0f;
    warp.At(3, 3) = 0.0f;

    float3 f = view_pos; // -(2.0f * up_dir);

    view_transform.SetTranslatePart(float3(f.Dot(view_transform.Row3(0)), f.Dot(view_transform.Row3(1)), -f.Dot(view_transform.Row3(2))));

    return warp*view_transform;

    //return SetFrustum(-1.0f, 1.0f, -1.0f, 1.0f, 1.0f, 20.0f)*float4x4::LookAt(-float3::unitZ, light_dir, float3::unitY, up_dir);
    //persp_mtx = float4x4::LookAt(-float3::unitZ, view_dir, float3::unitY, light_dir);
    
    //persp_mtx.SetTranslatePart(-view_dir*8.0f);
}

float4x4 ModuleRenderer::SetFrustum(float left, float right, float bottom, float top, float _near, float _far)
{
    float rl = right-left;
    float tb = top-bottom;
    float fn = _far-_near;

    float4x4 frustum;
    frustum.SetCol(0, 2.0f*_near/rl,             0.0f,                 0.0f,  0.0f);
    frustum.SetCol(1, 0.0f,              2.0f*_near/tb,                0.0f,  0.0f);
    frustum.SetCol(2, (right+left)/rl, (top+bottom)/tb,    -(_far+_near)/fn, -1.0f);
    frustum.SetCol(3, 0.0f,                       0.0f, -2.0f*_far*_near/fn, 0.0f);

    return frustum;
}

float4x4 ModuleRenderer::SetOrtho(float left, float right, float bottom, float top, float _near, float _far)
{
	float a = 2.0f / (right - left);
	float b = 2.0f / (top - bottom);
	float c = -2.0f / (_far - _near);

	float tx = -(right + left) / (right - left);
	float ty = - (top + bottom)/(top - bottom);
	float tz = - (_far + _near)/(_far - _near);

    float4x4 projection;
    projection.SetRow(0, math::float4(a, 0.0f, 0.0f, 0.0f));
    projection.SetRow(1, math::float4(0.0f , b, 0.0f, 0.0f));
    projection.SetRow(2, math::float4(0.0f , 0.0f, c, 0.0f));
    projection.SetRow(3, math::float4(tx, ty, tz, 1.0f));

    return projection;
}

void ModuleRenderer::CalcLightCameraBBox(const Quat& light_rotation, const ComponentCamera* camera, float near_distance, float far_distance, AABB& aabb)
{
    float3 camera_points[8];
    float3 light_points[8];

    aabb.SetNegativeInfinity();

    float4x4 light_mat = light_rotation.Inverted().ToFloat3x3();

    Frustum f = camera->frustum;
    f.farPlaneDistance = far_distance;
    f.nearPlaneDistance = near_distance;
    f.GetCornerPoints(camera_points);
    std::swap(camera_points[2], camera_points[5]);
    std::swap(camera_points[3], camera_points[4]);
    std::swap(camera_points[4], camera_points[5]);
    std::swap(camera_points[6], camera_points[7]);

    for(uint i=0; i< 8; ++i)
    {
        light_points[i] = light_mat.TransformPos(camera_points[i]);
    }

    aabb.Enclose(light_points, 8);
}

void ModuleRenderer::CalcLightObjectsBBox(const float4x4& light_mat, const AABB& camera_aabb, AABB& aabb, NodeList& casters, const NodeList& objects)
{
    for(const TRenderInfo& render_info : objects)
    {
        if(render_info.mesh->CastShadows())
        {
			render_info.go->RecalculateBoundingBox();
            AABB bbox = render_info.go->GetLocalBBox();

            if(bbox.IsFinite())
            {
                float4x4 transform = light_mat*render_info.go->GetGlobalTransformation();

                bbox.TransformAsAABB(transform);

                bbox.Scale(bbox.CenterPoint(), 1.5f);

                if(camera_aabb.Intersects(bbox))
                {
                    aabb.Enclose(bbox);
                    casters.push_back(render_info);
                }
            }
        }
    }
}

void ModuleRenderer::CalcLightObjectsBBox(const Quat& light_rotation, const float3& light_dir, AABB& aabb, NodeList& casters)
{
    float4x4 light_mat = light_rotation.Inverted().ToFloat3x3();
    AABB camera_aabb = aabb;
    AABB big_camera_aabb = aabb;
	//camera_aabb.maxPoint.z = FLOAT_INF;
    aabb.SetNegativeInfinity();


    // Make camera frustum bigger in light direction

    static const float expand_amount = 30.0f;

    for (uint i = 0; i < 3; ++i)
    {
        if (light_dir[i] > 0.0f)
            big_camera_aabb.minPoint[i] -= expand_amount;
        else if (light_dir[i] < 0.0f)
            big_camera_aabb.maxPoint[i] += expand_amount;
    }

    CalcLightObjectsBBox(light_mat, big_camera_aabb, aabb, casters, render_list.GetOpaques());
    CalcLightObjectsBBox(light_mat, big_camera_aabb, aabb, casters, render_list.GetTransparents());

	aabb = aabb.Intersection(camera_aabb);

    // \todo: for transparents

    // \todo: 
    // En una situación real faltaria meter objetos que, estando fuera del frustum estan dentro de los dos planos laterales de volumen ortogonal en
    // dirección a la luz. Estos objetos, a pesar de no estar en el frustum, prodrían arrojar sombras sobre otros que si lo están.
    // Ojo!!!!!!!!!!!!!!!
}

void ModuleRenderer::GenerateShadowFBO(ShadowMap& map, unsigned width, unsigned height)
{
    if(width != map.width || height != map.height)
    {
        if(map.tex != 0)
        {
            glDeleteTextures(1, &map.tex);
        }

        if(width != 0 && height != 0)
        {
            glBindFramebuffer(GL_FRAMEBUFFER, map.fbo);
            glGenTextures(1, &map.tex);
            glBindTexture(GL_TEXTURE_2D, map.tex);

            glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); 
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);

            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, map.tex, 0);

            glDrawBuffer(GL_NONE);

            glBindFramebuffer(GL_FRAMEBUFFER, map.sq_fbo);
            glGenTextures(1, &map.sq_tex);
            glBindTexture(GL_TEXTURE_2D, map.sq_tex);

            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, width, height, 0, GL_RGB, GL_FLOAT, 0);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, map.sq_tex, 0);

            glDrawBuffer(GL_COLOR_ATTACHMENT0);

            glBindFramebuffer(GL_FRAMEBUFFER, map.blur_fbo_0);
            glGenTextures(1, &map.blur_tex_0);
            glBindTexture(GL_TEXTURE_2D, map.blur_tex_0);

            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, width, height, 0, GL_RGB, GL_FLOAT, 0);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, map.blur_tex_0, 0);

            glDrawBuffer(GL_COLOR_ATTACHMENT0);

            glBindFramebuffer(GL_FRAMEBUFFER, map.blur_fbo_1);
            glGenTextures(1, &map.blur_tex_1);
            glBindTexture(GL_TEXTURE_2D, map.blur_tex_1);

            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, width, height, 0, GL_RGB, GL_FLOAT, 0);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, map.blur_tex_1, 0);

            glDrawBuffer(GL_COLOR_ATTACHMENT0);

            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            glBindTexture(GL_TEXTURE_2D, 0);
        }

		map.width  = width;
		map.height = height;
    }
}

