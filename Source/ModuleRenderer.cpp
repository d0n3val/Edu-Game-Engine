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

void ModuleRenderer::Draw(ComponentCamera* camera, unsigned width, unsigned height)
{
	draw_nodes.clear();

	if (camera->frustum_culling == true)
	{
		App->level->quadtree.CollectIntersections(draw_nodes, camera->frustum);
	}
	else
	{
		App->level->quadtree.CollectObjects(draw_nodes);
	}


	float4x4 proj = camera->GetOpenGLProjectionMatrix();
	float4x4 view = camera->GetOpenGLViewMatrix();

    DrawNodes(&ModuleRenderer::DrawMeshColor, proj, view);
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
            (this->*drawer)(node->GetGlobalTransformation(), mesh, material, projection, view);
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

    glUniform1f(App->programs->GetUniformLocation("material.shininess"), material->GetShininess());

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, specular != nullptr ? specular->GetID() : 0);
    glUniform1i(App->programs->GetUniformLocation("material.specular_map"), 1);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, diffuse != nullptr ? diffuse->GetID() : 0);
    glUniform1i(App->programs->GetUniformLocation("material.diffuse_map"), 0);

    glUniform4fv(App->programs->GetUniformLocation("material.diffuse_color"), 1, (const float*)&material->GetDiffuseColor());
    glUniform3fv(App->programs->GetUniformLocation("material.specular_color"), 1, (const float*)&material->GetSpecularColor());

    glUniform1f(App->programs->GetUniformLocation("material.k_ambient"), material->GetKAmbient());
    glUniform1f(App->programs->GetUniformLocation("material.k_diffuse"), material->GetKDiffuse());
    glUniform1f(App->programs->GetUniformLocation("material.k_specular"), material->GetKSpecular());
}

void ModuleRenderer::UpdateLightUniform() const
{
    const AmbientLight& ambient = App->level->GetAmbientLight();
    const DirLight& directional = App->level->GetDirLight();

    float3 dir = directional.GetDir();

    glUniform3fv(App->programs->GetUniformLocation("ambient.color"), 1, (const float*)&ambient.GetColor());
    glUniform3fv(App->programs->GetUniformLocation("directional.color"), 1, (const float*)&directional.GetColor());
    glUniform3fv(App->programs->GetUniformLocation("directional.dir"), 1, (const float*)&dir);
}

