#include "Globals.h"
#include "ModuleRenderer.h"
#include "ModulePrograms.h"
#include "ModuleLevelManager.h"
#include "ModuleResources.h"
#include "ModuleHints.h"

#include "DefaultShaderLocations.h"
#include "PostprocessShaderLocations.h"

#include "GameObject.h"

#include "ComponentMesh.h"
#include "ComponentMaterial.h"
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

	int width, height, channels;
	unsigned char* data = SOIL_load_image("Assets/Textures/PBR/BarnaEnvHDR.dds", &width, &height, &channels, SOIL_LOAD_AUTO);
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

    glGenFramebuffers(1, &shadow_fbo);
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

    if(shadow_fbo != 0)
    {
        glDeleteFramebuffers(1, &shadow_fbo);
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

    float4x4 light_view, light_proj;
    ComputeDirLightViewProj(light_view, light_proj);

    GenerateShadowFBO(width, height);

    glViewport(0, 0, width, height);
    glBindFramebuffer(GL_FRAMEBUFFER, shadow_fbo);
	glClear(GL_DEPTH_BUFFER_BIT);

    App->programs->UseProgram("shadow", 1);
    glUniformMatrix4fv(App->programs->GetUniformLocation("proj"), 1, GL_TRUE, reinterpret_cast<const float*>(&light_proj));
    glUniformMatrix4fv(App->programs->GetUniformLocation("view"), 1, GL_TRUE, reinterpret_cast<const float*>(&light_view));
    DrawNodes(&ModuleRenderer::DrawShadow);

    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    // Set camera uniforms shared for all
    App->programs->UseProgram("default", 1);

    glUniformMatrix4fv(App->programs->GetUniformLocation("proj"), 1, GL_TRUE, reinterpret_cast<const float*>(&proj));
    glUniformMatrix4fv(App->programs->GetUniformLocation("view"), 1, GL_TRUE, reinterpret_cast<const float*>(&view));
    glUniform3f(App->programs->GetUniformLocation("view_pos"), view_pos.x, view_pos.y, view_pos.z);

    glUniformMatrix4fv(App->programs->GetUniformLocation("light_proj"), 1, GL_TRUE, reinterpret_cast<const float*>(&light_proj));
    glUniformMatrix4fv(App->programs->GetUniformLocation("light_view"), 1, GL_TRUE, reinterpret_cast<const float*>(&light_view));
    glUniform1f(App->programs->GetUniformLocation("shadow_bias"), 0.001f); //hints->GetFloatValue(Hint::SHADOW_BIAS));
    glActiveTexture(GL_TEXTURE8);
	glBindTexture(GL_TEXTURE_2D, shadow_tex);
	glUniform1i(App->programs->GetUniformLocation("shadow_map"), 8);

    App->programs->UnuseProgram();

    App->programs->UseProgram("particles", 0);
    glUniformMatrix4fv(App->programs->GetUniformLocation("proj"), 1, GL_TRUE, reinterpret_cast<const float*>(&proj));
    glUniformMatrix4fv(App->programs->GetUniformLocation("view"), 1, GL_TRUE, reinterpret_cast<const float*>(&view));
    App->programs->UnuseProgram();

    App->programs->UseProgram("trails", 0);
    glUniformMatrix4fv(App->programs->GetUniformLocation("proj"), 1, GL_TRUE, reinterpret_cast<const float*>(&proj));
    glUniformMatrix4fv(App->programs->GetUniformLocation("view"), 1, GL_TRUE, reinterpret_cast<const float*>(&view));
    App->programs->UnuseProgram();

    DrawNodes(&ModuleRenderer::DrawColor);

    //DrawSkybox(proj, view);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
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
    ComponentMesh* mesh                = go->FindFirstComponent<ComponentMesh>();
    ComponentMaterial* material        = go->FindFirstComponent<ComponentMaterial>();
    ComponentParticleSystem* particles = go->FindFirstComponent<ComponentParticleSystem>();
    ComponentTrail* trail              = go->FindFirstComponent<ComponentTrail>();

    TRenderInfo render;
    render.name         = go->name.c_str();
    render.go           = go;

    if(mesh != nullptr && material != nullptr && mesh->GetVisible())
    {
        render.distance = (go->global_bbox.CenterPoint()-camera_pos).LengthSq();
        render.mesh     = mesh;

        if(material->RenderMode() == ComponentMaterial::RENDER_OPAQUE)
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
    else if(particles != nullptr)
    {
        render.distance = (go->GetGlobalPosition()-camera_pos).LengthSq();
        render.particles= particles;
        render.layer = particles->GetLayer();

        NodeList::iterator it = std::lower_bound(transparent_nodes.begin(), transparent_nodes.end(), render, TFarthestMesh());
        transparent_nodes.insert(it, render);
    }
    else if(trail != nullptr)
    {
        render.distance = (go->GetGlobalPosition()-camera_pos).LengthSq();
        render.trail = trail;

        NodeList::iterator it = std::lower_bound(transparent_nodes.begin(), transparent_nodes.end(), render, TFarthestMesh());
        transparent_nodes.insert(it, render);
    }

    for(std::list<GameObject*>::iterator lIt = go->childs.begin(), lEnd = go->childs.end(); lIt != lEnd; ++lIt)
    {
        CollectObjects(camera_pos, *lIt);
    }
}

void ModuleRenderer::DrawNodes(void (ModuleRenderer::*drawer)(const TRenderInfo&))
{
	for(NodeList::iterator it = opaque_nodes.begin(), end = opaque_nodes.end(); it != end; ++it)
	{
        (this->*drawer)(*it);
    }

	for(NodeList::iterator it = transparent_nodes.begin(), end = transparent_nodes.end(); it != end; ++it)
	{
        (this->*drawer)(*it);
    }

    App->programs->UnuseProgram();
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

void ModuleRenderer::DrawMeshColor(const ComponentMesh* mesh)
{
    App->programs->UseProgram("default", 1);

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

    const char* macros[]		  = { "#define MSAA 1 \n", "#define GAMMA 1\n" }; 
    const unsigned num_macros     = sizeof(macros)/sizeof(const char*);

    App->programs->Load("postprocess", "Assets/Shaders/postprocess.vs", "Assets/Shaders/postprocess.fs", macros, num_macros, nullptr, 0);
    App->programs->Load("skybox", "Assets/Shaders/skybox.vs", "Assets/Shaders/skybox.fs", nullptr, 0, nullptr, 0);
    App->programs->Load("particles", "Assets/Shaders/particles.vs", "Assets/Shaders/particles.fs", nullptr, 0, nullptr, 0);
    App->programs->Load("trails", "Assets/Shaders/trails.vs", "Assets/Shaders/trails.fs", nullptr, 0, nullptr, 0);
    App->programs->Load("shadow", "Assets/Shaders/shadow.vs", "Assets/Shaders/shadow.fs", nullptr, 0, nullptr, 0);
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
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	bool msaa  = App->hints->GetBoolValue(ModuleHints::ENABLE_MSAA);
    int flags  = msaa ? 1 << 0 : 0;
    flags      = flags | (App->hints->GetBoolValue(ModuleHints::ENABLE_GAMMA) ? 1 << 1 : 0);

    App->programs->UseProgram("postprocess", flags);

    unsigned indices[NUM_POSPROCESS_SUBROUTINES];

    indices[TONEMAP_LOCATION] = App->hints->GetIntValue(ModuleHints::TONEMAPPING);

    glUniformSubroutinesuiv(GL_FRAGMENT_SHADER, sizeof(indices)/sizeof(unsigned), indices);

    glActiveTexture(GL_TEXTURE0);

    if(msaa)
    {
        glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, screen_texture);
    }
    else
    {
        glBindTexture(GL_TEXTURE_2D, screen_texture);
    }

    glUniform1i(SCREEN_TEXTURE_LOCATION, 0); 
    glUniform1i(VIEWPORT_WIDTH, width); 
    glUniform1i(VIEWPORT_HEIGHT, height); 

    glBindVertexArray(post_vao);

    glDrawArrays(GL_TRIANGLES, 0, 6); 

    glBindVertexArray(0);


    if(msaa)
    {
        glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
    }
    else
    {
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    App->programs->UnuseProgram();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void ModuleRenderer::ComputeDirLightViewProj(float4x4& view, float4x4& proj)
{
    const DirLight* light = App->level->GetDirLight();

	if (light == nullptr)
    {
        proj = float4x4::identity;
        view = float4x4::identity;
    }
    else
	{
        AABB aabb;
        aabb.SetNegativeInfinity();

        float3 front        = light->GetDir();
		float3 up           = light->GetUp();
		Quat light_rotation = Quat::LookAt(-float3::unitZ, front, float3::unitY, up); 

        CalcLightSpaceBBox(light_rotation, aabb);

        float3 center = aabb.CenterPoint();

        Frustum frustum; 
        frustum.type               = FrustumType::OrthographicFrustum;
        frustum.pos                = light_rotation.Transform(float3(center.x, center.y, aabb.maxPoint.z));
        frustum.front              = front;
        frustum.up                 = up;
        frustum.nearPlaneDistance  = 0.0f;
        frustum.farPlaneDistance   = (aabb.maxPoint.z - aabb.minPoint.z);
        frustum.orthographicWidth  = (aabb.maxPoint.x - aabb.minPoint.x);
        frustum.orthographicHeight = (aabb.maxPoint.y - aabb.minPoint.y);

        proj = frustum.ProjectionMatrix();
        view = frustum.ViewMatrix();

        math::float3 p[8];
        frustum.GetCornerPoints(p);
        std::swap(p[2], p[5]);
        std::swap(p[3], p[4]);
        std::swap(p[4], p[5]);
        std::swap(p[6], p[7]);
        dd::box(p, dd::colors::Green);

        float4x4 inverted = view.Inverted();
        dd::axisTriad(inverted, 10.0f, 100.0f, 0, false);
    }
}

void ModuleRenderer::CalcLightSpaceBBox(const Quat& light_rotation, AABB& aabb)
{
    float4x4 light_mat = light_rotation.Inverted().ToFloat3x3();

    for(NodeList::iterator it = opaque_nodes.begin(), end = opaque_nodes.end(); it != end; ++it)
    {
        const TRenderInfo& render_info = *it;
        ComponentMaterial* material = render_info.go->FindFirstComponent<ComponentMaterial>();

        if(material && material->CastShadows())
        {
			render_info.go->RecalculateBoundingBox();
            AABB bbox = render_info.go->GetLocalBBox();

            if(bbox.IsFinite())
            {
                float4x4 transform = light_mat*render_info.go->GetGlobalTransformation();

                bbox.TransformAsAABB(transform);
                bbox.Scale(bbox.CenterPoint(), 1.25f);
                aabb.Enclose(bbox);
            }
        }
    }

    // \todo: for transparents

    // \todo: 
    // En una situación real faltaria meter objetos que, estando fuera del frustum estan dentro de los dos planos laterales de volumen ortogonal en
    // dirección a la luz. Estos objetos, a pesar de no estar en el frustum, prodrían arrojar sombras sobre otros que si lo están.
    // Ojo!!!!!!!!!!!!!!!
}

void ModuleRenderer::GenerateShadowFBO(unsigned width, unsigned height)
{
    if(width != shadow_width || height != shadow_height)
    {
        if(shadow_tex != 0)
        {
            glDeleteTextures(1, &shadow_tex);
        }

        if(width != 0 && height != 0)
        {
            glBindFramebuffer(GL_FRAMEBUFFER, shadow_fbo);
            glGenTextures(1, &shadow_tex);
            glBindTexture(GL_TEXTURE_2D, shadow_tex);

            glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT16, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadow_tex, 0);

            glDrawBuffer(GL_NONE);

            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            glBindTexture(GL_TEXTURE_2D, 0);
        }

		shadow_width  = width;
		shadow_height = height;
    }
}

