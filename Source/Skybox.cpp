#include "Globals.h"
#include "Skybox.h"

#include "Application.h"
#include "ModuleResources.h"
#include "ModulePrograms.h"
#include "Config.h"

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
        Resource* res = App->resources->Get(cubemap);
        
        if (res)
        {
            res->Release();
        }
    }
}

void Skybox::Load(const Config& config)
{
    UID texture = config.GetUInt("Texture", 0);
    if (texture)
    {
        SetCubemap(texture);
    }
}

void Skybox::Save(Config& config) const
{
    config.AddUInt("Texture", uint(cubemap));
}

void Skybox::Draw(const float4x4& proj, const float4x4& view)
{
    bool isCubemap;

    Texture* texture = nullptr;
    if(cubemap != 0)
    {
        ResourceTexture* textureRes = App->resources->GetTexture(cubemap);
        if (textureRes)
        {
            isCubemap = textureRes->GetType() == ResourceTexture::TextureCube;
            texture = textureRes->GetTexture();
        }
    }


    if(!texture)
    {
        isCubemap = true;
        texture = App->resources->GetDefaultSkybox()->GetTexture();
    }

    App->programs->UseProgram(isCubemap ? "skybox" : "equirectangular", 0);

    if (texture)
    {
        texture->SetWrapping(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
        texture->Bind(0, App->programs->GetUniformLocation("skybox"));
    }

    glUniformMatrix4fv(App->programs->GetUniformLocation("proj"), 1, GL_TRUE, reinterpret_cast<const float*>(&proj));
    glUniformMatrix4fv(App->programs->GetUniformLocation("view"), 1, GL_TRUE, reinterpret_cast<const float*>(&view));

    glDepthFunc(GL_ALWAYS);
    vao->Bind();
    glDrawArrays(GL_TRIANGLES, 0, 6*6);
    glDepthFunc(GL_LESS);
}

void Skybox::SetCubemap(UID uid)
{
    Resource* old = App->resources->Get(cubemap);
    if(old)
    {
        old->Release();
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

