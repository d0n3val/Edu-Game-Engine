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

    glUniform1f(App->programs->GetUniformLocation("material.shininess"), shininess);

    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, emissive_id);
    glUniform1i(App->programs->GetUniformLocation("material.emissive_map"), 3);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, occlusion_id);
    glUniform1i(App->programs->GetUniformLocation("material.occlusion_map"), 2);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, specular_id);
    glUniform1i(App->programs->GetUniformLocation("material.specular_map"), 1);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, diffuse_id);
    glUniform1i(App->programs->GetUniformLocation("material.diffuse_map"), 0);

    glUniform4fv(App->programs->GetUniformLocation("material.diffuse_color"), 1, (const float*)&diffuse_color);
    glUniform3fv(App->programs->GetUniformLocation("material.specular_color"), 1, (const float*)&specular_color);
    glUniform3fv(App->programs->GetUniformLocation("material.emissive_color"), 1, (const float*)&emissive_color);

    glUniform1f(App->programs->GetUniformLocation("material.k_ambient"), material->GetKAmbient());
    glUniform1f(App->programs->GetUniformLocation("material.k_diffuse"), material->GetKDiffuse());
    glUniform1f(App->programs->GetUniformLocation("material.k_specular"), material->GetKSpecular());
}

void ModuleRenderer::UpdateLightUniform() const
{
    const AmbientLight* ambient = App->level->GetAmbientLight();
    const DirLight* directional = App->level->GetDirLight();

    float3 dir = directional->GetDir();

    glUniform3fv(App->programs->GetUniformLocation("lights.ambient.color"), 1, (const float*)&ambient->GetColor());
    glUniform3fv(App->programs->GetUniformLocation("lights.directional.color"), 1, (const float*)&directional->GetColor());
    glUniform3fv(App->programs->GetUniformLocation("lights.directional.dir"), 1, (const float*)&dir);
    glUniform1ui(App->programs->GetUniformLocation("lights.num_point"), 0);
}

