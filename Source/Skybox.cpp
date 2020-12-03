#include "Globals.h"
#include "Skybox.h"

#include "Application.h"
#include "ModuleResources.h"
#include "ModulePrograms.h"

#include "ResourceTexture.h"

#include "OpenGL.h"

Skybox::Skybox()
{
    static float skybox_vertices[] = {
        // front
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

    VertexAttrib attribs[] = { { 0, 3, GL_FLOAT, false, 0, 0 } };

    vbo = std::unique_ptr<Buffer>(Buffer::CreateVBO(GL_STATIC_DRAW, sizeof(skybox_vertices), skybox_vertices));
    vao = std::make_unique<VertexArray>(vbo.get(), nullptr, attribs, sizeof(attribs) / sizeof(VertexAttrib));
}

Skybox::~Skybox()
{
    if(cubemap)
    {
        App->resources->Get(cubemap)->Release();
    }
}

void Skybox::Draw(const float4x4& proj, const float4x4& view)
{
    App->programs->UseProgram("skybox", 0);

    glUniformMatrix4fv(App->programs->GetUniformLocation("proj"), 1, GL_TRUE, reinterpret_cast<const float*>(&proj));
    glUniformMatrix4fv(App->programs->GetUniformLocation("view"), 1, GL_TRUE, reinterpret_cast<const float*>(&view));

    Texture* texture = nullptr;
    if(cubemap != 0)
    {
        texture = App->resources->GetTexture(cubemap)->GetTexture();
    }
    else
    {
        texture = App->resources->GetDefaultSkybox()->GetTexture();
    }

    if(texture)
    {
        texture->Bind(0, App->programs->GetUniformLocation("skybox"));
    }

    glDisable(GL_DEPTH_TEST);
    vao->Bind();
    glDrawArrays(GL_TRIANGLES, 0, 6*6);

    glEnable(GL_DEPTH_TEST);
}

void Skybox::SetCubemap(UID uid)
{
    if(cubemap != 0)
    {
        App->resources->Get(cubemap)->Release();
    }

    ResourceTexture* res = App->resources->GetTexture(uid);
    if(res)
    {
        res->LoadToMemory();
        cubemap = uid;
    }
    else
    {
        LOG("UID %d is not a texture!!!", uid);
        cubemap = 0;
    }
}

