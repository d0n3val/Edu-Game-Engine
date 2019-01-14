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
    sky_cubemap = SOIL_load_OGL_single_cubemap("Assets/Textures/PBR/test2EnvHDR.dds", face_order, 3, 0, SOIL_FLAG_DDS_LOAD_DIRECT);

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

}

void ModuleRenderer::Draw(ComponentCamera* camera, unsigned fbo, unsigned width, unsigned height)
{
	opaque_nodes.clear();
    transparent_nodes.clear();

	//if (camera->frustum_culling == true)
	//{
		//App->level->quadtree.CollectIntersections(draw_nodes, camera->frustum);
	//}
	//else
	//{
		//App->level->quadtree.CollectObjects(draw_nodes);
	//}

	float4x4 proj   = camera->GetProjectionMatrix();	
	float4x4 view   = camera->GetViewMatrix();
    float3 view_pos = view.RotatePart().Transposed().Transform(-view.TranslatePart());

    CollectObjects(view_pos, App->level->GetRoot());

    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    glViewport(0, 0, width, height);
	glClearColor(camera->background.r, camera->background.g, camera->background.b, camera->background.a);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    DrawNodes(&ModuleRenderer::DrawMeshColor, proj, view, view_pos);
    DrawSkybox(proj, view);

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
    ComponentMesh* mesh = go->FindFirstComponent<ComponentMesh>();
    ComponentMaterial* material = go->FindFirstComponent<ComponentMaterial>();

    if(mesh != nullptr && material != nullptr && mesh->GetVisible())
    {
        AABB bbox;
        mesh->GetBoundingBox(bbox);

        TRenderInfo render;
        render.name     = go->name.c_str();
        render.mesh     = mesh;
        render.material = material;
        render.transform = go->GetGlobalTransformation();
        render.distance = (go->global_bbox.CenterPoint()-camera_pos).LengthSq();

        if(material->RenderMode() == ComponentMaterial::RENDER_OPAQUE)
        {
            NodeList::iterator it = std::lower_bound(opaque_nodes.begin(), opaque_nodes.end(), render.distance, TNearestMesh());

            opaque_nodes.insert(it, render);
        }
        else
        {
            NodeList::iterator it = std::lower_bound(transparent_nodes.begin(), transparent_nodes.end(), render.distance, TFarthestMesh());

            transparent_nodes.insert(it, render);
        }
    }

    for(std::list<GameObject*>::iterator lIt = go->childs.begin(), lEnd = go->childs.end(); lIt != lEnd; ++lIt)
    {
        CollectObjects(camera_pos, *lIt);
    }
}

void ModuleRenderer::DrawNodes(void (ModuleRenderer::*drawer)(const TRenderInfo&,
							   const float4x4&, const float4x4&, const float3&), 
                               const float4x4& projection, const float4x4& view, 
                               const float3& view_pos)
{
	for(NodeList::iterator it = opaque_nodes.begin(), end = opaque_nodes.end(); it != end; ++it)
	{
        (this->*drawer)(*it, projection, view, view_pos);
    }

	for(NodeList::iterator it = transparent_nodes.begin(), end = transparent_nodes.end(); it != end; ++it)
	{
        (this->*drawer)(*it, projection, view, view_pos);
    }

    App->programs->UnuseProgram();
}

void ModuleRenderer::DrawMeshColor(const TRenderInfo& render_info, const float4x4& projection, const float4x4& view, const float3& view_pos)
{    
    const ResourceMesh* mesh_res    = render_info.mesh->GetResource();
    const ResourceMaterial* mat_res = render_info.material->GetResource();

    if(mat_res != nullptr && mesh_res != nullptr)
    {
        App->programs->UseProgram("default", 0);

        UpdateMaterialUniform(mat_res);
        UpdateLightUniform();

        glUniformMatrix4fv(App->programs->GetUniformLocation("proj"), 1, GL_TRUE, reinterpret_cast<const float*>(&projection));
        glUniformMatrix4fv(App->programs->GetUniformLocation("view"), 1, GL_TRUE, reinterpret_cast<const float*>(&view));
        glUniformMatrix4fv(App->programs->GetUniformLocation("model"), 1, GL_TRUE, reinterpret_cast<const float*>(&render_info.transform));
        glUniform3f(App->programs->GetUniformLocation("view_pos"), view_pos.x, view_pos.y, view_pos.z);

        glBindVertexArray(mesh_res->vao);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh_res->ibo);
        glDrawElements(GL_TRIANGLES, mesh_res->num_indices, GL_UNSIGNED_INT, nullptr);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        glBindVertexArray(0);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, 0);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
}

