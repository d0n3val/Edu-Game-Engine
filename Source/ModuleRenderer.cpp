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

#include "ResourceMesh.h"
#include "ResourceMaterial.h"
#include "ResourceTexture.h"

#include "AmbientLight.h"
#include "DirLight.h"
#include "PointLight.h"
#include "SpotLight.h"

#include "Application.h"

#include "OpenGL.h"
#include "DebugDraw.h"

#include "imgui/imgui.h"

#include "SOIL2.h"

#include <string>
#include <functional>
#include <algorithm>

#include "mmgr/mmgr.h"

ModuleRenderer::ModuleRenderer() : Module("renderer")
{
}

bool ModuleRenderer::Init(Config* config /*= nullptr*/)
{
    LoadDefaultShaders();
    CreatePostprocessData();
	CreateSkybox();

	return true;
}

void ModuleRenderer::CreatePostprocessData()
{
	float vertex_buffer_data[] =
	{
        // positions
		-1.0f, -1.0f, 0.0f,
		1.0f, -1.0f, 0.0f,
		-1.0f, 1.0f, 0.0f,

		-1.0f, 1.0f, 0.0f,
		1.0f, -1.0f, 0.0f,
		1.0f,  1.0f, 0.0f,

        // uvs
		0.0f, 0.0f,
		1.0f, 0.0f,
		0.0f, 1.0f,
		
		0.0f, 1.0f,
		1.0f, 0.0f,
		1.0f, 1.0f
	};

    glGenBuffers(1, &post_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, post_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_buffer_data), vertex_buffer_data, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glGenVertexArrays(1, &post_vao);
    glBindVertexArray(post_vao);

    glBindBuffer(GL_ARRAY_BUFFER, post_vbo);
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)(sizeof(float)*3*6));

    glBindVertexArray(0);
}

void ModuleRenderer::CreateSkybox()
{
    const char face_order[9] = { 'N', 'S', 'W', 'E', 'U', 'D' };

	//int width, height, channels;
	//unsigned char* data = SOIL_load_image("Assets/Textures/PBR/BarnaEnvHDR.dds", &width, &height, &channels, SOIL_LOAD_AUTO);
    sky_cubemap = SOIL_load_OGL_single_cubemap("Assets/Textures/PBR/BarnaEnvHDR.dds", face_order, 3, 0, SOIL_FLAG_DDS_LOAD_DIRECT);
    sky_irradiance = SOIL_load_OGL_single_cubemap("Assets/Textures/PBR/BarnaDiffuseHDR.dds", face_order, 3, 0, SOIL_FLAG_DDS_LOAD_DIRECT);
    sky_prefilter = SOIL_load_OGL_single_cubemap("Assets/Textures/PBR/BarnaSpecularHDR.dds", face_order, 3, 0, SOIL_FLAG_DDS_LOAD_DIRECT);
    sky_brdf = SOIL_load_OGL_texture("Assets/Textures/PBR/BarnaBRDF.dds", 3, 0, SOIL_FLAG_DDS_LOAD_DIRECT);
	
	/*
	sky_cubemap = SOIL_load_OGL_single_cubemap("Assets/Textures/PBR/MilkywayEnvHDR.dds", face_order, 3, 0, SOIL_FLAG_DDS_LOAD_DIRECT);
    sky_irradiance = SOIL_load_OGL_single_cubemap("Assets/Textures/PBR/MilkywayDiffuseHDR.dds", face_order, 3, 0, SOIL_FLAG_DDS_LOAD_DIRECT);
    sky_prefilter = SOIL_load_OGL_single_cubemap("Assets/Textures/PBR/MilkywaySpecularHDR.dds", face_order, 3, 0, SOIL_FLAG_DDS_LOAD_DIRECT);
    sky_brdf = SOIL_load_OGL_texture("Assets/Textures/PBR/MilkywayBRDF.dds", 3, 0, SOIL_FLAG_DDS_LOAD_DIRECT);
    */
    

    glGenBuffers(1, &sky_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, sky_vbo);

    float sky_vertices[] = {
        // positions          
        -1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

        1.0f, -1.0f, -1.0f,
        1.0f, -1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f,  1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

        -1.0f,  1.0f, -1.0f,
        1.0f,  1.0f, -1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
        1.0f, -1.0f,  1.0f
    };

    glBufferData(GL_ARRAY_BUFFER, sizeof(float3)*6*6, sky_vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glGenVertexArrays(1, &sky_vao);
    glBindVertexArray(sky_vao);

    glBindBuffer(GL_ARRAY_BUFFER, sky_vbo);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float3), (void*)0);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    for(uint i=0; i< CASCADE_COUNT; ++i)
    {
        glGenFramebuffers(1, &cascades[i].fbo);
        glGenFramebuffers(1, &cascades[i].sq_fbo);
        glGenFramebuffers(1, &cascades[i].blur_fbo_0);
        glGenFramebuffers(1, &cascades[i].blur_fbo_1);
    }

    glGenFramebuffers(1, &bloom_fbo);
    glGenFramebuffers(1, &bloom_blur_fbo_0);
    glGenFramebuffers(1, &bloom_blur_fbo_1);
}

