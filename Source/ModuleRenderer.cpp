#include "Globals.h"

#include "ModuleRenderer.h"
#include "ModulePrograms.h"
#include "ModuleLevelManager.h"
#include "ModuleResources.h"
#include "ModuleHints.h"

#include "DefaultShaderLocations.h"

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

#include "mmgr/mmgr.h"

ModuleRenderer::ModuleRenderer() : Module("renderer")
{
}

bool ModuleRenderer::Init(Config* config /*= nullptr*/)
{
    LoadDefaultShaders();

	return true;
}
   
ModuleRenderer::~ModuleRenderer()
{
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

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
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
}

void ModuleRenderer::UpdateMaterialUniform(const ResourceMaterial* material) const
{
    static const unsigned MATERIAL_LOCATION = 0;

    const ResourceTexture* specular  = material->GetTextureRes(ResourceMaterial::TextureSpecular);
    const ResourceTexture* diffuse   = material->GetTextureRes(ResourceMaterial::TextureDiffuse);
    const ResourceTexture* occlusion = material->GetTextureRes(ResourceMaterial::TextureOcclusion);
    const ResourceTexture* emissive  = material->GetTextureRes(ResourceMaterial::TextureEmissive);

    unsigned diffuse_id   = diffuse ? diffuse->GetID() : App->resources->GetWhiteFallback()->GetID();
    unsigned specular_id  = specular ? specular->GetID() : App->resources->GetWhiteFallback()->GetID();
    unsigned occlusion_id = occlusion ? occlusion->GetID() : App->resources->GetWhiteFallback()->GetID();
    unsigned emissive_id  = emissive ? emissive->GetID() : App->resources->GetWhiteFallback()->GetID();

    float4 diffuse_color  = material->GetDiffuseColor();
    float3 specular_color = specular ? float3(1.0f) : material->GetSpecularColor();
    float3 emissive_color = emissive ? float3(1.0f) : material->GetEmissiveColor();
    float shininess	      = specular ? 1.0f : material->GetShininess();

    glUniform1f(SHININESS_LOC, shininess);

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

