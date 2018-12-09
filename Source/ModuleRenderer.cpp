#include "Globals.h"

#include "ModuleRenderer.h"
#include "ModulePrograms.h"
#include "ModuleLevelManager.h"
#include "ModuleResources.h"
#include "ModuleHints.h"

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

#include "Application.h"

#include "OpenGL.h"

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
	draw_nodes.clear();

	//if (camera->frustum_culling == true)
	//{
		//App->level->quadtree.CollectIntersections(draw_nodes, camera->frustum);
	//}
	//else
	//{
		//App->level->quadtree.CollectObjects(draw_nodes);
	//}

    CollectObjects(App->level->GetRoot());

    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    glViewport(0, 0, width, height);
	glClearColor(camera->background.r, camera->background.g, camera->background.b, camera->background.a);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	float4x4 proj = camera->GetOpenGLProjectionMatrix();
	float4x4 view = camera->GetOpenGLViewMatrix();

    DrawNodes(&ModuleRenderer::DrawMeshColor, proj, view);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void ModuleRenderer::CollectObjects( GameObject* go)
{
    draw_nodes.push_back(go);

    for(std::list<GameObject*>::iterator lIt = go->childs.begin(), lEnd = go->childs.end(); lIt != lEnd; ++lIt)
    {
        CollectObjects(*lIt);
    }
}

void ModuleRenderer::DrawNodes(void (ModuleRenderer::*drawer)(const float4x4&, 
							    const ComponentMesh*, const ComponentMaterial*, const float4x4&, 
                                const float4x4&), const float4x4& projection, const float4x4& view)
{
	for(NodeList::iterator it = draw_nodes.begin(), end = draw_nodes.end(); it != end; ++it)
	{
		GameObject* node = *it;

        ComponentMesh* mesh = node->FindFirstComponent<ComponentMesh>();
        ComponentMaterial* material = node->FindFirstComponent<ComponentMaterial>();

        if(mesh != nullptr && material != nullptr)
        {
            (this->*drawer)(node->GetGlobalTransformation().Transposed(), mesh, material, projection, view);
        }
    }

    App->programs->UnuseProgram();
}

void ModuleRenderer::DrawMeshColor(const float4x4& transform, const ComponentMesh* mesh, const ComponentMaterial* material, 
                                   const float4x4& projection, const float4x4& view)
{    
    const ResourceMesh* mesh_res    = mesh->GetResource();
    const ResourceMaterial* mat_res = material->GetResource();

    if(mat_res != nullptr && mesh_res != nullptr)
    {
        App->programs->UseProgram("default", 0);

        UpdateMaterialUniform(mat_res);
        UpdateLightUniform();

        glUniformMatrix4fv(App->programs->GetUniformLocation("proj"), 1, GL_FALSE, reinterpret_cast<const float*>(&projection));
        glUniformMatrix4fv(App->programs->GetUniformLocation("view"), 1, GL_FALSE, reinterpret_cast<const float*>(&view));
        glUniformMatrix4fv(App->programs->GetUniformLocation("model"), 1, GL_FALSE, reinterpret_cast<const float*>(&transform));

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

    const ResourceTexture* specular = material->GetTextureRes(ResourceMaterial::TextureSpecular);
    const ResourceTexture* diffuse  = material->GetTextureRes(ResourceMaterial::TextureDiffuse);
    const ResourceTexture* occlusion  = material->GetTextureRes(ResourceMaterial::TextureOcclusion);
    const ResourceTexture* emissive  = material->GetTextureRes(ResourceMaterial::TextureEmissive);

    unsigned diffuse_id  = diffuse ? diffuse->GetID() : App->resources->GetWhiteFallback()->GetID();
    unsigned specular_id = specular ? specular->GetID() : App->resources->GetWhiteFallback()->GetID();
    unsigned occlusion_id = occlusion ? occlusion->GetID() : App->resources->GetWhiteFallback()->GetID();
    unsigned emissive_id = emissive ? emissive->GetID() : App->resources->GetWhiteFallback()->GetID();

    float4 diffuse_color = material->GetDiffuseColor();
    float3 specular_color = specular ? float3(1.0f) : material->GetSpecularColor();
    float3 emissive_color = emissive ? float3(1.0f) : material->GetEmissiveColor();
    float shininess = specular ? 1.0f : material->GetShininess();

    glUniform1f(MATERIAL_LOCATION+4, shininess);

    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, emissive_id);
    glUniform1i(MATERIAL_LOCATION+6, 3);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, occlusion_id);
    glUniform1i(MATERIAL_LOCATION+5, 2);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, specular_id);
    glUniform1i(MATERIAL_LOCATION+2, 1);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, diffuse_id);
    glUniform1i(MATERIAL_LOCATION, 0);

    glUniform4fv(MATERIAL_LOCATION+1, 1, (const float*)&diffuse_color);
    glUniform3fv(MATERIAL_LOCATION+3, 1, (const float*)&specular_color);
    glUniform3fv(MATERIAL_LOCATION+7, 1, (const float*)&emissive_color);

    glUniform1f(MATERIAL_LOCATION+8, material->GetKAmbient());
    glUniform1f(MATERIAL_LOCATION+9, material->GetKDiffuse());
    glUniform1f(MATERIAL_LOCATION+10, material->GetKSpecular());
}

void ModuleRenderer::UpdateLightUniform() const
{
    static const unsigned LIGHTS_LOCATION = 20;
    static const unsigned MAX_NUM_LIGHTS = 4;

    const AmbientLight* ambient = App->level->GetAmbientLight();
    const DirLight* directional = App->level->GetDirLight();

    float3 dir = directional->GetDir();

    glUniform3fv(LIGHTS_LOCATION, 1, (const float*)&ambient->GetColor());
    glUniform3fv(LIGHTS_LOCATION+1, 1, (const float*)&dir);
    glUniform3fv(LIGHTS_LOCATION+2, 1, (const float*)&directional->GetColor());

    uint num_point = min(App->level->GetNumPointLights(), MAX_NUM_LIGHTS);

    for(uint i=0; i< num_point; ++i)
    {
        glUniform3fv(LIGHTS_LOCATION+3+i*5, 1, (const float*)&App->level->GetPointLight(i)->GetPosition());
        glUniform3fv(LIGHTS_LOCATION+4+i*5, 1, (const float*)&App->level->GetPointLight(i)->GetColor());
        glUniform1f(LIGHTS_LOCATION+5+i*5, App->level->GetPointLight(i)->GetConstantAtt());
        glUniform1f(LIGHTS_LOCATION+6+i*5, App->level->GetPointLight(i)->GetLinearAtt());
        glUniform1f(LIGHTS_LOCATION+7+i*5, App->level->GetPointLight(i)->GetQuadricAtt());
    }

    glUniform1ui(LIGHTS_LOCATION+3+MAX_NUM_LIGHTS*5, num_point);
}