ModuleRenderer::~ModuleRenderer()
{
    if(post_vbo != 0)
    {
        glDeleteBuffers(1, &post_vbo);
    }

    if(post_vao != 0)
    {
        glDeleteVertexArrays(1, &post_vao);
    }

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

    if(bloom_tex)
    {
        glDeleteTextures(1, &bloom_tex);
    }

    if(color_tex)
    {
        glDeleteTextures(1, &color_tex);
    }

    if(bloom_blur_tex_0)
    {
        glDeleteTextures(1, &bloom_blur_tex_0);
    }

    if(bloom_blur_tex_1)
    {
        glDeleteTextures(1, &bloom_blur_tex_1);
    }

    if(bloom_fbo)
    {
        glDeleteFramebuffers(1, &bloom_fbo);
    }

    if(bloom_blur_fbo_0)
    {
        glDeleteFramebuffers(1, &bloom_blur_fbo_0);
    }

    if(bloom_blur_fbo_1)
    {
        glDeleteFramebuffers(1, &bloom_blur_fbo_1);
    }
}

void ModuleRenderer::Draw(ComponentCamera* camera, unsigned fbo, unsigned width, unsigned height)
{
	opaque_nodes.clear();
    transparent_nodes.clear();

	float4x4 proj   = camera->GetProjectionMatrix();	
	float4x4 view   = camera->GetViewMatrix();
    float3 view_pos = view.RotatePart().Transposed().Transform(-view.TranslatePart());

    CollectObjects(view_pos, App->level->GetRoot());

    if(App->hints->GetBoolValue(ModuleHints::ENABLE_SHADOW_MAPPING))
    {
        ShadowPass(camera, width, height);
    }

    ColorPass(proj, view, view_pos, fbo, width, height);
}

void ModuleRenderer::DrawForSelection(ComponentCamera* camera)
{
	opaque_nodes.clear();
    transparent_nodes.clear();

	float4x4 proj   = camera->GetProjectionMatrix();	
	float4x4 view   = camera->GetViewMatrix();
    float3 view_pos = view.RotatePart().Transposed().Transform(-view.TranslatePart());

    CollectObjects(view_pos, App->level->GetRoot());

    SelectionPass(proj, view);
}

