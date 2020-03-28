#include "Globals.h"

#include "ComponentGrass.h"

#include "ResourceMaterial.h"
#include "ResourceMesh.h"
#include "ModuleResources.h"
#include "ModulePrograms.h"
#include "GameObject.h"

#include "Application.h"

#include "OpenGL.h"

#include "mmgr/mmgr.h"

ComponentGrass::ComponentGrass(GameObject* object) : Component(object, Types::Grass)
{
    // first row ==> positions, second row ==> uv´s
    static const float vertices[]   = { -0.5f,  0.0f, 0.0f , 0.5f,  0.0f, 0.0f , 0.5f, 1.0f, 0.0f , -0.5f, 1.0f, 0.0f, 
                                         0.0f,  0.0f,        1.0f,  0.0f,        1.0f, 1.0f,         0.0f, 1.0f };

    static const unsigned indices[] = { 0, 1, 2, 0, 2, 3 };

    static VertexAttrib attribs[]   = { {0, 3, GL_FLOAT, false, sizeof(float)*3, 0 }, {2, 2, GL_FLOAT, GL_FALSE, sizeof(float)*2, 4*sizeof(float)*3} };

    billboard_vbo.reset(Buffer::CreateVBO(GL_STATIC_DRAW, sizeof(vertices), (void*)vertices));
    billboard_ibo.reset(Buffer::CreateIBO(GL_STATIC_DRAW, sizeof(indices), (void*)indices));
    billboard_vao = std::make_unique<VertexArray>(billboard_vbo.get(), billboard_ibo.get(), attribs, 2);
}

void ComponentGrass::OnSave(Config& config) const 
{
	config.AddUID("Mesh", mesh.GetUID());
	config.AddUID("Material", material.GetUID());
}

void ComponentGrass::OnLoad(Config* config) 
{
	SetMesh(config->GetUID("Mesh", 0));
	SetMaterial(config->GetUID("Material", 0));
}

void ComponentGrass::Draw()
{
    const GameObject* go             = GetGameObject();
	const ResourceMesh* mesh         = GetMesh();
	const ResourceMaterial* material = GetMaterial();

    if(material != nullptr && mesh != nullptr)
    {
        float4x4 transform = go->GetGlobalTransformation();

        glUniformMatrix4fv(App->programs->GetUniformLocation("model"), 1, GL_TRUE, reinterpret_cast<const float*>(&transform));

        material->UpdateUniforms();
        material->BindTextures();
        mesh->Draw();
        material->UnbindTextures();
    }
}

const ResourceMesh* ComponentGrass::GetMesh() const 
{
    return mesh.GetPtr<ResourceMesh>(); 
}

ResourceMesh* ComponentGrass::GetMesh()
{
    return mesh.GetPtr<ResourceMesh>(); 
}

const ResourceMaterial* ComponentGrass::GetMaterial() const
{ 
    return material.GetPtr<ResourceMaterial>(); 
}

ResourceMaterial* ComponentGrass::GetMaterial()
{
    return material.GetPtr<ResourceMaterial>(); 
}