void ModuleRenderer::LoadDefaultShaders()
{
    App->programs->Load("default", "Assets/Shaders/default.vs", "Assets/Shaders/default.fs", nullptr, 0, nullptr, 0);

    const char* macros[]		  = { "#define MSAA 1 \n" }; 
    const unsigned num_macros     = sizeof(macros)/sizeof(const char*);

    App->programs->Load("postprocess", "Assets/Shaders/postprocess.vs", "Assets/Shaders/postprocess.fs", macros, num_macros, nullptr, 0);
    App->programs->Load("skybox", "Assets/Shaders/skybox.vs", "Assets/Shaders/skybox.fs", nullptr, 0, nullptr, 0);
}

void ModuleRenderer::UpdateMaterialUniform(const ResourceMaterial* material) const
{
    static const unsigned MATERIAL_LOCATION = 0;

    const ResourceTexture* specular  = material->GetTextureRes(ResourceMaterial::TextureSpecular);
    const ResourceTexture* diffuse   = material->GetTextureRes(ResourceMaterial::TextureDiffuse);
    const ResourceTexture* occlusion = material->GetTextureRes(ResourceMaterial::TextureOcclusion);
    const ResourceTexture* emissive  = material->GetTextureRes(ResourceMaterial::TextureEmissive);
    const ResourceTexture* normal    = material->GetTextureRes(ResourceMaterial::TextureNormal);

    unsigned diffuse_id   = diffuse ? diffuse->GetID() : App->resources->GetWhiteFallback()->GetID();

    unsigned specular_id  = specular && App->hints->GetBoolValue(ModuleHints::ENABLE_SPECULAR_MAPPING) ? 
                            specular->GetID() : App->resources->GetWhiteFallback()->GetID();

    unsigned occlusion_id = occlusion ? occlusion->GetID() : App->resources->GetWhiteFallback()->GetID();
    unsigned emissive_id  = emissive ? emissive->GetID() : App->resources->GetWhiteFallback()->GetID();
    unsigned normal_id    = normal && App->hints->GetBoolValue(ModuleHints::ENABLE_NORMAL_MAPPING) ? normal->GetID() : 0;

    float4 diffuse_color  = material->GetDiffuseColor();
    float3 specular_color = specular && App->hints->GetBoolValue(ModuleHints::ENABLE_SPECULAR_MAPPING) ? float3(1.0f) : material->GetSpecularColor();
    float3 emissive_color = emissive ? float3(1.0f) : material->GetEmissiveColor();
    float shininess	      = /*specular ? 1.0f : */ material->GetShininess();

    glUniform1f(SHININESS_LOC, shininess);

    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, normal_id);
    glUniform1i(NORMAL_MAP_LOC, 4);

    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, emissive_id);
    glUniform1i(EMISSIVE_MAP_LOC, 3);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, occlusion_id);
    glUniform1i(OCCLUSION_MAP_LOC, 2);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, specular_id);
    glUniform1i(SPECULAR_MAP_LOC, 1);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, diffuse_id);
    glUniform1i(DIFFUSE_MAP_LOC, 0);

    glUniform4fv(DIFFUSE_COLOR_LOC, 1, (const float*)&diffuse_color);
    glUniform3fv(SPECULAR_COLOR_LOC, 1, (const float*)&specular_color);
    glUniform3fv(EMISSIVE_COLOR_LOC, 1, (const float*)&emissive_color);

    glUniform1f(AMBIENT_CONSTANT_LOC, material->GetKAmbient());
    glUniform1f(DIFFUSE_CONSTANT_LOC, material->GetKDiffuse());
    glUniform1f(SPECULAR_CONSTANT_LOC, material->GetKSpecular());

    unsigned indices[NUM_SUBROUTINE_UNIFORMS];

    if(normal && App->hints->GetBoolValue(ModuleHints::ENABLE_NORMAL_MAPPING))
    {
        indices[GET_NORMAL_LOCATION] = GET_NORMAL_FROM_TEXTURE;
    }
    else
    {
        indices[GET_NORMAL_LOCATION] = GET_NORMAL_FROM_VERTEX;
    }

    if(App->hints->GetBoolValue(ModuleHints::ENABLE_FRESNEL))
    {
        indices[GET_FRESNEL_LOCATION] = GET_FRESNEL_SCHLICK;
    }
    else
    {
        indices[GET_FRESNEL_LOCATION] = GET_NO_FRESNEL;
    }

    glUniformSubroutinesuiv(GL_FRAGMENT_SHADER, sizeof(indices)/sizeof(unsigned), indices);
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
    float metric_proportion = App->hints->GetFloatValue(ModuleHints::METRIC_PROPORTION);
    unsigned total_tris = 0;

    for(NodeList::iterator it = opaque_nodes.begin(), end = opaque_nodes.end(); it != end; ++it)
    {
        const ResourceMesh* mesh = it->mesh->GetResource();

        if(mesh)
        {
            total_tris += mesh->num_indices/3;
        }
#if 0
        if(mesh && (mesh->attribs & ResourceMesh::ATTRIB_TANGENTS) != 0 && (mesh->attribs& ResourceMesh::ATTRIB_NORMALS))
        {
            for(unsigned i = 0, count = mesh->num_vertices; i < count; ++i)
            {
                float3 position  = it->transform.TransformPos(mesh->src_vertices[i]);
                float3 normal    = it->transform.TransformDir(mesh->src_normals[i]);
                float3 tangent   = it->transform.TransformDir(mesh->src_tangents[i]);
                float3 bitangent = normal.Cross(tangent);

                float4x4 tbn(float4(tangent, 0.0f), float4(bitangent, 0.0f), float4(normal, 0.0f), float4(position, 1.0f));

                dd::axisTriad(tbn, metric_proportion*0.1f*0.1f, metric_proportion*0.1f, 0);
            }
        }
#endif
    }

    for(NodeList::iterator it = transparent_nodes.begin(), end = transparent_nodes.end(); it != end; ++it)
    {
        const ResourceMesh* mesh = it->mesh->GetResource();

        if(mesh)
        {
            total_tris += mesh->num_indices/3;
        }
#if 0
        if(mesh && (mesh->attribs & ResourceMesh::ATTRIB_TANGENTS) != 0 && (mesh->attribs& ResourceMesh::ATTRIB_NORMALS))
        {
            for(unsigned i = 0, count = mesh->num_vertices; i < count; ++i)
            {
                float3 position  = it->transform.TransformPos(mesh->src_vertices[i]);
                float3 normal    = it->transform.TransformDir(mesh->src_normals[i]);
                float3 tangent   = it->transform.TransformDir(mesh->src_tangents[i]);
                float3 bitangent = normal.Cross(tangent);

                float4x4 tbn(float4(tangent, 0.0f), float4(bitangent, 0.0f), float4(normal, 0.0f), float4(position, 1.0f));

                dd::axisTriad(tbn, metric_proportion*0.1f*0.1f, metric_proportion*0.1f, 0);
            }
        }
#endif
    }

    ImGui::Text("Total tris %d ", total_tris);
}

void ModuleRenderer::Postprocess(unsigned screen_texture, unsigned fbo, unsigned width, unsigned height)
{
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    bool msaa = App->hints->GetBoolValue(ModuleHints::ENABLE_MSAA);

    App->programs->UseProgram("postprocess", msaa ? 1 : 0);

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