void ModuleRenderer::ShadowPass(ComponentCamera* camera, unsigned width, unsigned height)
{
    uint shadow_width[3] = { 384, 384, 256};
    uint shadow_height[3] = { 384, 384, 256};

    cascades[0].near_distance = 100;
    cascades[1].near_distance = 1000;
    cascades[2].near_distance = 2000;

    cascades[0].far_distance = 1000;
    cascades[1].far_distance = 2000;
    cascades[2].far_distance = 10000;

    cascades[0].period = 6;
    cascades[1].period = 8;
    cascades[2].period = 12;

    for(uint i=0; i<  CASCADE_COUNT; ++i)
    {
        cascades[i].tick = ((cascades[i].tick+1)%cascades[i].period);

        if(cascades[i].tick == 0)
        {
            if(App->hints->GetBoolValue(ModuleHints::UPDATE_SHADOW_VOLUME))
            {
                ComputeDirLightShadowVolume(camera, i);
            }

            if(App->hints->GetBoolValue(ModuleHints::SHOW_SHADOW_CLIPPING))
            {
                math::float3 p[8];
                cascades[i].world_bb.GetCornerPoints(p);
                std::swap(p[2], p[5]);
                std::swap(p[3], p[4]);
                std::swap(p[4], p[5]);
                std::swap(p[6], p[7]);
                dd::box(p, i== 0 ? dd::colors::Red : (i == 1 ? dd::colors::Green : dd::colors::Blue));

                cascades[i].frustum.GetCornerPoints(p);
                std::swap(p[2], p[5]);
                std::swap(p[3], p[4]);
                std::swap(p[4], p[5]);
                std::swap(p[6], p[7]);
                dd::box(p, dd::colors::White);
            }

            GenerateShadowFBO(cascades[i], shadow_width[i], shadow_height[i]);

            glViewport(0, 0, shadow_width[i], shadow_height[i]);
            glBindFramebuffer(GL_FRAMEBUFFER, cascades[i].fbo);

            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            //glClear(GL_DEPTH_BUFFER_BIT);

            App->programs->UseProgram("shadow", 0);

            glUniformMatrix4fv(App->programs->GetUniformLocation("proj"), 1, GL_TRUE, reinterpret_cast<const float*>(&cascades[i].proj));
            glUniformMatrix4fv(App->programs->GetUniformLocation("view"), 1, GL_TRUE, reinterpret_cast<const float*>(&cascades[i].view));

            if(App->hints->GetBoolValue(ModuleHints::ENABLE_SHADOW_FRONT_CULLING))
            {
                glCullFace(GL_FRONT);
            }

            DrawNodes(cascades[i].casters, &ModuleRenderer::DrawShadow);

            if (App->hints->GetBoolValue(ModuleHints::ENABLE_SHADOW_FRONT_CULLING))
            {
                glCullFace(GL_BACK);
            }

            BlurShadow(i);
        }
    }
}

