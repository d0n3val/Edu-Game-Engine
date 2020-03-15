#include "Globals.h"

#include "ComponentGrass.h"

#include "ResourceTexture.h"

#include "ModuleResources.h"

#include "Application.h"

#include "OpenGL.h"

#include "mmgr/mmgr.h"

ComponentGrass::ComponentGrass(GameObject* object) : Component(object, Types::Grass)
{
    // first row ==> positions, second row ==> uv´s
    static const float vertices[] = { -0.5f,  0.0f, 0.0f , 0.5f,  0.0f, 0.0f , 0.5f, 1.0f, 0.0f , -0.5f, 1.0f, 0.0f, 
                                       0.0f,  0.0f,        1.0f,  0.0f,        1.0f, 1.0f,        0.0f, 1.0f };

    static const unsigned indices[] = { 0, 1, 2, 0, 2, 3 };

    billboard_vbo.reset(Buffer::CreateVBO(GL_STATIC_DRAW, sizeof(vertices), (void*)vertices));
    billboard_ibo.reset(Buffer::CreateIBO(GL_STATIC_DRAW, sizeof(indices), (void*)indices));

    glGenVertexArrays(1, &billboard_vao);
    glBindVertexArray(billboard_vao);

    billboard_vbo->Bind();

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float)*3, (void*)0);

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(float)*2, (void*)(4*sizeof(float)*3));

    billboard_vbo->Unbind();

}

ComponentGrass::~ComponentGrass()
{
	Resource* res = App->resources->Get(albedo);
	if (res != nullptr)
	{
		res->Release();
	}

	res = App->resources->Get(normal);
	if (res != nullptr)
	{
		res->Release();
	}

    if(billboard_vao != 0)
    {
        glDeleteVertexArrays(1, &billboard_vao);
        billboard_vao = 0;
    }
}

void ComponentGrass::OnSave(Config& config) const 
{
	config.AddUID("Albedo", albedo);
	config.AddUID("Normal", normal);
}

void ComponentGrass::OnLoad(Config* config) 
{
	SetAlbedo(config->GetUID("Albedo", 0));
	SetNormal(config->GetUID("Normal", 0));
}

bool ComponentGrass::SetAlbedo(UID uid)
{
    Resource* res = App->resources->Get(albedo);
    if(res != nullptr)
    {
        assert(res->GetType() == Resource::texture);

        res->Release();
    }

    res = App->resources->Get(uid);

    if(res != nullptr && res->GetType() == Resource::texture)
    {
        if(res->LoadToMemory())
        {
            albedo = uid;

            return true;
        }
    }

    return false;
}

const ResourceTexture* ComponentGrass::GetAlbedo () const
{
    return static_cast<const ResourceTexture*>(App->resources->Get(albedo));
}
   
bool ComponentGrass::SetNormal(UID uid)
{
    Resource* res = App->resources->Get(normal);
    if(res != nullptr)
    {
        assert(res->GetType() == Resource::texture);

        res->Release();
    }

    res = App->resources->Get(uid);

    if(res != nullptr && res->GetType() == Resource::texture)
    {
        if(res->LoadToMemory())
        {
            normal = uid;

            return true;
        }
    }

    return false;
}

const ResourceTexture* ComponentGrass::GetNormal() const
{
    return static_cast<const ResourceTexture*>(App->resources->Get(albedo));
}
