#include "Globals.h"

#include "ComponentGrass.h"

#include "ResourceMaterial.h"
#include "ResourceMesh.h"
#include "ModuleResources.h"
#include "ModulePrograms.h"
#include "GameObject.h"

#include "Application.h"

#include "OpenGL.h"

#include "Leaks.h"

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


    // value noise
    
    random_values.reserve(256);

    float scale = 15.0f;
    
    for(uint i=0; i< 256; ++i)
    {
        random_values.push_back(App->random->Float01Incl());
    }
}

float ComponentGrass::ValueNoise(float v) const 
{
    float i = floor(v);
    float f = v-i; // fractional

    float a = Hash(i);
    float b = Hash(i+1);

    float smooth = f*f*(3.0f-2.0f*f); // Cubic Hermite

    return a*(1-smooth)+b*smooth; // lerp
}

float ComponentGrass::Hash(float value) const
{ 
    float lValue = sinf(value)*345235.323f;

    return lValue-floor(lValue); 
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
    const GameObject* go       = GetGameObject();
	const ResourceMesh* mesh   = GetMesh();
	ResourceMaterial* material = GetMaterial();

    if(material != nullptr && mesh != nullptr)
    {
        float4x4 transform = go->GetGlobalTransformation();

        glUniformMatrix4fv(App->programs->GetUniformLocation("model"), 1, GL_TRUE, reinterpret_cast<const float*>(&transform));

        /* TODO: 
        material->UpdateUniforms();
        material->BindTextures();
        mesh->Draw();
        material->UnbindTextures();
        */
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