void ModuleRenderer::ColorPass(const float4x4& proj, const float4x4& view, const float3& view_pos, unsigned fbo, unsigned width, unsigned height)
{
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    // Set camera uniforms shared for all
    App->programs->UseProgram("default", App->hints->GetBoolValue(ModuleHints::ENABLE_SHADOW_MAPPING) ? 1 : 0);

    glUniformMatrix4fv(App->programs->GetUniformLocation("proj"), 1, GL_TRUE, reinterpret_cast<const float*>(&proj));
    glUniformMatrix4fv(App->programs->GetUniformLocation("view"), 1, GL_TRUE, reinterpret_cast<const float*>(&view));
    glUniform3f(App->programs->GetUniformLocation("view_pos"), view_pos.x, view_pos.y, view_pos.z);

    if(App->hints->GetBoolValue(ModuleHints::ENABLE_SHADOW_MAPPING))
    {
        glUniformMatrix4fv(App->programs->GetUniformLocation("light_proj[0]"), 1, GL_TRUE, reinterpret_cast<const float*>(&cascades[0].proj));
        glUniformMatrix4fv(App->programs->GetUniformLocation("light_proj[1]"), 1, GL_TRUE, reinterpret_cast<const float*>(&cascades[1].proj));
        glUniformMatrix4fv(App->programs->GetUniformLocation("light_proj[2]"), 1, GL_TRUE, reinterpret_cast<const float*>(&cascades[2].proj));
        glUniformMatrix4fv(App->programs->GetUniformLocation("light_view[0]"), 1, GL_TRUE, reinterpret_cast<const float*>(&cascades[0].view));
        glUniformMatrix4fv(App->programs->GetUniformLocation("light_view[1]"), 1, GL_TRUE, reinterpret_cast<const float*>(&cascades[1].view));
        glUniformMatrix4fv(App->programs->GetUniformLocation("light_view[2]"), 1, GL_TRUE, reinterpret_cast<const float*>(&cascades[2].view));

        glUniform1f(App->programs->GetUniformLocation("shadow_bias"), App->hints->GetFloatValue(ModuleHints::SHADOW_BIAS));
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

    App->programs->UnuseProgram();

    App->programs->UseProgram("particles", 0);
    glUniformMatrix4fv(App->programs->GetUniformLocation("proj"), 1, GL_TRUE, reinterpret_cast<const float*>(&proj));
    glUniformMatrix4fv(App->programs->GetUniformLocation("view"), 1, GL_TRUE, reinterpret_cast<const float*>(&view));
    App->programs->UnuseProgram();

    App->programs->UseProgram("trails", 0);
    glUniformMatrix4fv(App->programs->GetUniformLocation("proj"), 1, GL_TRUE, reinterpret_cast<const float*>(&proj));
    glUniformMatrix4fv(App->programs->GetUniformLocation("view"), 1, GL_TRUE, reinterpret_cast<const float*>(&view));
    App->programs->UnuseProgram();


    DrawNodes(opaque_nodes, &ModuleRenderer::DrawColor);
    DrawNodes(transparent_nodes, &ModuleRenderer::DrawColor);

    //DrawSkybox(proj, view);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void ModuleRenderer::SelectionPass(const float4x4& proj, const float4x4& view)
{
    // Set camera uniforms shared for all
    App->programs->UseProgram("selection", 0);

    glUniformMatrix4fv(App->programs->GetUniformLocation("proj"), 1, GL_TRUE, reinterpret_cast<const float*>(&proj));
    glUniformMatrix4fv(App->programs->GetUniformLocation("view"), 1, GL_TRUE, reinterpret_cast<const float*>(&view));


    DrawNodes(opaque_nodes, &ModuleRenderer::DrawSelection);
    DrawNodes(transparent_nodes, &ModuleRenderer::DrawSelection);
}

void ModuleRenderer::DrawSkybox(const float4x4& proj, const float4x4& view)
{
    if(sky_vao != 0)
    {
        App->programs->UseProgram("skybox", 0);

        glUniformMatrix4fv(App->programs->GetUniformLocation("proj"), 1, GL_TRUE, reinterpret_cast<const float*>(&proj));
        glUniformMatrix4fv(App->programs->GetUniformLocation("view"), 1, GL_TRUE, reinterpret_cast<const float*>(&view));

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, sky_cubemap);
        glUniform1i(App->programs->GetUniformLocation("skybox"), 0);

        glBindVertexArray(sky_vao);
        glDrawArrays(GL_TRIANGLES, 0, 6*6);
        glBindVertexArray(0);

        App->programs->UnuseProgram();
    }
}


void ModuleRenderer::CollectObjects(const float3& camera_pos, GameObject* go)
{
    CollectMeshRenderers(camera_pos, go);
    CollectParticleSystems(camera_pos, go);
    CollectTrails(camera_pos, go);

    for(std::list<GameObject*>::iterator lIt = go->childs.begin(), lEnd = go->childs.end(); lIt != lEnd; ++lIt)
    {
        CollectObjects(camera_pos, *lIt);
    }
}

void ModuleRenderer::CollectMeshRenderers(const float3& camera_pos, GameObject* go)
{
    std::vector<Component*> components;
    go->FindComponents(Component::MeshRenderer, components);

    float distance = (go->global_bbox.CenterPoint()-camera_pos).LengthSq();

    for(Component* comp : components)
    {
        TRenderInfo render;
        render.name = go->name.c_str();
        render.go   = go;
        render.mesh = static_cast<ComponentMeshRenderer*>(comp);
        render.distance = distance;

        if(render.mesh->GetVisible())
        {
            if(render.mesh->RenderMode() == ComponentMeshRenderer::RENDER_OPAQUE)
            {
                NodeList::iterator it = std::lower_bound(opaque_nodes.begin(), opaque_nodes.end(), render, TNearestMesh());

                opaque_nodes.insert(it, render);
            }
            else
            {
                NodeList::iterator it = std::lower_bound(transparent_nodes.begin(), transparent_nodes.end(), render, TFarthestMesh());

                transparent_nodes.insert(it, render);
            }
        }
    }
}

void ModuleRenderer::CollectParticleSystems(const float3& camera_pos, GameObject* go)
{
    std::vector<Component*> components;
    go->FindComponents(Component::ParticleSystem, components);

    float distance = (go->GetGlobalPosition()-camera_pos).LengthSq();

    for(Component* comp : components)
    {
        TRenderInfo render;
        render.name = go->name.c_str();
        render.go   = go;

        render.distance = distance;
        render.particles= static_cast<ComponentParticleSystem*>(comp);
        render.layer = render.particles->GetLayer();

        NodeList::iterator it = std::lower_bound(transparent_nodes.begin(), transparent_nodes.end(), render, TFarthestMesh());
        transparent_nodes.insert(it, render);
    }
}

void ModuleRenderer::CollectTrails(const float3& camera_pos, GameObject* go)
{
    std::vector<Component*> components;
    go->FindComponents(Component::ParticleSystem, components);

    float distance = (go->GetGlobalPosition()-camera_pos).LengthSq();

    for(Component* comp : components)
    {
        TRenderInfo render;
        render.name = go->name.c_str();
        render.go   = go;

        render.distance = distance;
        render.trail = static_cast<ComponentTrail*>(comp);

        NodeList::iterator it = std::lower_bound(transparent_nodes.begin(), transparent_nodes.end(), render, TFarthestMesh());
        transparent_nodes.insert(it, render);
    }
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
        render_info.particles->Draw(false);
    }
    else if(render_info.trail && render_info.trail)
    {
        // update selection uniform
        render_info.trail->Draw();
    }
}

