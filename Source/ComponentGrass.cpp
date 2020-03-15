#include "Globals.h"

#include "ComponentGrass.h"

#include "ResourceTexture.h"

#include "ModuleResources.h"

#include "Application.h"

#include "OpenGL.h"

#include "mmgr/mmgr.h"

ComponentGrass::ComponentGrass(GameObject* object) : Component(object, Types::Grass)
{
    // first row ==> positions, second row ==> uv�s
    static const float vertices[] = { -0.5f,  0.0f, 0.0f , 0.5f,  0.0f, 0.0f , 0.5f, 1.0f, 0.0f , -0.5f, 1.0f, 0.0f, 
                                       0.0f,  0.0f,        1.0f,  0.0f,        1.0f, 1.0f,        0.0f, 1.0f };

    static const unsigned indices[] = { 0, 1, 2, 0, 2, 3 };

    static VertexAttrib attribs[] = { {0, 3, GL_FLOAT, false, sizeof(float)*3, 0 }, {2, 2, GL_FLOAT, GL_FALSE, sizeof(float)*2, 4*sizeof(float)*3} };

    billboard_vbo.reset(Buffer::CreateVBO(GL_STATIC_DRAW, sizeof(vertices), (void*)vertices));
    billboard_ibo.reset(Buffer::CreateIBO(GL_STATIC_DRAW, sizeof(indices), (void*)indices));
    billboard_vao = std::make_unique<VertexArray>(billboard_vbo.get(), billboard_ibo.get(), attribs, 2);
}

ComponentGrass::~ComponentGrass()
{
}

void ComponentGrass::OnSave(Config& config) const 
{
	config.AddUID("Albedo", albedo.GetUID());
	config.AddUID("Normal", normal.GetUID());
}

void ComponentGrass::OnLoad(Config* config) 
{
	SetAlbedo(config->GetUID("Albedo", 0));
	SetNormal(config->GetUID("Normal", 0));
}

bool ComponentGrass::SetAlbedo(UID uid)
{
    albedo = uid;

    return albedo;
}

bool ComponentGrass::SetNormal(UID uid)
{
    normal = uid;

    return normal;
}

