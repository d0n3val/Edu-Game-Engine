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

    if(test == nullptr)
    {
        test = App->resources->GetDefaultSkybox()->GetTexture();
    }

    App->programs->UseProgram("skybox", 0);

    if (test)
    {
        test->Bind(0, App->programs->GetUniformLocation("skybox"));
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
    if(cubemap != 0)
    {
        App->resources->Get(cubemap)->Release();
    }

    ResourceTexture* res = App->resources->GetTexture(uid);
    if(res)
    {
        res->LoadToMemory();
        
        if(res->GetType() == ResourceTexture::TextureCube)
        {
            test = res->GetTexture();
        }
        else
        {
            test = ConvertToCubemap(uid, 512, 512);
        }

        cubemap = uid;
    }
    else
    {
        LOG("UID %d is not a texture!!!", uid);
        cubemap = 0;
    }
}

TextureCube* Skybox::ConvertToCubemap(UID uid, uint width, uint height) const 
{
	Frustum frustum;
	frustum.type = FrustumType::PerspectiveFrustum;

	frustum.pos  = float3::zero;

	frustum.nearPlaneDistance = 0.1f;
	frustum.farPlaneDistance  = 100.0f;
	frustum.verticalFov       = math::pi/2.0f; 
	frustum.horizontalFov     = math::pi/2.0f; 

    Framebuffer frameBuffer;
    TextureCube* cubeMap = new TextureCube();

    // initialize each cubemap plane
    for(uint i=0; i< 6; ++i)
    {
        cubeMap->SetData(i, 0, width, height, GL_RGB16F, GL_RGB, GL_FLOAT, nullptr);
    }

    // \todo: Check each plane x,y origin 
    float3 front[6] = { float3(1.0f, 0.0f, 0.0f), float3(-1.0f, 0.0f, 0.0f), 
                        float3(0.0f, 1.0f, 0.0f), float3(0.0f, -1.0f, 0.0f), 
                        float3(0.0f, 0.0f, 1.0f), float3(0.0f, 0.0f, -1.0f) };

    float3 up[6]    = { float3(0.0f, -1.0f, 0.0f), float3(0.0f, -1.0f, 0.0f),
                        float3(0.0f, 0.0f, 1.0f), float3(0.0f, 0.0f, -1.0f),
                        float3(0.0f, -1.0f, 0.0f), float3(0.0f, -1.0f, 0.0f) };

    App->programs->UseProgram("equirectangular");

    Texture* texture = App->resources->GetTexture(uid)->GetTexture();
    texture->Bind(0, App->programs->GetUniformLocation("skybox"));

    // Render each cube plane
    for(uint i=0; i<6; ++i)
    {
        frameBuffer.AttachColor(cubeMap, i, 0, 0);
        frameBuffer.Clear(width, height);

        frustum.front = front[i];
        frustum.up = up[i];

        float4x4 proj = frustum.ProjectionMatrix();
        float4x4 view = frustum.ViewMatrix();

        glUniformMatrix4fv(App->programs->GetUniformLocation("proj"), 1, GL_TRUE, reinterpret_cast<const float*>(&proj));
        glUniformMatrix4fv(App->programs->GetUniformLocation("view"), 1, GL_TRUE, reinterpret_cast<const float*>(&view));

        glDepthFunc(GL_ALWAYS);
        vao->Bind();
        glDrawArrays(GL_TRIANGLES, 0, 6*6);
        glDepthFunc(GL_LESS);
    }

    return cubeMap;
}