void ModuleRenderer::DrawMeshColor(const ComponentMeshRenderer* mesh)
{
    App->programs->UseProgram("default", App->hints->GetBoolValue(ModuleHints::ENABLE_SHADOW_MAPPING) ? 1 : 0);

    UpdateLightUniform();
    mesh->Draw();
}

void ModuleRenderer::DrawParticles(ComponentParticleSystem* particles)
{
    App->programs->UseProgram("particles", 0);
    particles->Draw(false);
}

void ModuleRenderer::DrawTrails(ComponentTrail* trail)
{
    App->programs->UseProgram("trails", 0);
    trail->Draw();
}

void ModuleRenderer::LoadDefaultShaders()
{
    const char* default_macros[]	= { "#define SHADOWS_ENABLED 1 \n" }; 
    const unsigned num_default_macros  = sizeof(default_macros)/sizeof(const char*);

    App->programs->Load("default", "Assets/Shaders/default.vs", "Assets/Shaders/default.fs", default_macros, num_default_macros, nullptr, 0);

    const char* macros[]		  = { "#define BLOOM 1 \n", "#define GAMMA 1\n" }; 
    const unsigned num_macros     = sizeof(macros)/sizeof(const char*);

    App->programs->Load("postprocess", "Assets/Shaders/postprocess.vs", "Assets/Shaders/postprocess.fs", macros, num_macros, nullptr, 0);
    App->programs->Load("skybox", "Assets/Shaders/skybox.vs", "Assets/Shaders/skybox.fs", nullptr, 0, nullptr, 0);
    App->programs->Load("particles", "Assets/Shaders/particles.vs", "Assets/Shaders/particles.fs", nullptr, 0, nullptr, 0);
    App->programs->Load("trails", "Assets/Shaders/trails.vs", "Assets/Shaders/trails.fs", nullptr, 0, nullptr, 0);
    App->programs->Load("shadow", "Assets/Shaders/shadow.vs", "Assets/Shaders/shadow.fs", nullptr, 0, nullptr, 0);

    const char* gaussian_macros[]       = { "#define HORIZONTAL 1 \n" }; 
    const unsigned num_gaussian_macros  = sizeof(gaussian_macros)/sizeof(const char*);

    App->programs->Load("gaussian", "Assets/Shaders/postprocess.vs", "Assets/Shaders/gaussian.fs", gaussian_macros, num_gaussian_macros, nullptr, 0);
    App->programs->Load("chebyshev", "Assets/Shaders/postprocess.vs", "Assets/Shaders/chebyshev.fs", nullptr, 0, nullptr, 0);

    const char* bloom_macros[]       = { "#define MSAA 1 \n" }; 
    const unsigned num_bloom_macros  = sizeof(bloom_macros)/sizeof(const char*);

    App->programs->Load("bloom", "Assets/Shaders/postprocess.vs", "Assets/Shaders/bloom.fs", bloom_macros, num_bloom_macros, nullptr, 0);

    const char* show_uv_macros[]       = { "#define TEXCOORD1 1 \n" }; 
    const unsigned num_uv_macros  = sizeof(show_uv_macros)/sizeof(const char*);
    App->programs->Load("show_uvs", "Assets/Shaders/show_uvs.vs", "Assets/Shaders/show_uvs.fs", show_uv_macros, num_uv_macros, nullptr, 0);
    App->programs->Load("selection", "Assets/Shaders/selection.vs", "Assets/Shaders/selection.fs", nullptr, 0, nullptr, 0);
    App->programs->Load("color", "Assets/Shaders/color.vs", "Assets/Shaders/color.fs", nullptr, 0, nullptr, 0);
    App->programs->Load("grid", "Assets/Shaders/gridVS.glsl", "Assets/Shaders/gridPS.glsl", nullptr, 0, nullptr, 0);
    App->programs->Load("showtexture", "Assets/Shaders/fullscreenVS.glsl", "Assets/Shaders/fullscreenTextureFS.glsl", nullptr, 0, nullptr, 0);
    App->programs->Load("convert_texture", "Assets/Shaders/fullscreenVS.glsl", "Assets/Shaders/convertMetallicTextureFS.glsl", nullptr, 0, nullptr, 0);
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
    DebugDrawTangentSpace();
    DebugDrawAnimation();
    DebugDrawParticles();
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

void DebugDrawParticles(ComponentParticleSystem* particles)
{
	float4x4 transform = particles->GetGameObject()->GetGlobalTransformation();
	switch (particles->shape.type)
	{
        case ComponentParticleSystem::Circle:
            dd::circle(transform.TranslatePart(), transform.Col3(1), dd::colors::Gray, particles->shape.radius, 20);
            break;
        case ComponentParticleSystem::Cone:
            {
                float base_radius = tan(particles->shape.angle) + particles->shape.radius;
                dd::cone(transform.TranslatePart(), transform.Col3(1), dd::colors::Gray, base_radius, particles->shape.radius, 20);
                break;
            }
    }
}

void ModuleRenderer::DebugDrawParticles()
{
    for(NodeList::iterator it = transparent_nodes.begin(), end = transparent_nodes.end(); it != end; ++it)
    {
        if(it->particles)
        {
            ::DebugDrawParticles(it->particles);
        }
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

void ModuleRenderer::Postprocess(unsigned screen_texture, unsigned fbo, unsigned width, unsigned height)
{
    // \todo: apply bloom gaussian

	bool msaa  = App->hints->GetBoolValue(ModuleHints::ENABLE_MSAA);

    GenerateBloomFBO(width, height);

    // bloom
    glBindVertexArray(post_vao);
    glBindFramebuffer(GL_FRAMEBUFFER, bloom_fbo);
    {
        glClear(GL_COLOR_BUFFER_BIT);
        App->programs->UseProgram("bloom", msaa ? 1 : 0);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(msaa ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D, screen_texture);
        glUniform1i(App->programs->GetUniformLocation("image"), 0); 
        glDrawArrays(GL_TRIANGLES, 0, 6); 
    }


	
    float weights[] = { 0.198596f,	0.175713f,	0.121703f,	0.065984f,	0.028002f,	0.0093f };

    // blur
    glBindVertexArray(post_vao);
    glBindFramebuffer(GL_FRAMEBUFFER, bloom_blur_fbo_0);
    {
        glClear(GL_COLOR_BUFFER_BIT);
        App->programs->UseProgram("gaussian", 1);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, bloom_tex);
        glUniform1i(App->programs->GetUniformLocation("image"), 0); 
        glUniform1fv(App->programs->GetUniformLocation("weight"), sizeof(weights)/sizeof(float), weights);
        glDrawArrays(GL_TRIANGLES, 0, 6); 
    }

    glBindVertexArray(post_vao);
    glBindFramebuffer(GL_FRAMEBUFFER, bloom_blur_fbo_1);
    {
        glClear(GL_COLOR_BUFFER_BIT);
        App->programs->UseProgram("gaussian", 0);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, bloom_blur_tex_0);
        glUniform1i(App->programs->GetUniformLocation("image"), 0); 
        glUniform1fv(App->programs->GetUniformLocation("weight"), sizeof(weights)/sizeof(float), weights);
        glDrawArrays(GL_TRIANGLES, 0, 6); 
    }

    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    int flags = App->hints->GetBoolValue(ModuleHints::ENABLE_BLOOM) ? 1 : 0;
    flags = flags | (App->hints->GetBoolValue(ModuleHints::ENABLE_GAMMA) ? 1 << 1 : 0);

    App->programs->UseProgram("postprocess", flags);

    unsigned indices[NUM_POSPROCESS_SUBROUTINES];

    indices[TONEMAP_LOCATION] = App->hints->GetIntValue(ModuleHints::TONEMAPPING);

    glUniformSubroutinesuiv(GL_FRAGMENT_SHADER, sizeof(indices)/sizeof(unsigned), indices);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, color_tex);
    glUniform1i(SCREEN_TEXTURE_LOCATION, 0); 

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, bloom_blur_tex_1);
    glUniform1i(BLOOM_TEXTURE_LOCATION, 1); 

    glBindVertexArray(post_vao);
    glDrawArrays(GL_TRIANGLES, 0, 6); 

    glBindVertexArray(0);

    App->programs->UnuseProgram();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void ModuleRenderer::BlurShadow(uint index)
{
    glBindVertexArray(post_vao);

    glBindFramebuffer(GL_FRAMEBUFFER, cascades[index].sq_fbo);
    {
        App->programs->UseProgram("chebyshev", 0);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, cascades[index].tex);
        glUniform1i(App->programs->GetUniformLocation("image"), 0); 
        glDrawArrays(GL_TRIANGLES, 0, 6); 
    }

    float weights[] = { 0.38774f,	0.24477f, 0.06136f };


    glBindFramebuffer(GL_FRAMEBUFFER, cascades[index].blur_fbo_0);
    {
        App->programs->UseProgram("gaussian", 1);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, cascades[index].sq_tex);
        glUniform1i(App->programs->GetUniformLocation("image"), 0); 
        glUniform1fv(App->programs->GetUniformLocation("weight"), sizeof(weights)/sizeof(float), weights);
        glDrawArrays(GL_TRIANGLES, 0, 6); 
    }

    glBindFramebuffer(GL_FRAMEBUFFER, cascades[index].blur_fbo_1);
    {
        App->programs->UseProgram("gaussian", 0);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, cascades[index].blur_tex_0);
        glUniform1i(App->programs->GetUniformLocation("image"), 0); 
        glUniform1fv(App->programs->GetUniformLocation("weight"), sizeof(weights)/sizeof(float), weights);
        glDrawArrays(GL_TRIANGLES, 0, 6); 
        glDrawArrays(GL_TRIANGLES, 0, 6); 
    }


    glBindVertexArray(0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    App->programs->UnuseProgram();
}

void ModuleRenderer::ComputeDirLightShadowVolume(ComponentCamera* camera, uint index)
{
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
        cascades[index].frustum.nearPlaneDistance = cascades[index].near_distance;
        cascades[index].frustum.farPlaneDistance = cascades[index].far_distance;

        CalcLightCameraBBox(light_rotation, camera, cascades[index].near_distance, cascades[index].far_distance, cascades[index].aabb);
        CalcLightObjectsBBox(light_rotation, cascades[index].aabb, cascades[index].casters);

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


        cascades[index].proj = SetOrtho(-frustum.orthographicWidth/2, frustum.orthographicWidth/2,
                -frustum.orthographicHeight/2, frustum.orthographicHeight/2, 
                frustum.nearPlaneDistance, frustum.farPlaneDistance);
		cascades[index].proj = frustum.ProjectionMatrix();
		cascades[index].view = frustum.ViewMatrix();
    }
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
    projection.SetCol(0, math::float4(a, 0.0f, 0.0f, 0.0f));
    projection.SetCol(1, math::float4(0.0f , b, 0.0f, 0.0f));
    projection.SetCol(2, math::float4(0.0f , 0.0f, c, 0.0f));
    projection.SetCol(3, math::float4(tx, ty, tz, 1.0f));

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

void ModuleRenderer::CalcLightObjectsBBox(const Quat& light_rotation, AABB& aabb, NodeList& casters)
{
    float4x4 light_mat = light_rotation.Inverted().ToFloat3x3();
    AABB camera_aabb = aabb;
	camera_aabb.maxPoint.z = FLOAT_INF;
    aabb.SetNegativeInfinity();

    for(NodeList::iterator it = opaque_nodes.begin(), end = opaque_nodes.end(); it != end; ++it)
    {
        const TRenderInfo& render_info = *it;

        if(render_info.mesh->CastShadows())
        {
			render_info.go->RecalculateBoundingBox();
            AABB bbox = render_info.go->GetLocalBBox();

            if(bbox.IsFinite())
            {
                float4x4 transform = light_mat*render_info.go->GetGlobalTransformation();

                bbox.TransformAsAABB(transform);

                bbox.Scale(bbox.CenterPoint(), 1.25f);

                if(camera_aabb.Intersects(bbox))
                {
                    aabb.Enclose(bbox);
                    casters.push_back(render_info);
                }
            }
        }
    }

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
            
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, map.tex, 0);

            glDrawBuffer(GL_NONE);

            glBindFramebuffer(GL_FRAMEBUFFER, map.sq_fbo);
            glGenTextures(1, &map.sq_tex);
            glBindTexture(GL_TEXTURE_2D, map.sq_tex);

            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, width, height, 0, GL_RGB, GL_FLOAT, 0);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, map.sq_tex, 0);

            glDrawBuffer(GL_COLOR_ATTACHMENT0);

            glBindFramebuffer(GL_FRAMEBUFFER, map.blur_fbo_0);
            glGenTextures(1, &map.blur_tex_0);
            glBindTexture(GL_TEXTURE_2D, map.blur_tex_0);

            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, width, height, 0, GL_RGB, GL_FLOAT, 0);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, map.blur_tex_0, 0);

            glDrawBuffer(GL_COLOR_ATTACHMENT0);

            glBindFramebuffer(GL_FRAMEBUFFER, map.blur_fbo_1);
            glGenTextures(1, &map.blur_tex_1);
            glBindTexture(GL_TEXTURE_2D, map.blur_tex_1);

            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, width, height, 0, GL_RGB, GL_FLOAT, 0);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
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

void ModuleRenderer::GenerateBloomFBO(unsigned width, unsigned height)
{
    if(width != bloom_width || height != bloom_height)
    {
        if(bloom_blur_tex_0 != 0)
        {
            glDeleteTextures(1, &bloom_blur_tex_0);
        }

        if(bloom_blur_tex_1 != 0)
        {
            glDeleteTextures(1, &bloom_blur_tex_1);
        }

        if(width != 0 && height != 0)
        {
            // split bloom color
            glBindFramebuffer(GL_FRAMEBUFFER, bloom_fbo);
            glGenTextures(1, &color_tex);
            glBindTexture(GL_TEXTURE_2D, color_tex);

            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, width, height, 0, GL_RGB, GL_FLOAT, 0);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, color_tex, 0);

            glGenTextures(1, &bloom_tex);
            glBindTexture(GL_TEXTURE_2D, bloom_tex);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, width, height, 0, GL_RGB, GL_FLOAT, 0);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, bloom_tex, 0);

            unsigned int attachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
            glDrawBuffers(2, attachments);  

            // blur 

            glBindFramebuffer(GL_FRAMEBUFFER, bloom_blur_fbo_0);
            glGenTextures(1, &bloom_blur_tex_0);
            glBindTexture(GL_TEXTURE_2D, bloom_blur_tex_0);

            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, width, height, 0, GL_RGB, GL_FLOAT, 0);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, bloom_blur_tex_0, 0);

            glDrawBuffer(GL_COLOR_ATTACHMENT0);

            glBindFramebuffer(GL_FRAMEBUFFER, bloom_blur_fbo_1);
            glGenTextures(1, &bloom_blur_tex_1);
            glBindTexture(GL_TEXTURE_2D, bloom_blur_tex_1);

            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, width, height, 0, GL_RGB, GL_FLOAT, 0);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, bloom_blur_tex_1, 0);

            glDrawBuffer(GL_COLOR_ATTACHMENT0);
        }

		bloom_width  = width;
		bloom_height = height;
    }
}
